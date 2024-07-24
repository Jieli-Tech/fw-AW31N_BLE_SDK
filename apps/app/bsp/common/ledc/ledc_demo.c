/*********************************************************************************************
 *   Filename        : ledc_test.c

 *   Description     : ledc功能示例

    *   Author          :

    *   Email           :

   *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/

#include "includes.h"
#include "cpu.h"
#include "ledc.h"
#include "gpio.h"
#include "stdlib.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[LEDC_TEST]"
#include "log.h"

#if LEDC_TEST_ENABLE
/****************** LEDC 推RGB灯  ******************/
static JL_LEDC_TypeDef *JL_LEDCx[2] = {
    JL_LEDC0,
    JL_LEDC1
};

#define LED_MAX_NUM      100      //灯带最大灯数
#define LED_MAX_BIT      (3 * 8)  //灯的类型:单个灯的位数,RGB(3*8),RGBW(4 * 8)

static u8 ledc_idle = 1; //置1可进睡眠,置0不可进睡眠
static u8 ledc_buf[LED_MAX_NUM * LED_MAX_BIT / 8] __attribute__((aligned(4)));

static int ledc_cur_gpio = -1;
static int ledc_cur_index = -1;
static int ledc_type = -1;
static volatile bool ledc_busy = 0;

#define LEDC_BUF_SIZE    sizeof(ledc_buf)
//需要根据灯的通信波形，修改结构体参数
static struct ledc_platform_data ledc_platform = {
    .index = 0,
    .port = -1,
    .idle_level = 0,
    .out_inv = 0,
    .bit_inv = 1,
    .t_unit = t_42ns,
    .t1h_cnt = 24,
    .t1l_cnt = 7,
    .t0h_cnt = 7,
    .t0l_cnt = 24,
    .t_rest_cnt = 2000,  /* 1000(46us) 2000(88us) 20000(876us) */
    .cbfun = NULL,
};

static void ledc_isr_cbfun(void)
{
    log_debug("%s", __func__);
    log_char('i');
    ledc_busy = 0;
}

// ledc外设固定是高位先发,顺序不一样，需要修改bit_inv
int ledc_open(u32 gpio) //单个灯的数据宽度
{
    log_info("%s[gpio:0x%x bits:%d]", __func__, gpio, LED_MAX_BIT);
    struct ledc_platform_data *arg = &ledc_platform;
    arg->port = gpio;
    arg->cbfun = ledc_isr_cbfun;
    ledc_cur_gpio = arg->port;
    ledc_cur_index = arg->index;
    ledc_type = LED_MAX_BIT;
    ledc_busy = 0;
    ledc_idle = 0;
    ledc_init(arg);
    //推灭所有灯数据
    memset(ledc_buf, 0x00, LEDC_BUF_SIZE);
    JL_LEDCx[ledc_cur_index]->ADR = (u32)ledc_buf;
    JL_LEDCx[ledc_cur_index]->FD = LED_MAX_NUM * 8;//ledc_type;
    JL_LEDCx[ledc_cur_index]->LP = 0;
    JL_LEDCx[ledc_cur_index]->CON |= BIT(0);//启动
    JL_LEDCx[ledc_cur_index]->CON &= ~BIT(5);//关闭中断
    while (!(JL_LEDCx[ledc_cur_index]->CON & BIT(7)));
    JL_LEDCx[ledc_cur_index]->CON |= BIT(6);
    return 0;
}
int ledc_close(void)
{
    log_info("%s", __func__);
    JL_LEDCx[ledc_cur_index]->CON = 0;
    ledc_idle = 1;
    ledc_cur_gpio = -1;
    ledc_cur_index = -1;
    ledc_type = -1;
    ledc_busy = 0;
    return 0;
}

//切换推灯IO, old_gpio_level:旧IO的电平, new_gpio:新IO
void ledc_switch_port(bool old_gpio_level, u32 new_gpio)
{
    log_debug("%s[old_gpio_level:%d new_gpio:0x%x]", __func__, old_gpio_level, new_gpio);
    //暂时不用IO，输出维持电平
    if (old_gpio_level) {
        gpio_set_mode(IO_PORT_SPILT(ledc_cur_gpio), PORT_OUTPUT_HIGH);
    } else {
        gpio_set_mode(IO_PORT_SPILT(ledc_cur_gpio), PORT_OUTPUT_LOW);
    }
    gpio_disable_fun_output_port(ledc_cur_gpio);
    //切换其他IO
    ledc_cur_gpio = new_gpio;
    gpio_set_mode(IO_PORT_SPILT(ledc_cur_gpio), PORT_OUTPUT_LOW);
    gpio_set_fun_output_port(ledc_cur_gpio, FO_LEDC_DOUT0 + ledc_cur_index, 0, 1);
}

