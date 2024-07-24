#include "gptimer.h"
#include "clock.h"
#include "log.h"

static void timer_callback_func_0(u32 tid, void *private_data)
{
    putchar('t');
    //可以翻转IO口测量时间
    gpio_set_mode(PORTA, BIT(0), PORT_OUTPUT_HIGH);
    gpio_set_mode(PORTA, BIT(0), PORT_OUTPUT_LOW);
}
static void timer_callback_func_1(u32 tid, void *private_data)
{
    putchar('T');
    //可以翻转IO口测量时间
    gpio_set_mode(PORTA, BIT(1), PORT_OUTPUT_HIGH);
    gpio_set_mode(PORTA, BIT(1), PORT_OUTPUT_LOW);
}
static void timer_callback_func_2(u32 tid, void *private_data)
{
    gpio_set_mode(PORTA, BIT(0), PORT_OUTPUT_HIGH);
    gpio_set_mode(PORTA, BIT(0), PORT_OUTPUT_LOW);
}

static void timer_demo()
{
    printf("timer_demo()\n");
    const struct gptimer_config timer_config = {
        .timer.period_us = 1 * 1000 * 1000, //定时周期, 1000000us
        .irq_cb = timer_callback_func_0, //设置中断回调函数
        .irq_priority = 1, //中断优先级
        .mode = GPTIMER_MODE_TIMER, //设置工作模式
        .private_data = NULL,
    };
    u32 id = gptimer_init(TIMER0, &timer_config); //初始化timer配置,成功会返回分配的timer_id, 失败返回-1
    printf("timer_demo id:%d\n", id);

    gptimer_start(id); //启动timer
    gptimer_set_timer_period(id, 500 * 1000); //设置定时周期为500ms
    u32 timer_us = gptimer_get_timer_period(id); //获取定时周期
    printf("timer period_us = %dus\n", timer_us);
    gptimer_set_irq_callback(id, timer_callback_func_1); //设置回调函数
}

static void pwm_demo()
{
    printf("pwm_demo()\n");
    const struct gptimer_config pwm_config = {
        .pwm.freq = 1 * 1000, //设置输出频率
        .pwm.duty = 3456, //设置占空比为34.56%
        .pwm.port = PORTA, //设置pwm输出IO_ PORTA_02
        .pwm.pin = BIT(2), //设置pwm输出IO_PORTA_02
        .mode = GPTIMER_MODE_PWM, //设置工作模式
    };
    u32 id = gptimer_init(TIMER1, &pwm_config);
    printf("pwm_demo id:%d\n", id);
    gpio_set_mode(PORTA, BIT(2), PORT_OUTPUT_LOW); //IO口设为输出

    gptimer_start(id); //启动timer
    gptimer_set_pwm_freq(id, 2000); //设置pwm频率
    u32 freq = gptimer_get_pwm_freq(id);
    printf("pwm freq = %dHz\n", freq);
    gptimer_set_pwm_duty(id, 7500); //设置pwm占空比
    u32 duty = gptimer_get_pwm_duty(id);
    printf("pwm duty = %d/10000\n", duty);
    gptimer_pwm_flip(id); //pwm翻转
}

static void capture_demo()
{
    printf("capture_demo()\n");
    const struct gptimer_config capture_config = {
        .capture.filter = 0,//100000, //设置滤波频率, 1000Hz
        .capture.max_period = 0,//110000, //最大计时周期, 单位us, 默认给0
        .capture.port = PORTA, //设置捕获口IO_PORTA_04
        .capture.pin = BIT(4), //设置捕获口IO_PORTA_04
        .irq_cb = timer_callback_func_2, //设置回调函数
        .irq_priority = 3, //设置捕获中断优先级
        .mode = GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE, //设置工作模式
    };
    u32 id = gptimer_init(TIMER2, &capture_config);
    printf("capture_demo id:%d\n", id);
    gpio_set_mode(PORTA, BIT(4), PORT_INPUT_FLOATING); //IO口设为浮空输入

    gptimer_start(id); //启动timer
    gptimer_set_capture_filter(id, 1000); //设置滤波参数为1000Hz
    gptimer_set_capture_count(id, 0); //设置cnt寄存器
    u32 cnt = gptimer_get_capture_count(id);
    printf("capture cnt = %d\n", cnt);
    u32 us = gptimer_get_capture_cnt2us(id); //获取捕获时间间隔, 单位:us
    printf("capture_us = %dus \n", us);
}

