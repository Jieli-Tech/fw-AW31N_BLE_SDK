#include "power_interface.h"
#include "app_config.h"
#include "gpio.h"
#include "sys_memory.h"
#include "adc_api.h"
#include "sys_timer.h"

#define LOG_TAG_CONST       PMU
#define LOG_TAG             "[PMU]"
#include "log.h"

/*用于调试低功耗异常时使用，可开启库的调试信息*/
/* const char debug_timeout = 0;           //打印蓝牙和系统分别可进入低功耗的时间(msec) */
/* const char debug_is_idle = 0;           //打印当前哪些模块处于busy,用于蓝牙已经进入sniff但系统无法进入低功耗的情况，如果usr_timer处于busy则会打印对应的func地址 */

/*-----------------------------------------------------------------------
 *上层进入低功耗接口
 *
 */
void vPortSuppressTicksAndSleep(u32 usec);
void sys_power_down(u32 usec)
{
    u32 sys_timer;

    if (usec == -2) {
        wdt_close();
    }

    //step:1获取系统定时器可休眠时间
    sys_timer = get_sys_timer_sleep_time(NULL);

    //step:2计算出系统可休眠时间
    usec = MIN(sys_timer, usec);

    //step:3睡眠前做预擦除动作
    /* sysmem_pre_erase_api(); */

    //step:4进入睡眠
    vPortSuppressTicksAndSleep(usec);

    lowpower_dump();

    adc_exit_power_down_update();
    /* wdt_init(WDT_8S); */
}

#include "tick_timer_driver.h"
static u32 reserve;
static u32 reserve_2ms;
void sleep_time_compensate_callback(void *priv, u32 usec)
{
    putchar('A');

    local_irq_disable();

    //log_info("usec:%d", usec);

    u32 sys_jiffies = maskrom_get_jiffies();
    u32 tmp_usec = usec + reserve;
    sys_jiffies = sys_jiffies + (tmp_usec / (1000 * 10));
    reserve = (tmp_usec % (1000 * 10)); //保留不足10ms的余数
    maskrom_set_jiffies(sys_jiffies);

    /* 补偿jiffies_2ms */
    sys_jiffies = maskrom_get_jiffies_2ms();
    tmp_usec = usec + reserve_2ms;
    sys_jiffies = sys_jiffies + (tmp_usec / (1000 * 2));
    reserve_2ms = (tmp_usec % (1000 * 2)); //保留不足2ms的余数
    maskrom_set_jiffies_2ms(sys_jiffies);

    local_irq_enable();

}

/* static enum LOW_POWER_LEVEL power_app_level(void) */
/* { */
/*     return LOW_POWER_MODE_SLEEP; */
/* } */

/* static u8 power_app_idle(void) */
/* { */
/*     return 1; */
/* } */
/*  */
/* REGISTER_LP_TARGET(power_app_lp_target) = { */
/*     .name = "power_app", */
/*     .level = power_app_level, */
/*     .is_idle = power_app_idle, */
/* }; */