int ledc_send_buf_nowait(u8 *buf, u32 len)  //立即返回,记得检查返回值
{
    log_debug("%s[len:%d]", __func__, len);
    if (buf == 0) {
        return -1;
    }
    if (len == 0) {
        return -2;
    }
    if (ledc_busy) {
        return -3;
    }
    if (len > LEDC_BUF_SIZE) {
        return -4;
    }
    ledc_busy = 1;
    memcpy(ledc_buf, buf, len);
    JL_LEDCx[ledc_cur_index]->ADR = (u32)ledc_buf;
    JL_LEDCx[ledc_cur_index]->FD = len * 8;//ledc_type;
    JL_LEDCx[ledc_cur_index]->LP = 0;
    JL_LEDCx[ledc_cur_index]->CON |= BIT(0);//启动
    JL_LEDCx[ledc_cur_index]->CON |= BIT(5);//使能中断
    return 0;
}

int ledc_send_buf_wait(u8 *buf, u32 len)  //阻塞形式,中断函数不可调用
{
    log_debug("%s[len:%d]", __func__, len);
    if (buf == 0) {
        return -1;
    }
    if (len == 0) {
        return -2;
    }
    if (ledc_busy) {
        return -3;
    }
    if (len > LEDC_BUF_SIZE) {
        return -4;
    }
    ledc_busy = 1;
    memcpy(ledc_buf, buf, len);
    JL_LEDCx[ledc_cur_index]->ADR = (u32)ledc_buf;
    JL_LEDCx[ledc_cur_index]->FD = len * 8;//ledc_type;
    JL_LEDCx[ledc_cur_index]->LP = 0;
    JL_LEDCx[ledc_cur_index]->CON |= BIT(0);//启动
    JL_LEDCx[ledc_cur_index]->CON &= ~BIT(5);//关闭中断
    while (!(JL_LEDCx[ledc_cur_index]->CON & BIT(7)));
    JL_LEDCx[ledc_cur_index]->CON |= BIT(6);
    ledc_busy = 0;
    return 0;
}

/*
*************************************************************
* 测量推灯时序耗时
* 20个灯珠的灯带, 80us + 20*24*1.2us == 80 + 576 == 656us,
* 实际波形测量, 88us + 20*24*1.3us == 88 + 624 == 712us
* 加上函数调用,buf赋值,切换IO等操作,耗时728us
*************************************************************
*/

//测试功能
#define SEND_BUF(buf, len)  ledc_send_buf_wait(buf, len)
/* #define SEND_BUF(buf, len)  ledc_send_buf_nowait(buf, len) */
static u8 led_rand(void)
{
    return rand32() % 0xFF;
}

static void ledc_test_loop(void *priv) // G R B
{
    static u32 step = 0;
    log_debug("%s[step:%d]", __func__, step);
    switch (step) { //测试3个灯，rgb格式，即9个字节
    case 0:
        step++;
        u8 buf1[] = {
            led_rand(), 0x00,       0x00,        //led0-G  led0-R  led0-B
            0x00,       led_rand(), 0x00,        //led1-G  led1-R  led1-B
            0x00,       0x00,       led_rand(),  //led2-G  led2-R  led2-B
        };
        SEND_BUF(buf1, sizeof(buf1));
        break;
    case 1:
        step++;
        u8 buf2[] = {
            0x00,       led_rand(), 0x00,        //led0-G  led0-R  led0-B
            0x00,       0x00,       led_rand(),  //led1-G  led1-R  led1-B
            led_rand(), 0x00,       0x00,        //led2-G  led2-R  led2-B
        };
        SEND_BUF(buf2, sizeof(buf2));
        break;
    case 2:
        step++;
        u8 buf3[] = {
            0x00,       0x00,       led_rand(),  //led0-G  led0-R  led0-B
            led_rand(), 0x00,       0x00,        //led1-G  led1-R  led1-B
            0x00,       led_rand(), 0x00,        //led2-G  led2-R  led2-B
        };
        SEND_BUF(buf3, sizeof(buf3));
        break;
    }
    if (step > 2) {
        step = 0;
    }
}

/*************************************************************************************************/
/*!
 *  \brief    LEDC测试函数，定时器调用
 *
 *  \param
 *
 *  \return
 *
 *  \note       内部调用
 */
/*************************************************************************************************/
int ledc_rgb_test(void)
{
    log_info("%s\r\n", __func__);
    ledc_open(IO_PORTA_04);
    sys_timer_add(NULL, ledc_test_loop, 500);
    return 0;
}

static u8 ledc_idle_query(void)
{
    if (ledc_idle) {
        return 1;
    } else {
        return 0;
    }
}
REGISTER_LP_TARGET(ledc_lp_target) = {
    .name = "ledc_lp",
    .is_idle = ledc_idle_query,
};

#endif
