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
#include "gpadc.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[tick]"
#include "log.h"

#define SEC_FOR_JIFFIES    // AT(.jiffies.text.cache.L2)

#define JIFFIES_MSEC_MASK         0x7FFFFFFUL
#define JIFFIES_MSEC_CIRCLE       ((JIFFIES_MSEC_MASK + 1) * 10)
#define JIFFIES_USEC_MASK         0x3FFFFUL
#define JIFFIES_USEC_CYCLE        ((JIFFIES_USEC_MASK + 1) * 10000)


//default 0
volatile u32 jiffies;
volatile u32 jiffies_2ms;
static u32 reserve_us;

static u32 prev_jiffies;
static u32 prev_tick_cnt;

#ifdef CONFIG_TICK_TIMER_INT_UNMASK_ENABLE
static u32 prev_tick_prd;
#endif

static u8 tick_power_resume; //低功耗唤醒标识,保证某些扫描要做一次; (range:0~2)

extern struct key_driver_para key_scan_para;
extern void app_timer_loop(void);


__attribute__((always_inline))
static void tick_increase_include(void)
{
    jiffies_2ms++;
    if (0 == (jiffies_2ms % (DEF_JIFFIES_MS / 2))) {
        jiffies++;
    }
}

/* ticktimer定时任务公共处理函数*/
void tick_timer_loop(void)
{
    tick_increase_include();

#if KEY_AD_EN || KEY_IO_EN
    if (tick_power_resume || (0 == (jiffies_2ms % 3))) {
        key_driver_scan(&key_scan_para);
    }
#endif

    if (tick_power_resume || (0 == (jiffies_2ms % 2))) {
        adc_scan(NULL);
    }

    //clear
    tick_power_resume = 0;

    u8 state = tick_timer_get_state();
    if (state == STATE_INIT) {
        return;
    }
    app_timer_loop();

}

SEC_FOR_JIFFIES
static void jiffies_2ms_and_tick_cnt(unsigned long *_jiffies, unsigned long *_usec, bool need_lock)
{
//tick is 2ms,not 10ms
    unsigned long jiffies_a, jiffies_b;
    unsigned long tick_cnt_b;
    unsigned long tick_prd_a, tick_prd_b;

    if (need_lock) {
        //spin_lock(NULL);
        local_irq_disable();
    }

    if (q32small(core_num())->TTMR_PRD < 10) {
        *_jiffies = jiffies_2ms;
        *_usec = 0;

        if (need_lock) {
            //spin_unlock(NULL);
            local_irq_enable();
        }
        return;
    }

    do {
        jiffies_a = jiffies_2ms;
        tick_prd_a = q32small(core_num())->TTMR_PRD;
        tick_cnt_b = q32small(core_num())->TTMR_CNT;
        tick_prd_b = q32small(core_num())->TTMR_PRD;
        jiffies_b = jiffies_2ms;
    } while (jiffies_a != jiffies_b || tick_prd_a != tick_prd_b);

    /* 检测tick_cnt溢出,jiffies还没有更新的情况 */
    if (jiffies_b == prev_jiffies) {
        if (tick_cnt_b < prev_tick_cnt) {
#ifdef CONFIG_TICK_TIMER_INT_UNMASK_ENABLE
            if (tick_prd_b == prev_tick_prd) {
                jiffies_b++;
            }
#endif
        }
    }

    if (q32small(core_num())->TTMR_CON & BIT(7)) {
        //溢出
        q32small(core_num())->TTMR_CON |=  BIT(6);
        tick_increase_include();//更新jiffies_2ms

        //更新赋值参数
        tick_cnt_b = q32small(core_num())->TTMR_CNT;
        tick_prd_b = q32small(core_num())->TTMR_PRD;
        jiffies_b = jiffies_2ms;
    }

#ifdef CONFIG_TICK_TIMER_INT_UNMASK_ENABLE
    prev_jiffies = jiffies_b;
    prev_tick_cnt = tick_cnt_b;
    prev_tick_prd = tick_prd_b;
#else
    if (jiffies_b == jiffies_a) {
        prev_jiffies = jiffies_b;
        prev_tick_cnt = tick_cnt_b;
    }
#endif

    if (need_lock) {
        //spin_unlock(NULL);
        local_irq_enable();
    }

    //tick is 2ms
    *_jiffies = jiffies_b;
    *_usec = tick_cnt_b * 1000 / (tick_prd_b / 2);
}

