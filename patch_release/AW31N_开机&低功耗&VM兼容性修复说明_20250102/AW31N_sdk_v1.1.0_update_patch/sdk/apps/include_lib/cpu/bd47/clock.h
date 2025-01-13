/*********************************************************************************************
    *   Filename        : typedef.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2016-07-12 10:53

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _CLOCK_H_
#define _CLOCK_H_
#include "typedef.h"
/*
 * system enter critical and exit critical handle
 * */
struct clock_critical_handler {
    void (*enter)();
    void (*exit)();
};

#define HSB_CRITICAL_HANDLE_REG(name, enter, exit) \
	const struct clock_critical_handler hsb_##name \
		 SEC_USED(.hsb_critical_txt) = {enter, exit};

extern struct clock_critical_handler hsb_critical_handler_begin[];
extern struct clock_critical_handler hsb_critical_handler_end[];

#define list_for_each_loop_hsb_critical(h) \
	for (h=hsb_critical_handler_begin; h<hsb_critical_handler_end; h++)

#define LSB_CRITICAL_HANDLE_REG(name, enter, exit) \
	const struct clock_critical_handler lsb_##name \
		 SEC_USED(.lsb_critical_txt) = {enter, exit}

extern struct clock_critical_handler lsb_critical_handler_begin[];
extern struct clock_critical_handler lsb_critical_handler_end[];

#define list_for_each_loop_lsb_critical(h) \
	for (h=lsb_critical_handler_begin; h<lsb_critical_handler_end; h++)

enum CLK_OUT_SOURCE {
    CLK_OUT_DISABLE,
    CLK_OUT_RTC_OSC,
    CLK_OUT_LRC_CLK,
    CLK_OUT_STD_12M,
    CLK_OUT_BTOSC_24M,
    CLK_OUT_BTOSC_48M,
    CLK_OUT_HSB,
    CLK_OUT_LSB,
    CLK_OUT_PLL_96M,
    CLK_OUT_RC250K,
    CLK_OUT_RC16M,
    CLK_OUT_DISABLE1,
    CLK_OUT_RF_CKO75M,
    CLK_OUT_XOSC_FSCK,
    CLK_OUT_USB_CLK,
};
enum CLK_OUT2_SOURCE {
    CLK_OUT2_DISABLE,
    CLK_OUT2_RTC_OSC,
    CLK_OUT2_LRC_CLK,
    CLK_OUT2_STD_24M,
    CLK_OUT2_STD_48M,
    CLK_OUT2_SYSPLL_D3P5,
    CLK_OUT2_SYSPLL_D2P5,
    CLK_OUT2_SYSPLL_D2P0,
    CLK_OUT2_SYSPLL_D1P5,
    CLK_OUT2_SYSPLL_D1P0,
};


#define MHz_UNIT    (1000000L)
#define KHz_UNIT    (1000L)
#define MHz	        (1000000L)

enum clk_mode {
    CLOCK_MODE_ADAPTIVE = 0,
    CLOCK_MODE_USR,
};

enum pll_ref_source {
    PLL_REF_XOSC,       //外部晶振，单端模式
    PLL_REF_XOSC_DIFF,  //外部晶振，差分模式
    PLL_REF_LRC,
    PLL_REF_HRC,
    PLL_REF_RTC_OSC,
    PLL_REF_XCLK,
};


enum syspll_ref_sel {
    SYSPLL_REF_SEL_LRC_200K = 0x0,
    SYSPLL_REF_SEL_EXT_CLK,
    SYSPLL_REF_SEL_BTOSC_24M,
    SYSPLL_REF_SEL_PAT_CLK,
    SYSPLL_REF_SEL_RC_16M, // SYSPLL_REF_SEL_BTOSC_DIFF,
};

void lrc_init(void);
void lrc200k_init();
void xosc_2pin_init_stable(); //fast mode 24M
void xosc_2pin_init_normal(); //normal mode 24M
void xosc_1pin_init_stable(); //fast mode 24M
void xosc_1pin_init_normal(); //normal mode 24M
u8 btosc_1pin_cfg(u32 lrc_freq);
u8 get_osc_1pin_sta();
void clk_voltage_init(u8 mode, u8 sys_dvdd);
int clk_early_init(enum pll_ref_source pll_ref, u32 ref_frequency, u32 pll_frequency);
int clk_set(const char *name, int clk);
int clk_get(const char *name);
void clock_set_sfc_max_freq(u32 max_freq);
void clock_set_p33_max_freq(u32 max_freq);
void clock_bt_init();
void update_vdd_table(u8 val);

u32 sys_clock_get(void);
void clock_dump(void);

void clk_out0(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out2(u8 gpio, enum CLK_OUT2_SOURCE clk, u32 div);
void clk_out0_close(u8 gpio);
void clk_out1_close(u8 gpio);
void clk_out2_close(u8 gpio);

void delay(u32 cnt);
void udelay(u32 us);
void mdelay(u32 ms);
void rc_udelay(u32 us);
u32 get_sys_us_cnt(void);


extern void IcuPfetchRegion(u32 *beg, u32 len);

enum xosc_oe_type {
    //wla8
    XOSC_OE_BT = 0x20,
    XOSC_OE_CK1X = 0x40,
    XOSC_OE_CK2X = 0x80,
    //wla9
    XOSC_OE_PMU = 0x0101,
    XOSC_OE_SYS = 0x0120,
    XOSC_OE_TEST = 0x0140,
};
void xosc_oe_set(enum xosc_oe_type oe_type, u8 en);

void clock_adaptive_sw(u32 sw);

u32 clk_set_max_frequency(u32 freq);

#endif

