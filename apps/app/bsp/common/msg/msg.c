

#pragma bss_seg(".msg.data.bss")
#pragma data_seg(".msg.data")
#pragma const_seg(".msg.text.const")
#pragma code_seg(".msg.text")
#pragma str_literal_override(".msg.text.const")

#include "msg.h"
#include "irq.h"
#include "common.h"
/* #include "string.h" */
#include "circular_buf.h"
#include "uart.h"
#include <stdarg.h>
#include "config.h"
#include "power_interface.h"
#include "app_config.h"

/* #define LOG_TAG_CONST       NORM */
#define LOG_TAG_CONST       OFF
#define LOG_TAG             "[msg]"
#include "log.h"


#define EVENT_TOTAL     (1+(sizeof(event2msg)/2 -1)/32)

static const u16 event2msg[] = {
    MSG_F1A1_FILE_END, 		/* 0 */
    MSG_F1A1_FILE_ERR, 		/* 1 */
    MSG_F1A1_LOOP,
    MSG_F1A2_FILE_END,
    MSG_F1A2_FILE_ERR,
    MSG_F1A2_LOOP,          /* 5 */
    MSG_MIDI_FILE_END,
    MSG_MIDI_FILE_ERR,
    NO_MSG,
    MSG_A_FILE_END,
    MSG_A_FILE_ERR,    		/* 10 */
    MSG_A_LOOP,
    MSG_MP3_FILE_END,
    MSG_MP3_FILE_ERR,
    MSG_MP3_LOOP,
    MSG_WAV_FILE_END,	    /* 15 */
    MSG_WAV_FILE_ERR,
    MSG_WAV_LOOP,
    MSG_OPUS_FILE_END,      /* 18 */
    MSG_OPUS_FILE_ERR,
    MSG_OPUS_LOOP,

    MSG_APP_SWITCH_ACTIVE,  /* 21 */
    MSG_WFILE_FULL,

    MSG_OTG_IN,             /* 23 */
    MSG_OTG_OUT,
    MSG_USB_DISK_IN,
    MSG_USB_DISK_OUT,
    MSG_PC_IN,
    MSG_PC_OUT,             /* 28 */
    MSG_PC_SPK,             /* 29 */
    MSG_PC_MIC,             /* 30 */
    MSG_SDMMCA_IN,
    MSG_SDMMCA_OUT,
    MSG_AUX_IN,
    MSG_AUX_OUT,
    MSG_EXTFLSH_IN,         /* 35 */

    MSG_BLE_APP_UPDATE_START,		/* 36 */
    MSG_BLE_TESTBOX_UPDATE_START,	/* 37 */
    MSG_UART_TESTBOX_UPDATE_START,	/* 38 */
    NO_MSG,
};

NOT_KEEP_RAM
static cbuffer_t msg_cbuf;

NOT_KEEP_RAM
static u32 event_buf[EVENT_TOTAL];

NOT_KEEP_RAM
static u32 msg_pool[MAX_POOL];
/* static u32 msg_test_num; */

#define  MSG_BUF_BUSY_BIT    BIT(0)
#define  MSG_EVENT_BUSY_BIT  BIT(1)
static   u8 msg_busy_state; //记录msg的状态

#ifdef CONFIG_DEBUG_ENABLE
static u16 msg_remain_min = (MAX_POOL << 2);
#define msg_cbuf_remain()     (msg_cbuf.total_len - cbuf_get_data_size(&msg_cbuf))
#endif

void clear_one_event(u32 event)
{
    if (event >= ARRAY_SIZE(event2msg)) {
        return;
    }
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    event_buf[event / 32] &= ~BIT(event % 32);
    OS_EXIT_CRITICAL();
}


