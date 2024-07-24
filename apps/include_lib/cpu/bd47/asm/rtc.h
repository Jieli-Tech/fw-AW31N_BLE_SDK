#ifndef __RTC_H__
#define __RTC_H__

#include "typedef.h"

#define CLK_SEL_LRC     1
#define CLK_SEL_32K     2
#define CLK_SEL_BTOSC   3

#define USE_VIR_RTC     1

extern const int config_rtc_enable;

struct rtc_config_init {
    const struct sys_time *default_sys_time;
    const struct sys_time *default_alarm;
    void (*cbfun)(void);
    u32 rtc_clk;
    u8 alm_en;
};

struct sys_time {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
} _GNU_PACKED_;

//API
void vir_rtc_wakeup_enable(u32 wkup_ms);//softoff定时唤醒配置，wkup_ms单位:ms
void vir_rtc_wakeup_disable(void);

void rtc_dev_init(const struct rtc_config_init *rtc);
void rtc_dev_deinit(void);
void rtc_read_time(struct sys_time *time);
void rtc_write_time(const struct sys_time *time);
void rtc_read_alarm(struct sys_time *time);
void rtc_write_alarm(const struct sys_time *time);
void rtc_debug_dump(void);
void rtc_alarm_switch(u32 en);
void rtc_save_context_to_vm(void);
void rtc_reset_save_time(void);
u32 rtc_is_alarm_en(void);
u32 rtc_is_alarm_wkup(void);
u32 rtc_get_clk_sel(void);



bool leapyear(u32 year); //判断是否为闰年
u32 year_to_day(u32 year);
u32 month_to_day(u32 year, u32 month);
void day_to_ymd(u32 day, struct sys_time *sys_time);
u32 ymd_to_day(struct sys_time *time);
u32 caculate_weekday_by_time(struct sys_time *r_time); //计算当天为星期几
u32 get_day_of_month(u32 year, u32 month); //返回每月的天数





#endif // __RTC_API_H__
