#ifndef TICK_TIMER_H
#define TICK_TIMER_H
#include "config.h"

extern volatile u32 jiffies;
#define maskrom_get_jiffies() jiffies
#define maskrom_set_jiffies(n) jiffies = n

extern volatile u32 jiffies_2ms;
#define maskrom_get_jiffies_2ms() jiffies_2ms
#define maskrom_set_jiffies_2ms(n) jiffies_2ms = n

#define SET_TICK_TIME_MS           2//tick_time_irq
#define SET_TICK_TIME_US           (SET_TICK_TIME_MS*1000)
extern void tick_timer_init(void);
enum tick_timer_state {
    STATE_NOINIT = 0,
    STATE_INIT,
    STATE_RAM,
    STATE_SFC,
};

void tick_timer_set_state(u8 state);
u32 tick_timer_get_state(void);
u32 tick_timer_power_supend_post(void *priv, u32 usec);
u8 tick_timer_power_resume(void *priv, u32 usec);

// void tick_timer_sleep_init();
// extern void maskrom_update_jiffies(void);
// extern void maskrom_set_jiffies(u32 cnt);
// extern u32 maskrom_get_jiffies(void);
// void set_jiffies(u32 cnt);
// u32 get_jiffies(void);
// void delay_10ms(u32 tick);
// void os_time_dly(u32 tick);
// extern __attribute__((weak)) void tick_timer_set_on(void);
// extern __attribute__((weak)) void tick_timer_set_off(void);
// extern __attribute__((weak)) bool tick_timer_close(void);

#define COUNTDOWN_BY_JIFFIES(last_time, count_down)  \
	do { \
		u32 cur_jiffies = maskrom_get_jiffies(); \
		u32 pass_jiffies = cur_jiffies - last_time; \
		last_time = cur_jiffies; \
		if (count_down <= pass_jiffies) {  \
			count_down = 0; \
		} else {   \
			count_down -= pass_jiffies; \
		} \
	} while(0);
#endif
