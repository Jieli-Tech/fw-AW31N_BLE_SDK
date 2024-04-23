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

volatile u32 jiffies = 0;
volatile u32 jiffies_2ms = 0;
u8 tick_cnt;
extern void app_timer_loop(void);
extern struct key_driver_para key_scan_para;
/* ticktimer定时任务公共处理函数,tick_cnt只能由该函数改变 */
void tick_timer_loop(void)
{
    tick_cnt ++;
    jiffies_2ms++;
    if (0 == (tick_cnt % 5)) {
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
    mdelay(10);
#endif
}

void os_time_dly(u32 tick)
{
    delay_10ms(tick);
}
