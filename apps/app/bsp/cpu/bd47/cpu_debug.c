#include "power_interface.h"
#include "app_config.h"
#include "gpio.h"
#include "sys_memory.h"
#include "gpadc.h"
#include "sys_timer.h"
#include "cpu_debug.h"
#include "my_malloc.h"
#include "clock.h"
#include "printf.h"
#include "msg.h"

#ifdef CONFIG_SDK_DEBUG_LOG

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[CPU_DB]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf

#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

enum {
    DB_EVENT_CHECK_STACK_MALLOC  = 0,
    DB_EVENT_TEST_FREQ_TABLE,
    //add here

    DB_EVENT_MAX,
};

#define CPU_DB_TEST_FREQ_TABLE           0 //测试频率挡位
#define CPU_DB_TEST_FREQ_SWITCH_TIME     30000

#define CPU_DB_CHECK_SSTACK_INFO         0 //检查中断堆栈深度需要disable sleep
#define CPU_DB_LOG_OUT_TIMER             120000

static uint8_t cpu_db_is_active;
static uint8_t cpu_db_init;
static u32 cpu_db_event;

#define CPU_DB_EVENT_SET(a)              cpu_db_event |= BIT(a)
#define CPU_DB_EVENT_CLR(a)              cpu_db_event &= (~BIT(a))
#define CPU_DB_EVENT_CHECK(a)            cpu_db_event & BIT(a)
#define CPU_DB_EVENT_CLR_ALL()           cpu_db_event = 0;

//====================================================================
static void cpu_db_timer_handler(void *priv)
{
    if ((int)(priv) <  DB_EVENT_MAX) {
        CPU_DB_EVENT_SET((int)(priv));
    } else {
        log_info("err:db_timer type");
    }
}

//============================
static void sdk_cpu_debug_init(void)
{
    if (!cpu_db_init) {
        cpu_db_init = 1;
        CPU_DB_EVENT_SET(DB_EVENT_CHECK_STACK_MALLOC);
        sys_timer_add((void *)DB_EVENT_CHECK_STACK_MALLOC, cpu_db_timer_handler, CPU_DB_LOG_OUT_TIMER);

#if CPU_DB_TEST_FREQ_TABLE
        cpu_db_is_active = 1;//
        CPU_DB_EVENT_SET(DB_EVENT_TEST_FREQ_TABLE);
        sys_timer_add((void *)DB_EVENT_TEST_FREQ_TABLE, cpu_db_timer_handler, CPU_DB_TEST_FREQ_SWITCH_TIME);
#endif
    }
}

void sdk_cpu_debug_main_init(void)
{
#if CPU_DB_CHECK_SSTACK_INFO
    cpu_db_is_active = 1;//
#endif
    stack_debug_free_check_init();
}

void sdk_cpu_debug_check_sdk_info_logout(void)
{
    msg_debug_info();
    stack_debug_free_check_info();
#if TCFG_USER_BLE_ENABLE
    log_info("nk_remain= 0x%04x; bt_nk_malloc: size= 0x%04x ,free= 0x%04x", NK_RAM_REMAIN_SIZE, NK_RAM_MALLOC_SIZE, __bt_nk_get_free_size());
    log_info("nv_remain= 0x%04x; bt_nv_malloc: size= 0x%04x ,free= 0x%04x", NV_RAM_REMAIN_SIZE, NV_RAM_MALLOC_SIZE, __bt_get_free_size());
#endif

    log_info("my_malloc(heap): size= 0x%04x ,free= 0x%04x", SYS_HEAP_MALLOC_SIZE, my_get_free_size());

    log_info("cur_is_irq?%d", cpu_run_is_irq_mode());
}

//============================

#if TCFG_CLOCK_SYS_PLL_HZ == 240000000
static const u8 freq_test_table[] = {60, 120, 160};
#else
static const u8 freq_test_table[] = {48, 64, 96, 128};
#endif

static void sdk_cpu_debug_test_freq(void)
{
#if TCFG_CLOCK_SYS_PLL_HZ == 240000000
    log_info("set pll-240m:");
#else
    log_info("set pll-192m:");
#endif

    static u8 freq_index = 0;
    if (freq_index >= sizeof(freq_test_table) / sizeof(u8)) {
        freq_index = 0;
    }

    u32 freq_val = freq_test_table[freq_index] * 1000000;
    freq_index++;
    log_info("test sys_freq = %d", freq_val);
    clk_set("sys", freq_val);
}

//============================
void sdk_cpu_debug_loop_call(void)
{
    sdk_cpu_debug_init();

    if (cpu_db_event) {
        log_info("cpu_db_event:0x%04x", cpu_db_event);
    }

    if (CPU_DB_EVENT_CHECK(DB_EVENT_CHECK_STACK_MALLOC)) {
        CPU_DB_EVENT_CLR(DB_EVENT_CHECK_STACK_MALLOC);
        sdk_cpu_debug_check_sdk_info_logout();
    }

    if (CPU_DB_EVENT_CHECK(DB_EVENT_TEST_FREQ_TABLE)) {
        CPU_DB_EVENT_CLR(DB_EVENT_TEST_FREQ_TABLE);
        sdk_cpu_debug_test_freq();
    }
}

//============================
//配置 config_exception_record_info =1,重写下面的函数获取异常信息，用户自定义存在vm或flash区域记录
void debug_record_user_deal(char *record_buf, u32 record_buf_len, u32 *usp, u32 *ssp, u32 *sp)
{
    if (config_exception_record_info) {
        //格式如下:
        printf(">>>>>>debug_hook(%u):\n%s\n", strlen(record_buf), record_buf);
        printf("usp : \r\n");
        put_buf((u8 *)usp, 512);
        printf("ssp : \r\n");
        put_buf((u8 *)ssp, 512);
        printf(" sp : \r\n");
        put_buf((u8 *)sp, 512);
    }
}

//============================
static uint8_t cpu_db_idle_query(void)
{
    return !cpu_db_is_active;
}

REGISTER_LP_TARGET(cpu_debug_lp_target) = {
    .name = "cpu_debug",
    .is_idle = cpu_db_idle_query,
};

#endif





