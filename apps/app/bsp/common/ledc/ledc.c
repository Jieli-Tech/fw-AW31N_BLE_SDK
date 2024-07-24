/*********************************************************************************************
 *   Filename        : ledc.c

 *   Description     :ledc功能示例

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
#define LOG_TAG             "[LEDC]"
#include "log.h"

#if LEDC_TEST_ENABLE

#define LED_NUM_MAX     5
static u8 ledc_test_buf[3 * LED_NUM_MAX] __attribute__((aligned(4)));

static JL_LEDC_TypeDef *JL_LEDCx[2] = {
    JL_LEDC0,
    JL_LEDC1
};

static void (*ledc0_isr_cbfun)(void) = NULL;
static void (*ledc1_isr_cbfun)(void) = NULL;
static u8 t_div[9] = {1, 2, 3, 6, 12, 24, 48, 96, 192};

___interrupt
void ledc_isr(void)
{
    if (JL_LEDC0->CON & BIT(7)) {
        JL_LEDC0->CON |= BIT(6);
        if (ledc0_isr_cbfun) {
            ledc0_isr_cbfun();
        }
    }
    if (JL_LEDC1->CON & BIT(7)) {
        JL_LEDC1->CON |= BIT(6);
        if (ledc1_isr_cbfun) {
            ledc1_isr_cbfun();
        }
    }
}

void ledc_init(const struct ledc_platform_data *arg)
{
    gpio_set_mode(IO_PORT_SPILT(arg->port), PORT_OUTPUT_LOW);
    gpio_set_fun_output_port(arg->port, FO_LEDC_DOUT0 + arg->index, 0, 1);

    if (arg->cbfun) {
        if (arg->index == 0) {
            ledc0_isr_cbfun = arg->cbfun;
        } else {
            ledc1_isr_cbfun = arg->cbfun;
        }
    }
    request_irq(IRQ_LED_CON_IDX, IRQ_LEDC_IP, ledc_isr, 0);

    //std_48M
    JL_LEDCK->CLK &= ~(0b11 << 0);
    JL_LEDCK->CLK |= (0b11 << 0);
    //set div
    JL_LEDCK->CLK &= ~(0xff << 8);
    JL_LEDCK->CLK |= ((t_div[arg->t_unit] - 1) << 8);

    JL_LEDCx[arg->index]->CON = BIT(6);
    if (arg->idle_level) {
        JL_LEDCx[arg->index]->CON |= BIT(4);
    }
    if (arg->out_inv) {
        JL_LEDCx[arg->index]->CON |= BIT(3);
    }
    JL_LEDCx[arg->index]->CON |= (arg->bit_inv << 1);

    JL_LEDCx[arg->index]->TIX = 0;
    JL_LEDCx[arg->index]->TIX |= ((arg->t1h_cnt - 1) << 24);
    JL_LEDCx[arg->index]->TIX |= ((arg->t1l_cnt - 1) << 16);
    JL_LEDCx[arg->index]->TIX |= ((arg->t0h_cnt - 1) << 8);
    JL_LEDCx[arg->index]->TIX |= ((arg->t0l_cnt - 1) << 0);

    JL_LEDCx[arg->index]->RSTX = 0;
    JL_LEDCx[arg->index]->RSTX |= (arg->t_rest_cnt << 8);

    log_info("JL_LEDCK->CLK = 0x%x\n", JL_LEDCK->CLK);
    log_info("JL_LEDCx[%d]->CON = 0x%x\n", arg->index, JL_LEDCx[arg->index]->CON);
    log_info("JL_LEDCx[%d]->TIX = 0x%x\n", arg->index, JL_LEDCx[arg->index]->TIX);
    log_info("JL_LEDCx[%d]->RSTX = 0x%x\n", arg->index, JL_LEDCx[arg->index]->RSTX);
}

void ledc_rgb_to_buf(u8 r, u8 g, u8 b, u8 *buf, int idx)
{
    buf = buf + idx * 3;
    buf[2] = b;
    buf[1] = r;
    buf[0] = g;
}

void ledc_send_rgbbuf(u8 index, u8 *rgbbuf, u32 led_num, u16 again_cnt)
{
    if (!led_num) {
        return;
    }
    JL_LEDCx[index]->ADR = (u32)rgbbuf;
    JL_LEDCx[index]->FD = led_num * 3 * 8;
    JL_LEDCx[index]->LP = again_cnt;
    JL_LEDCx[index]->CON |= BIT(0);//启动
    JL_LEDCx[index]->CON &= ~BIT(5);//关闭中断
    while (!(JL_LEDCx[index]->CON & BIT(7)));
    JL_LEDCx[index]->CON |= BIT(6);
}

void ledc_send_rgbbuf_isr(u8 index, u8 *rgbbuf, u32 led_num, u16 again_cnt)
{
    if (!led_num) {
        return;
    }
    JL_LEDCx[index]->ADR = (u32)rgbbuf;
    JL_LEDCx[index]->FD = led_num * 3 * 8;
    JL_LEDCx[index]->LP = again_cnt;
    JL_LEDCx[index]->CON |= BIT(0);//启动
    JL_LEDCx[index]->CON |= BIT(5);//使能中断
}

/*******************************    参考TEST示例 ***********************************/
LEDC_PLATFORM_DATA_BEGIN(ledc0_data)
.index = 0,
 .port = IO_PORTA_04,
  .idle_level = 0,
   .out_inv = 0,
    .bit_inv = 1,
     .t_unit = t_42ns,
      .t1h_cnt = 24,
       .t1l_cnt = 7,
        .t0h_cnt = 7,
         .t0l_cnt = 24,
          .t_rest_cnt = 20000,
           .cbfun = NULL,
            LEDC_PLATFORM_DATA_END()

            /*************************************************************************************************/
            /*!
             *  \brief    LEDC测试函数，可放在app应用初始化前进行调用
             *
             *  \param
             *
             *  \return
             *
             *  \note       内部调用
             */
            /*************************************************************************************************/
            void ledc_test(void)
{
    log_info("*************  ledc test  **************\n");
    ledc_init(&ledc0_data);
    u8 r_val = 0;
    u8 g_val = 85;
    u8 b_val = 175;
    u16 sec_num = 5;//循环发送的次数，用于一条大灯带又分为几条效果一样的小灯带
    extern void wdt_clear();
    while (1) {
        wdt_clear();
        os_time_dly(1);
        r_val += 1;
        g_val += 1;
        b_val += 1;
        ledc_rgb_to_buf(r_val, g_val, b_val, ledc_test_buf, 0);
#if 0
        ledc_send_rgbbuf(0, ledc_test_buf, 1, sec_num - 1);
#else
        ledc_send_rgbbuf_isr(0, ledc_test_buf, 1, sec_num - 1);
#endif
    }
}

#endif