//不能直接引用或改动jiffies变量,需使用get或者set接口操作
SEC_FOR_JIFFIES
u32 get_jiffies(void)
{
    return maskrom_get_jiffies();
}

SEC_FOR_JIFFIES
void set_jiffies(u32 cnt)
{
    local_irq_disable();
    maskrom_set_jiffies_2ms(cnt * 5);
    maskrom_set_jiffies(cnt);
    local_irq_enable();
}

SEC_FOR_JIFFIES
unsigned long jiffies_msec(void)
{
    unsigned long cur_jiffies, usec;

    jiffies_2ms_and_tick_cnt(&cur_jiffies, &usec, !cpu_irq_disabled());

    return (cur_jiffies & JIFFIES_MSEC_MASK) * 2 + usec / 1000;
}

SEC_FOR_JIFFIES
int jiffies_msec2offset(unsigned long begin_msec, unsigned long end_msec)
{
    int offset = end_msec - begin_msec;
    if (end_msec < begin_msec) {
        if (begin_msec - end_msec >  JIFFIES_MSEC_CIRCLE / 2) {
            offset = JIFFIES_MSEC_CIRCLE - begin_msec + end_msec;
        }
    }
    return offset;
}

SEC_FOR_JIFFIES
unsigned long jiffies_offset2msec(unsigned long begin_msec, int offset_msec)
{
    unsigned long end_msec = begin_msec + offset_msec;
    if (end_msec > JIFFIES_MSEC_CIRCLE) {
        end_msec -= JIFFIES_MSEC_CIRCLE;
    }
    return end_msec;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief usec精度时间获取，用于操作系统任务调度时间统计
 *
 * @param NULL
 */
/* ----------------------------------------------------------------------------*/
SEC_FOR_JIFFIES
unsigned long jiffies_usec(void)
{
    unsigned long cur_jiffies, usec;
    jiffies_2ms_and_tick_cnt(&cur_jiffies, &usec, !cpu_irq_disabled());

    return (cur_jiffies & JIFFIES_USEC_MASK) * 2000 + usec;
}

SEC_FOR_JIFFIES
int jiffies_usec2offset(unsigned long begin, unsigned long end)
{
    int offset = end - begin;
    if (offset < 0) {
        if (begin - end > JIFFIES_USEC_CYCLE / 2) {
            offset = JIFFIES_USEC_CYCLE - begin + end;
        }
    }
    return offset;
}

SEC_FOR_JIFFIES
unsigned long jiffies_offset2usec(unsigned long base_usec, int offset_usec)
{
    unsigned long end_usec = base_usec + offset_usec;
    if (end_usec >= JIFFIES_USEC_CYCLE) {
        end_usec -= JIFFIES_USEC_CYCLE;
    }
    return end_usec;
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

//低功耗启动睡眠调用
void sleep_tick_compensate_post(void *priv, u32 usec)
{
    reserve_us += tick_timer_power_supend_post(priv, usec); //add tick time
}

//低功耗唤醒后时间补充
void sleep_tick_compensate_resume(void *priv, u32 usec)
{
    if (usec != (u32) - 1 && usec != (u32) - 2) {
        //log_info("usec:%d", usec);
        u32 sys_jiffies = maskrom_get_jiffies();
        u32 sys_jiffies_2ms = maskrom_get_jiffies_2ms();
        //BD47减1/4 tick用于唤醒后主动执行一次tick中断累加
        u32 tmp_usec = usec + reserve_us - (SET_TICK_TIME_US / 4);

        sys_jiffies = sys_jiffies + (tmp_usec / (1000 * 10));

        //计算剩余时间余数是否溢出10ms
        u32 remain_us = (tmp_usec % (1000 * 10)) + (sys_jiffies_2ms % 5) * SET_TICK_TIME_US;
        sys_jiffies  += remain_us / (1000 * 10);
        maskrom_set_jiffies(sys_jiffies);

        /* 补偿jiffies_2ms */
        sys_jiffies_2ms = sys_jiffies_2ms + (tmp_usec / SET_TICK_TIME_US);
        reserve_us = (tmp_usec % SET_TICK_TIME_US); //保留不足2ms的余数
        maskrom_set_jiffies_2ms(sys_jiffies_2ms);
    }
    tick_timer_power_resume(priv, usec);
    tick_power_resume = 1;
}


