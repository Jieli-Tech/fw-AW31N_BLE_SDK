#if 0
#include "asm/rtc.h"
#include "asm/power_interface.h"
#include "app_config.h"

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[RTC_DEMO]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

void rtc_dev_init(const struct rtc_config_init *rtc);
void rtc_debug_dump(void);
void power_set_soft_poweroff();

static struct sys_time def_sys_time = {  //初始化系统时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static struct sys_time def_alarm = {     //初始化闹钟时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 5,
    .sec = 0,
};

struct sys_time rtc_time_test = {    //写时钟接口测试
    .year = 2024,
    .month = 11,
    .day = 30,
    .hour = 23,
    .min = 59,
    .sec = 50,
};

struct sys_time alarm_time_test = {  //写闹钟接口测试
    .year = 2024,
    .month = 12,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 10,
};

void rtc_alm_isr()    //闹钟回调函数测试
{
    log_info("alarm_timeout!!!\n\n");
}

struct rtc_config_init rtc_config_test = {   //RTC初始化结构体
    .default_sys_time = &def_sys_time,   //配置默认系统时钟
    .default_alarm = &def_alarm,        //配置默认闹钟
    .rtc_clk = CLK_SEL_LRC,       // 配置时钟源

    .alm_en = 1,            //闹钟使能
    .cbfun = rtc_alm_isr,
    //rtc闹钟回调函数
};

#define    SET_TIME_WAKEUP_RTC    0    //单独测试定时唤醒可打开这个


// 时间、闹钟打印，PA9可用于关机
void rtc_dump_test()
{
#if SET_TIME_WAKEUP_RTC
    vir_rtc_wakeup_enable(3000);
    power_set_soft_poweroff();
#else
    //时间打印
    rtc_debug_dump();
    JL_PORTA->DIR |= BIT(9);
    JL_PORTA->DIE |= BIT(9);
    JL_PORTA->PU0 |= BIT(9);
    JL_PORTA->PU1 &= ~BIT(9);
    JL_PORTA->PD0 &= ~BIT(9);
    JL_PORTA->PD1 &= ~BIT(9);
    if (!(JL_PORTA->IN & BIT(9))) {
        //soff
        power_set_soft_poweroff();
    }
#endif
}

void rtc_test(void)
{
#if SET_TIME_WAKEUP_RTC
    sys_timer_add(NULL, rtc_dump_test, 1000);
#else
    rtc_dev_init(&rtc_config_test);
    /* u32 is_rtc_wkup = rtc_is_alarm_wkup(); */
    /* log_info("\n\n\n\nis_rtc_wkup:%d\n", is_rtc_wkup); //0表示复位、1表示闹钟或定时唤醒、2表示rtc计时器溢出或者io唤醒 */
    log_info("rtc_clk_sel:%d\n", rtc_get_clk_sel()); //获取rtc时钟源
    log_info("rtc_is_alm_en:%d\n", rtc_is_alarm_en()); //获取闹钟是否使能

    struct sys_time tmp_time;
    //只在第一次上电配置
    if (is_reset_source(P33_VDDIO_POR_RST)) {
        void reset_source_dump();
        reset_source_dump();
        /********测试系统时钟读写接口********/
        rtc_read_time(&tmp_time);
        log_info("rtc_rtc_read_time_before: %d-%d-%d %d:%d:%d\n",
                 tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);
        rtc_write_time(&rtc_time_test);
        rtc_read_time(&tmp_time);
        log_info("rtc_rtc_read_time_after: %d-%d-%d %d:%d:%d\n",
                 tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);

        /********测试系统闹钟读写接口********/
        rtc_read_alarm(&tmp_time);                  //读当前alarm时间
        log_info("rtc_rtc_read_alarm_before: %d-%d-%d %d:%d:%d\n",
                 tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);
        rtc_write_alarm(&alarm_time_test);          //修改alarm时间
        rtc_read_alarm(&tmp_time);                  //读当前alarm时间
        log_info("rtc_rtc_read_alarm_after: %d-%d-%d %d:%d:%d\n",
                 tmp_time.year, tmp_time.month, tmp_time.day, tmp_time.hour, tmp_time.min, tmp_time.sec);

    }
    sys_timer_add(NULL, rtc_dump_test, 1000);
#endif
}
#endif
