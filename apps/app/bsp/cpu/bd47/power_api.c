#include "power_interface.h"
#include "app_config.h"
#include "gpio.h"
#include "sys_memory.h"
#include "adc_api.h"
#include "sys_timer.h"
#include "app_power_mg.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

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
    if (!sleep_check_enable) {
        return;
    }

    //    putchar('{');
    u32 sys_timer;

    //check overlay
    sleep_overlay_check_reload();

    if (usec == LOW_POWER_KEEP) {
        wdt_disable();
    } else {
        //step:1获取系统定时器可休眠时间
        sys_timer = get_sys_timer_sleep_time(NULL);
        //step:2计算出系统可休眠时间
        usec = MIN(sys_timer, usec);
    }

    //step:3睡眠前做预擦除动作
    /* sysmem_pre_erase_api(); */
    //step:4进入睡眠
    vPortSuppressTicksAndSleep(usec);

    if (usec == LOW_POWER_KEEP) {
        wdt_enable();
    }

    lowpower_dump();
    adc_exit_power_down_update();

//    putchar('}');
#endif
}