_NOINLINE_
static u32 get_event(void)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    u32 event_cls;
    for (u32 i = 0; i < EVENT_TOTAL; i++) {
        __asm__ volatile("%0 = clz(%1)":"=r"(event_cls):"r"(event_buf[i]));
        if (event_cls != 32) {
            OS_EXIT_CRITICAL();
            /* log_info(" has event 0x%x\n",i*32 + (31 - event_cls)); */
            return i * 32 + (31 - event_cls);
        }
    }
    OS_EXIT_CRITICAL();

    return NO_EVENT;
}

static u32 event2msg_api(u32 event)
{
    return event2msg[event];
}

bool get_event_status(u32 event)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    if (event_buf[event / 32] & BIT(event % 32)) {
        OS_EXIT_CRITICAL();
        return TRUE;
    }
    OS_EXIT_CRITICAL();
    return FALSE;
}

bool has_sys_event(void)
{
    /* CPU_SR_ALLOC(); */
    /* OS_ENTER_CRITICAL(); */
    for (u32 i = 0; i < EVENT_TOTAL; i++) {
        if (0 != event_buf[i]) {
            return true;
        }
    }
    /* OS_EXIT_CRITICAL(); */
    return FALSE;
}

_NOINLINE_
int get_msg_phy(int len, int *msg, bool idle)
{
    u32 param = 0;
    u16 *t_msg = (u16 *)&param;
    //get_msg
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    u32 tlen = cbuf_read(&msg_cbuf, (void *)t_msg, MSG_HEADER_BYTE_LEN);

    if (MSG_HEADER_BYTE_LEN != tlen) {
        /* if (MSG_HEADER_BYTE_LEN != cbuf_read(&msg_cbuf, (void *)&param, MSG_HEADER_BYTE_LEN)) { */
        /* memset(msg, NO_MSG, len); */
        OS_EXIT_CRITICAL();

        /* log_info(" gm a 0x%x\n",param); */
        /*get no msg,cpu enter idle.why do this? TODO*/
        /* __builtin_pi32_idle(); */

        msg_busy_state &= (~MSG_BUF_BUSY_BIT);
        msg[0] = NO_MSG;
        return MSG_NO_MSG;
    }
    /* log_info(" gm a 0x%x\n",param); */
    msg[0] = t_msg[0] & (MSG_HEADER_ALL_BIT >> MSG_PARAM_BIT_LEN);
    u32 param_len = param >> MSG_TYPE_BIT_LEN;
    if (param_len > (len - 1)) {
        OS_EXIT_CRITICAL();
        return MSG_BUF_NOT_ENOUGH;
    }
    u32 rlen = cbuf_read(&msg_cbuf, (void *)(msg + 1), param_len * sizeof(int));
    if ((param_len * sizeof(int)) != rlen) {
        OS_EXIT_CRITICAL();
        return MSG_CBUF_ERROR;
    }

    // 处理 sys_event* 和 dev->ops 指针
    if (msg[0] == MSG_TYPE_EVENT) {
        void *event_ptr = NULL;
        void *ops_ptr = NULL;

        rlen = cbuf_read(&msg_cbuf, (void *)&event_ptr, sizeof(void *));
        if (sizeof(void *) != rlen) {
            OS_EXIT_CRITICAL();
            return MSG_CBUF_ERROR;
        }
        msg[param_len + 1] = (int)event_ptr;

        rlen = cbuf_read(&msg_cbuf, (void *)&ops_ptr, sizeof(void *));
        if (sizeof(void *) != rlen) {
            OS_EXIT_CRITICAL();
            return MSG_CBUF_ERROR;
        }
        msg[param_len + 2] = (int)ops_ptr;
    }
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}


_NOINLINE_
static int get_msg_sub(int len, int *msg)
{
    /* u32 param = 0; */
    /* u16 *t_msg = (u16 *)&param; */
    //get_msg
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    u32 event = get_event();

    if (event != NO_EVENT) {
        /* log_info("event 0x%x\n", event); */
        clear_one_event(event);
        msg[0] = event2msg[event];
        //log_info("event_mag %d\n ", msg[0]);
        OS_EXIT_CRITICAL();
        return MSG_NO_ERROR;
    } else {
        msg_busy_state &= (~MSG_EVENT_BUSY_BIT);
    }
    OS_EXIT_CRITICAL();

    return get_msg_phy(len, msg, 1);
}

