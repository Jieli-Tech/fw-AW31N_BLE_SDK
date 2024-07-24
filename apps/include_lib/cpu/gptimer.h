#ifndef  __GPTIMER_H__
#define  __GPTIMER_H__

#include "gptimer_hw.h"
#include "gpio.h"

extern const u8 lib_gptimer_src_lsb_clk; //时钟源选择lsb_clk, 单位:MHz
extern const u8 lib_gptimer_src_std_clk; //时钟源选择std_x_clk, 单位:MHz
extern const u8 lib_gptimer_timer_mode_en; //gptimer timer功能使能
extern const u8 lib_gptimer_pwm_mode_en; //gptimer pwm功能使能
extern const u8 lib_gptimer_capture_mode_en; //gptimer capture功能使能
extern const u8 lib_gptimer_auto_tid_en; //gptimer_tid 内部自动分配使能
extern const u8 lib_gptimer_hw_callback_en[4]; //gptimer回调函数使能

enum gptimer_mode : u8 {
    GPTIMER_MODE_TIMER = 0, //定时模式
    GPTIMER_MODE_PWM, //pwm模式
    GPTIMER_MODE_CAPTURE_EDGE_RISE, //上升沿捕获
    GPTIMER_MODE_CAPTURE_EDGE_FALL, //下降沿捕获
    GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE, //双边沿捕获
};
enum gptimer_para : u8 { //从寄存器当前状态获取timer参数
    GPTIMER_PARA_PERIOD = 0,
    GPTIMER_PARA_FREQ,
    GPTIMER_PARA_DUTY,
    GPTIMER_PARA_CAPTURE,
};
enum gptimer_err : u8 { //返回值类型
    GPTIMER_ERR_INIT_FAIL = 0xFF,
};

typedef void (*timer_irq_callback)(u32 tid, void *private_data); //回调函数
struct gptimer_timer { //计时（计数）器配置
    u32 period_us;
};
struct gptimer_pwm { //pwm配置
    u32 freq;
    enum gpio_port port;
    u16 pin;
    u16 duty; //x10000
};
struct gptimer_capture { //捕获配置
    u32 filter; //需要滤除的频率，单位 Hz
    u32 max_period; //最大捕获周期, 单位:us, 需要用到才配置,默认配置0
    enum gpio_port port;
    u16 pin;
};

struct gptimer_config {
    union {
        struct gptimer_timer timer;
        struct gptimer_pwm pwm;
        struct gptimer_capture capture;
    };
    timer_irq_callback irq_cb;
    void *private_data; //用户参数
    u8 irq_priority;
    enum gptimer_mode mode; //工作模式
};

//以下为对外 api 接口---------------------------------------------------------
u32 gptimer_init(const timer_dev timerx, const struct gptimer_config *gt_cfg);
u32 gptimer_deinit(u32 tid);
u32 gptimer_start(u32 tid);
u32 gptimer_pause(u32 tid);
u32 gptimer_resume(u32 tid);
u32 gptimer_set_timer_period(u32 tid, u32 period_us);
u32 gptimer_get_timer_period(u32 tid);
u32 gptimer_set_pwm_freq(u32 tid, u32 freq);
u32 gptimer_get_pwm_freq(u32 tid);
u32 gptimer_set_pwm_duty(u32 tid, u32 duty);
u32 gptimer_get_pwm_duty(u32 tid);
u32 gptimer_pwm_flip(u32 tid);
u32 gptimer_pwm_enable(u32 tid);
u32 gptimer_pwm_disable(u32 tid);
u32 gptimer_set_capture_edge_type(u32 tid, enum gptimer_mode edge_type);
enum gptimer_mode gptimer_get_capture_edge_type(u32 tid);
u32 gptimer_set_irq_callback(u32 tid, void (*irq_cb)(u32 tid, void *private_data));
enum gptimer_mode gptimer_get_work_mode(u32 tid);
u32 gptimer_set_work_mode(u32 tid, enum gptimer_mode type);
u32 gptimer_set_capture_filter(u32 tid, u32 filter);
u32 gptimer_get_capture_filter(u32 tid);
u32 gptimer_set_capture_count(u32 tid, u32 cnt);
u32 gptimer_get_capture_count(u32 tid);
u32 gptimer_get_capture_cnt2us(u32 tid); //捕获发生时调用有效
u32 gptimer_set_count(u32 tid, u32 cnt);
u32 gptimer_get_count(u32 tid);
u32 gptimer_set_prd(u32 tid, u32 prd);
u32 gptimer_get_prd(u32 tid);
void *gptimer_get_private_data(u32 tid);
void gptimer_set_private_data(u32 tid, void *private_data);

