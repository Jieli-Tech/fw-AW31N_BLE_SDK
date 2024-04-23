/**@file  		p33_hw.h
* @brief        hw sfr layer
* @details	   	p33 analog \ lowpower flow
* @author		app & ic
* @date     	2021-10-13
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */
#ifndef __P33_HW_H__
#define __P33_HW_H__

//
//						analog
//
//-----------------------------------------------------------------------------
/*
 * clock:
 * rc250k：内置RC震荡器，受温度电压影响大，提供给PMU使用（低功耗、唤醒、复位）
 * lrc：内置RC震荡器，受温度电压影响小，提供给PMU/主系统外设/PLL
 * xosc_32k：RTC外挂32K晶振，提供给PMU/主系统外设使用
 * d2sh：单端晶振，来自蓝牙震荡器（btosc--->btosc pmu_oe开关---->d2sh使能开关）
 */
#define RC250K_EN(en)				p33_fast_access(P3_ANA_FLOW0, BIT(7), en)

#define CLOCK_KEEP(en)				p33_fast_access(P3_ANA_FLOW2, BIT(4), en)

//
//			p33 analog
//
//----------------------------------------------------------------------
/*
 * ana flow
 */
/*
 * wvddio，弱vddio，低功耗时使用，参考源为wvbg
 * mvddio，主vddio，运行时使用，参考源为mvbg
 *
 * 上电将wvddio配置成最低档，进入低功耗时配置成对应挡位
 *
 * 进入低功耗，keep mvddio，需要保持mvddio_en、mvbg、mvio_ifull、mvio_vlmt
 * 进入低功耗，keep charge，需要保持mvddio
 */
#define MVDDIO_EN(en)				p33_fast_access(P3_ANA_FLOW0, BIT(2), en)

#define MVIO_IFULLEN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(1), en)

#define MVIO_VLMTEN(en)				p33_fast_access(P3_ANA_FLOW1, BIT(0), en)

#define WVDDIO_EN(en)				p33_fast_access(P3_ANA_FLOW0, BIT(5), en)

/*
 * wvdd，低功耗提供给数字模块的电源
 *
 * swvld_auto_en：wvdd auto short dvdd，当进入poff模式时需要配置为1，
 * 若有p11 cpu，不需要配置，由p11 cpu执行wvdd断开/短接dvdd流程
 */
#define WVDD_LOAD_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(5), en)

#define WVDD_EN(en)					p33_fast_access(P3_ANA_FLOW1, BIT(4), en)

#define SWVLD_AUTO_EN(en)			p33_fast_access(P3_ANA_FLOW2, BIT(7), en)

/*
 * dcvdd
 */
#define DCVD_IFULLEN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(2), en)

/*
 * lpmr是p33的数字复位，低功耗流程逻辑掉电时复位主系统/p11
 *
 * lpmr_rst_dly表示延时释放复位，等待模拟模块使能之后才释放 \
 *
 * 低功耗流程由以下组成：
 * 1.配置唤醒
 * 2.时钟流程
 * 3.模拟模块流程
 * 4.电源流程
 *
 * 实际上数字掉电的情况可以由por上电复位/pmu数字复位，具体设计看哪种
 * 复位源看哪种复位先释放。
 *
 */
#define LPMR_RST_DLY(en)			p33_fast_access(P3_ANA_FLOW2, BIT(5), en)

#define VCM_DET_EN(en)				p33_fast_access(P3_ANA_FLOW2, BIT(3), en)

#define IS_VCM_EN()          		((P33_CON_GET(P3_ANA_FLOW2) & BIT(3)) ? 1:0)

#define LOWPOWER_FAST_WAKEUP(en)	p33_fast_access(P3_ANA_FLOW2, BIT(2), en)

//
//			power control
//
//----------------------------------------------------------------------
/*
 * vddio
 */
#define VDDIOW_VOL_SEL(sel)     	P33_CON_SET(P3_IOV_CON0, 4, 4, sel)

#define GET_VDDIOW_VOL_SEL()		(P33_CON_GET(P3_IOV_CON0)>>4 & 0xf)