int get_msg(int len, int *msg)
{
    if (!msg_busy_state) {
        msg[0] = NO_MSG;
        return MSG_NO_MSG;
    }
    return get_msg_sub(len, msg);
}

int post_event(int event)
{
    if (event >= ARRAY_SIZE(event2msg)) {
        return MSG_EVENT_PARAM_ERROR;
    }
    CPU_SR_ALLOC();
    /* log_info(">>> %d evenr post : 0x%x\n", msg_test_num++, event); */
    OS_ENTER_CRITICAL();
    event_buf[event / 32] |= BIT(event % 32);
    msg_busy_state |= MSG_EVENT_BUSY_BIT;
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}

int post_msg(int argc, ...)
{
    u32 param;
    u16 *t_msg = (u16 *)&param;
    CPU_SR_ALLOC();
    va_list argptr;
    OS_ENTER_CRITICAL();
    va_start(argptr, argc);

    int msg = va_arg(argptr, int);
    size_t additional_size = 0;
    if (msg == MSG_TYPE_EVENT) {
        additional_size += 2 * sizeof(void *); // for sys_event* and dev->ops
    }
    if (!cbuf_is_write_able(&msg_cbuf, (argc - 1) * sizeof(int) + additional_size + MSG_HEADER_BYTE_LEN)) {
        OS_EXIT_CRITICAL();
        va_end(argptr);
        putchar('f');
        return MSG_BUF_NOT_ENOUGH;
    }

    t_msg[0] = (MSG_HEADER_ALL_BIT >> MSG_PARAM_BIT_LEN) & msg;
    t_msg[0] = ((argc - 1) << (MSG_TYPE_BIT_LEN)) | t_msg[0];
    cbuf_write(&msg_cbuf, (void *)&t_msg[0], MSG_HEADER_BYTE_LEN);

    for (u32 i = 0; i < argc - 1; i++) {
        param = va_arg(argptr, int);
        cbuf_write(&msg_cbuf, (void *)&param, sizeof(int));
    }

    if (msg == MSG_TYPE_EVENT) {
        void *event_ptr = va_arg(argptr, void *);
        void *ops_ptr = va_arg(argptr, void *);
        cbuf_write(&msg_cbuf, &event_ptr, sizeof(void *));
        cbuf_write(&msg_cbuf, &ops_ptr, sizeof(void *));
    }
    va_end(argptr);
#ifdef CONFIG_DEBUG_ENABLE
    if (msg_remain_min > msg_cbuf_remain()) {
        msg_remain_min = msg_cbuf_remain();
    }
#endif
    msg_busy_state |= MSG_BUF_BUSY_BIT;
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}

void msg_debug_info(void)
{
#ifdef CONFIG_DEBUG_ENABLE
    printf("msg_remain_min= %04x", msg_remain_min);
#endif
}

void clear_all_message(void)
{
    cbuf_clear(&msg_cbuf);
}

void message_init()
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    memset(event_buf, 0, sizeof(event_buf));
    cbuf_init(&msg_cbuf, msg_pool, sizeof(msg_pool));
    //cbuf_clear(&msg_cbuf);
    msg_busy_state = 0;
    OS_EXIT_CRITICAL();
}

//entern low_power controller
static u8 msg_lowpower_idle_query(void)
{
    return !msg_busy_state;
}

REGISTER_LP_TARGET(msg_lowpower_target) = {
    .name = "msg_lowpwer_deal",
    .is_idle = msg_lowpower_idle_query,
};

static u8 msg_exit_leep(void)
{
    message_init();
    return 0;
}

//power off process
DEEPSLEEP_TARGET_REGISTER(sys_msg_sleep) = {
    .name   = "sys_msg",
    .enter  = NULL,
    .exit   = msg_exit_leep,
};