static void measure_demo()
{
    printf("measure_demo()\n");
    u32 id = gptimer_measure_time_init(TIMER0, 0);
    u32 time_us;
    while (1) {
        gptimer_measure_time_start(id);
        gpio_set_mode(PORTA, BIT(5), PORT_OUTPUT_HIGH);
        udelay(10000); //需配合示波器或逻辑分析仪测量IO间隔时间
        gpio_set_mode(PORTA, BIT(5), PORT_OUTPUT_LOW);
        time_us = gptimer_measure_time_end(id);
        printf("time_us = %d\n", time_us);
        udelay(500 * 1000);
        wdt_clear();
    }
}

static void other_api_demo()
{
    printf("other_demo()\n");
    u32 id = 0; // 这里仅演示接口用法, id 需 init 申请成功后才可使用
    u32 cnt = 0;
    gptimer_set_count(id, cnt); //设置cnt寄存器值
    cnt = gptimer_get_count(id); //获取cnt寄存器值
    printf("other_demo cnt:%d\n", cnt);

    u32 prd;
    gptimer_set_prd(id, prd); //设置prd寄存器值
    prd = gptimer_get_prd(id); //获取prd寄存器值
    printf("other_demo prd:%d\n", prd);

    u8 private_data[8];
    gptimer_set_private_data(id, private_data); //设置私有参数
    u8 *p = gptimer_get_private_data(id); //获取私有参数
    put_buf(p, 8);
}

//直接造作寄存器,实现特殊需求定时功能
static void custom_timer_demo()
{
    printf("custom_timer_demo()\n");
    //以 TIMER0 输出 pwm 为例, 输出7.68KHz, 占空比50.00%的pwm
    //根据芯片型号选择不同的demo
#if 0 //适用于bd47
    SFR(JL_PLL0->CON1, 15, 1, 1); //sys_pll_d2p5 时钟使能
    SFR(JL_CLOCK->STD_CON1, 10, 4, 6); //clk_out2 选择时钟源为 sys_pll_d2p5
    SFR(JL_CLOCK->STD_CON1, 14, 6, 9); //clk_out2 选择预分频系数为 (9+1)
    //按如上配置,系统时钟为192 * 1000000 Hz时,clk_out2 = 192000000 / 2.5 / 10 = 7.68 * 1000000 Hz

    SFR(JL_TIMER0->CON, 10, 4, 8); //timer 选择时钟源为 clk_out2
    SFR(JL_TIMER0->CON, 4, 4, 0); //timer 选择预分频系数为 1
    //按如上配置,timer_clk = 7.68 * 1000000 / 1 = 7.68 * 1000000 Hz
    //注意:最终需要满足 timer_clk <= (lsb_clk / 2)

    SFR(JL_TIMER0->CON, 8, 1, 1); //pwm功能使能
    JL_TIMER0->PRD = 1000; //设置 PRD 寄存器为 1000, pwm输出频率为 7.68 * 1000000 / 1000 = 7.68 * 1000 Hz;
    JL_TIMER0->PWM = 500; //设置 PWM 寄存器为 500, pwm输出占空比为 500 / 1000 * 100% = 50.00%

    gpio_set_function(IO_PORT_SPILT(IO_PORTA_00), PORT_FUNC_TIMER0_PWM); //pwm配置输出IO
    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_00), PORT_OUTPUT_LOW);
    SFR(JL_TIMER0->CON, 0, 2, 0b01); //timer启动
#endif
}

void gptimer_test_demo()
{
    /* timer_demo(); */
    /* pwm_demo(); */
    /* capture_demo(); */
    /* measure_demo(); */
    /* other_api_demo(); */
    /* custom_timer_demo(); */

    while (1) {
        wdt_clear();
    }
}
