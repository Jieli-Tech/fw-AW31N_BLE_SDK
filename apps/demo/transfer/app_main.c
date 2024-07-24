/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "app_power_mg.h"
#include "app_modules.h"
#include "msg.h"
#include "my_malloc.h"

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

//*********************************************************************************//
//                sdk 堆栈、堆、蓝牙ram的分配定义                                  //
//*********************************************************************************//
int _sstack_top[1] sec_used(.sstack_top);//fixed
int _ustack_top[1] sec_used(.ustack_top);//fixed

//for ld.c link
static int _sstack_space[SYS_STACK_SIZE_ALL / 4] sec_used(.sstack);
static int _ustack_space[USR_STACK_SIZE_ALL / 4] sec_used(.ustack);

static int _sys_heap_space[SYS_HEAP_SIZE / 4] sec_used(.sec_sys_heap);
static int _bt_nk_ram_min[BT_NK_RAM_SIZE / 4] sec_used(.sec_bt_nk_ram); //最少占用
static int _bt_nv_ram_min[BT_NV_RAM_SIZE / 4] sec_used(.sec_bt_nv_ram);//最少占用

//*********************************************************************************//
static struct application *main_application_operation_state(struct application *app, enum app_state state);

//*********************************************************************************//
//                                  中断优先级配置                                 //
//*********************************************************************************//
const int IRQ_IRTMR_IP        = 6;
const int IRQ_WFILE_IP        = 1;
const int IRQ_ADC_IP          = 1;
const int IRQ_TICKTMR_IP      = 3;
const int IRQ_USB_IP	      = 3;
const int IRQ_SD_IP		      = 3;
const int IRQ_CTMU_IP	      = 2;
const int IRQ_LEDC_IP         = 1;
const int IRQ_SLCD_IP         = 2;
//BT
const int IRQ_BT_TIMEBASE_IP  = 6;   //BT TIMEBASE
const int IRQ_BLE_EVENT_IP    = 5;   //BT RX_EVT
const int IRQ_BLE_RX_IP       = 5;   //BT RX
const int IRQ_BTSTACK_MSG_IP  = 4;   //BT STACK
const int IRQ_BREDR_IP        = 3;   //no use
const int IRQ_BT_RXMCH_IP     = 3;   //no use
const int IRQ_AES_IP          = 3;   //aes

extern void timer_task_scan();
void app_timer_loop(void)
{
    timer_task_scan();
}

APP_VAR app_var;

void app_var_init(void)
{
    app_var.play_poweron_tone = 1;

    app_var.auto_off_time =  0; //TCFG_AUTO_SHUT_DOWN_TIME;
    app_var.warning_tone_v = 340;
    app_var.poweroff_tone_v = 330;
}

__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

void clr_wdt(void);

void app_main()
{
    //TODO
    /* if (!UPDATE_SUPPORT_DEV_IS_NULL()) { */
    /*     int update = 0; */
    /*     update = update_result_deal(); */
    /* } */

    log_info(">>>>>>>>>>>>>>>>>app_main...\n");

    log_info("nk_malloc: %08x,%04x, nv_malloc: %08x,%04x", NK_RAM_MALLOC_START_ADDR, NK_RAM_MALLOC_SIZE,
             NV_RAM_MALLOC_START_ADDR, NV_RAM_MALLOC_SIZE);

    log_info("sstack:size,top= %04x, %08x,ustack:size,top= %04x, %08x", sizeof(_sstack_space), _sstack_top, sizeof(_ustack_space), _ustack_top);

#if TCFG_SYS_LVD_EN
    app_power_vbat_check();
#endif

    main_application_operation_state(NULL, APP_STA_START);

}

/*
 * app模式切换
 */
void app_switch(const char *name, int action)
{
}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    /* log_info("100ms wakeup"); */
    return 1;
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}


static void main_app_get_name(struct intent *it)
{
    init_intent(it);
// 选择应用分支
#if CONFIG_APP_LE_TRANS
    it->name = "le_trans";
    it->action = ACTION_LE_TRANS_MAIN;

#elif CONFIG_APP_DONGLE
    it->name = "dongle";
    it->action = ACTION_DONGLE_MAIN;

#elif CONFIG_APP_IDLE
    it->name = "idle";
    it->action = ACTION_IDLE_MAIN;

#elif CONFIG_APP_NONCONN_24G
    it->name = "nonconn_24g";
    it->action = ACTION_NOCONN_24G_MAIN;

#elif CONFIG_APP_MULTI
    it->name = "multi_conn";
    it->action = ACTION_MULTI_MAIN;

#elif CONFIG_APP_AT_CHAR_COM
    it->name = "at_com";
    it->action = ACTION_AT_COM;

#else
    ASSERT(0, "no app!!!");
#endif

}

static struct application *main_application_operation_state(struct application *app, enum app_state state)
{
    struct intent it;
    const struct application *dev = NULL;

    main_app_get_name(&it);

    log_info("run app>>> %s", it.name);
    log_info("%s,%s", __DATE__, __TIME__);

    list_for_each_app_main(dev) {
        if (memcmp(dev->name, it.name, strlen(it.name)) == 0) {
            if (dev->ops) {
                dev->ops->state_machine(app, state, &it);
            }
        } else {
            log_info("no app run");
        }
    }

    return NULL;
}

void main_sys_event_msg_handle(int *msg)
{
    struct sys_event *event_ptr = (struct sys_event *)msg[1];
    const struct application_operation *ops_ptr = (const struct application_operation *)msg[2];
    ops_ptr->event_handler(NULL, event_ptr);
    event_pool_free(event_ptr);
}


struct application *main_application_operation_event(struct application *app, struct sys_event *event)
{
    struct intent it;
    const struct application *dev = NULL;

    main_app_get_name(&it);

    list_for_each_app_main(dev) {
        if (memcmp(dev->name, it.name, strlen(it.name)) == 0) {
            if (dev->ops) {
                post_msg(3, MSG_TYPE_EVENT, event, dev->ops);
                /* dev->ops->event_handler(app, event); */
            } else {
                log_info("no event run");
            }
        }
    }

    return NULL;
}

void bt_event_update_to_user(u8 *addr, u32 type, u8 event, u32 value)
{
    log_info("bt_event_update_to_user type:%d\n", type);
    struct sys_event *e = event_pool_alloc();
    if (e == NULL) {
        log_info("Memory allocation failed for sys_event");
        return;
    }

    e->type = 0x0010;
    if (addr != NULL) {
        memcpy(e->u.bt.args, addr, 6);
    }
    e->arg = (void *)type;
    e->u.bt.event = event;
    e->u.bt.value = value;

    main_application_operation_event(NULL, e);
}


