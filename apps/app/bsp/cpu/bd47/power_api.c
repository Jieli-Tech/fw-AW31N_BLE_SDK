#include "power_interface.h"
#include "app_config.h"
#include "gpio.h"
#include "sys_memory.h"
#include "gpadc.h"
#include "sys_timer.h"
#include "app_power_mg.h"
#include "btcontroller_modules.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

#define SLEEP_MIN_TIME_US    8000
//标识overlay代码是否需要reload
static u8 sleep_overlay_destroy;

//控制是否跑sleep代码检查,
//当确定场景不执行低功耗,可控制跑低功耗流程检查,目的可减少读flash代码操作的功耗
//可用于小周期持续传输数据的场景下控制使用,参考示例
static u8 sleep_check_enable = 1;

/*-----------------------------------------------------------------------
 */
//overlay 代码检查重新加载,softoff 和 poweroff都会用到
void sleep_overlay_check_reload(void)
{
    if (sleep_overlay_destroy) {
        log_info("sleep_overlay_reload");
        local_irq_disable();
#if CONFIG_APP_OTA_EN
        extern u8 rcsp_update_is_start(void);
        if (rcsp_update_is_start()) {
            local_irq_enable();
            return;
        }
#endif
        lowpower_init();
        sleep_overlay_destroy = false;
        local_irq_enable();
    }
}

//overlay 卸载,里面会有memset
void sleep_overlay_set_destroy(void)
{
    if (!sleep_overlay_destroy) {
        log_info("sleep_overlay_destroy");
        local_irq_disable();
        lowpower_uninit();
        sleep_overlay_destroy = true;
        local_irq_enable();
    }
}

//控制是否执行低功耗流程执行,以达到节省读flash代码执行的操作
void sleep_run_check_enalbe(u8 enable)
{
    sleep_check_enable = enable;
}
/*-----------------------------------------------------------------------
 *上层进入低功耗接口
 *
 */
void sys_power_down(u32 usec)
{
#if TCFG_LOWPOWER_LOWPOWER_SEL

    //SYS_IO_DEBUG_FLIP(A, 1);
    if (!sleep_check_enable) {
        __asm__ volatile("idle");
        return;
    }

#if TCFG_USER_BLE_ENABLE
    if (check_ble_bb_wakeup_event_hanppened() == false) {
        __asm__ volatile("idle");
    }

    //优先判断ble，减少查询代码的耗时
    //SYS_IO_DEBUG_H(A, 2);
    if (ble_bb_is_busy()) {
        return;
    }
    //SYS_IO_DEBUG_L(A, 2);
#else
    __asm__ volatile("idle");
#endif

    //    putchar('{');
    u32 sys_timer_idle_us;

    //step:3睡眠前做预擦除动作
    sysmem_pre_erase_api();

    //check overlay
    if (sleep_overlay_destroy) {
        sleep_overlay_check_reload();
    }

    if (usec == LOW_POWER_KEEP) {
        wdt_disable();//close dog
    } else {
        //step:1获取系统定时器可休眠时间
        sys_timer_idle_us = get_sys_timer_sleep_time(NULL);
        //step:2计算出系统可休眠时间
        usec = MIN(sys_timer_idle_us, usec);
    }

    if (usec < SLEEP_MIN_TIME_US) {
        return;
    }

    //SYS_IO_DEBUG_H(A, 4);
    //step:4进入睡眠
    vPortSuppressTicksAndSleep(usec);
    //SYS_IO_DEBUG_L(A, 4);

    if (usec == LOW_POWER_KEEP) {
        wdt_enable();//open dog
    }

#ifdef CONFIG_DEBUG_ENABLE
    lowpower_dump();
#endif
    //    adc_exit_power_down_update();

//    putchar('}');
#endif
}

static u8 sleep_check_idle_query(void)
{
    return sleep_check_enable;
}

REGISTER_LP_TARGET(sleep_check_en_target) = {
    .name = "sleep_check_en",
    .is_idle = sleep_check_idle_query,
};