#define VDDIOM_VOL_SEL(sel)     	P33_CON_SET(P3_IOV_CON0, 0, 4, sel)

#define GET_VDDIOM_VOL_SEL()        (P33_CON_GET(P3_IOV_CON0) & 0xf)

#define VDDIO_HD_SEL(hd)       		P33_CON_SET(P3_IOV_CON1, 0, 2, hd)

//----------------------------------------------------------------------
/*
 * dcvdd
 */
#define DCVD_HD_SEL(sel)			P33_CON_SET(P3_DCV_CON0, 4, 2, sel)

#define DCVDD_LOAD_EN(en)			p33_fast_access(P3_DCV_CON0, BIT(2), en)

#define DCVDD_VOL_SEL(sel)      	P33_CON_SET(P3_DCV_CON1, 0, 4, sel)

#define DCVDD_LEVEL_DEFAULT			5

#define GET_DCVDD_VOL_SEL()      	(P33_CON_GET(P3_DCV_CON1) & 0xf)

//----------------------------------------------------------------------
/*
 * dvdd
 */
#define DVDD_HD_SEL(sel)  			P33_CON_SET(P3_DVD_CON0, 4, 2, sel)

#define DVDD_VOL_SEL(sel)     		P33_CON_SET(P3_DVD_CON0, 0, 4, sel)

#define DVDD_LEVEL_DEFAULT			9

#define GET_DVDD_VOL_SEL()     		(P33_CON_GET(P3_DVD_CON0) & 0xf)

#define DVDD_LOAD_EN(en)			p33_fast_access(P3_DVD_CON1, BIT(3), en)

//----------------------------------------------------------------------
/*
 * vld keep，主要配置PMU
 */

#define VLD_KEEP_RTC_WKUP(en)		p33_fast_access(P3_VLD_KEEP, BIT(7), en)

#define VLD_KEEP_WDT_EXPT(en)		p33_fast_access(P3_VLD_KEEP, BIT(6), en)

#define VLD_KEEP_VDD_LEL(en)		p33_fast_access(P3_VLD_KEEP, BIT(5), en)

#define VLD_KEEP_SYS_RST(en)		p33_fast_access(P3_VLD_KEEP, BIT(4), en)

#define VLD_KEEP_PWM_CLK(en)		p33_fast_access(P3_VLD_KEEP, BIT(3), en)

#define VLD_KEEP_RCLK_DIS(en)		p33_fast_access(P3_VLD_KEEP, BIT(2), en)

#define VLD_KEEP_WKUP(en)			p33_fast_access(P3_VLD_KEEP, BIT(1), en)

#define VLD_KEEP_CLK(en)			p33_fast_access(P3_VLD_KEEP, BIT(0), en)

//
//			analog control
//
//----------------------------------------------------------------------
#define DVDDLS_OR_BIT(a)    \
    p33_or_1byte(P3_LS_IO_USR    , BIT(a));  \
    p33_or_1byte(P3_LS_IO_ROM    , BIT(a));  \
    p33_or_1byte(P3_LS_IO_PINR   , BIT(a));

#define DVDDLS_AND_NBIT(a)    \
    p33_and_1byte(P3_LS_IO_USR    , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_IO_ROM    , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_IO_PINR    , (u8)~BIT(a));

#define DVDDLS_TX_BYTE(a)    \
    p33_tx_1byte(P3_LS_IO_USR    , a);  \
    p33_tx_1byte(P3_LS_IO_ROM    , a);  \
    p33_tx_1byte(P3_LS_IO_PINR   , a);

#define ROMLS_OR_BIT(a)    \
    p33_or_1byte(P3_LS_IO_ROM    , BIT(a));  \

#define ROMLS_ANA_NBIT(a)    \
    p33_and_1byte(P3_LS_IO_ROM    , (u8)~BIT(a));  \

#define ROMLS_TX_BYTE(a)    \
    p33_tx_1byte(P3_LS_IO_ROM    , a);  \

#endif