u32 gptimer_measure_time_init(const timer_dev timerx, u32 max_time_us); //比实际时间少约40us
u32 gptimer_measure_time_start(u32 tid);
u32 gptimer_measure_time_end(u32 tid);



//以下为底层驱动接口，直接操作寄存器---------------------------------------------------------
u32 timer_hw_timer_init(GPTIMER *GPTIMERx, u32 period_us, u32 priority, void (*irq_cb)(u32 tid, void *private_data));
u32 timer_hw_pwm_init(GPTIMER *GPTIMERx, enum gpio_port port, u32 pin, u32 freq, u32 duty);
u32 timer_hw_capture_init(GPTIMER *GPTIMERx, enum gpio_port port, u32 pin, u32 edge_type, u32 filter, u32 max_period, u32 priority, void (*irq_cb)(u32 tid, void *private_data));
// void timer_hw_deinit(GPTIMER *GPTIMERx);
void timer_hw_timer_deinit(GPTIMER *GPTIMERx);
void timer_hw_pwm_deinit(GPTIMER *GPTIMERx, enum gpio_port port, u32 pin);
void timer_hw_capture_deinit(GPTIMER *GPTIMERx, enum gpio_port port, u32 pin, u32 filter);
void timer_hw_start(GPTIMER *GPTIMERx, enum gptimer_mode mode);
u32 timer_hw_pause(GPTIMER *GPTIMERx);
u32 timer_hw_set_cur_para(GPTIMER *GPTIMERx, enum gptimer_para para_type, u32 para0, u32 para1);
u32 timer_hw_get_cur_para(GPTIMER *GPTIMERx, enum gptimer_para para_type);
void timer_hw_pwm_flip(GPTIMER *GPTIMERx);
void timer_hw_pwm_enable(GPTIMER *GPTIMERx);
void timer_hw_pwm_disable(GPTIMER *GPTIMERx);
void timer_hw_set_irq_cb(GPTIMER *GPTIMERx, void (*irq_cb)(u32 tid, void *private_data));
void timer_hw_set_work_mode(GPTIMER *GPTIMERx, enum gptimer_mode type);
u32 timer_hw_get_work_mode(GPTIMER *GPTIMERx);
u32 timer_hw_get_capture_filter(GPTIMER *GPTIMERx);
void timer_hw_set_cnt(GPTIMER *GPTIMERx, int cnt);
u32 timer_hw_get_cnt(GPTIMER *GPTIMERx);
void timer_hw_set_prd(GPTIMER *GPTIMERx, u32 prd);
u32 timer_hw_get_prd(GPTIMER *GPTIMERx);

u32 timer_hw_set_clk(u32 *clk_src_set, u32 *clk_div_set, u32 para, enum gptimer_para para_type);
u32 hw_irflt_set_clk(u32 *clk_src_set, u32 *clk_div_set, u32 filter);
GPTIMER *timer_hw_tid2timer(u32 tid);
void timer_hw_clr_pnd(GPTIMER *GPTIMERx);
u32 timer_hw_idle_check(GPTIMER *GPTIMERx);
u32 timer_hw_tick2us(GPTIMER *GPTIMERx, u32 tick);

u32 timer_hw_measure_time_init(const timer_dev timerx, u32 max_time_us);
u32 timer_hw_measure_time_start(u32 tid);
u32 timer_hw_measure_time_end(u32 tid);

void gptimer_dump();

void gptimer_test_func(void);

#endif

