#pragma bss_seg(".ttmr_sys.data.bss")
#pragma data_seg(".ttmr_sys.data")
#pragma const_seg(".ttmr_sys.text.const")
#pragma code_seg(".ttmr_sys.text")
#pragma str_literal_override(".ttmr_sys.text.const")

#include "tick_timer_driver.h"
#include "clock.h"
#include "printf.h"
#include "jiffies.h"
#include "msg.h"
#include "key.h"
#include "adc_api.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[tick]"
#include "log.h"

//default 0
volatile u32 jiffies;
volatile u32 jiffies_2ms;
static u32 reserve;
static u32 reserve_2ms;
u8 tick_cnt;

extern struct key_driver_para key_scan_para;
extern void app_timer_loop(void);

/* ticktimer定时任务公共处理函数,tick_cnt只能由该函数改变 */
void tick_timer_loop(void)
{
    tick_cnt ++;
    jiffies_2ms++;
    if (0 == (tick_cnt % (DEF_JIFFIES_MS / 2))) {
        jiffies++;
    }
    if (0 == (tick_cnt % 3)) {
        key_driver_scan(&key_scan_para);
    }
    if ((0 == (tick_cnt % 2))) {
        adc_scan();
    }

    u8 state = tick_timer_get_state();
    if (state == STATE_INIT) {
        return;
    }
    app_timer_loop();

}

void delay_10ms(u32 tick)
{
#if 0
    u32 jiff = maskrom_get_jiffies();
    tick = tick + jiff;
    while (1) {
        jiff = maskrom_get_jiffies();
        if (time_after(jiff, tick)) {
            break;
        }
    }
#else
    mdelay(tick * 10);
#endif
}

void os_time_dly(u32 tick)
{
    delay_10ms(tick);
}

//低功耗唤醒后时间补充
void sleep_tick_compensate_resume(void *priv, u32 usec)
{
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

}


