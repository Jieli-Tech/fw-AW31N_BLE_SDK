/**@file  		p33_io_hw.h
* @brief        hw sfr layer
* @details
* @author	    sunlicheng
* @date     	2021-10-13
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */
#ifndef __P33_IO_HW_H__
#define __P33_IO_HW_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************P3_PCNT_CON*****************************/
#define PCNT_PND_CLR()		    	p33_fast_access(P3_PCNT_CON, BIT(6), 1)

/***************************PINR********************************/
#define IS_PINR_EN()				(P33_CON_GET(P3_PINR_CON) & BIT(0))

#define GET_PINR_PORT()				P33_CON_GET(P3_PORT_SEL0)

/***************************PINR1*******************************
#define  GET_PINR_PND1()			(P33_CON_GET(P3_PINR_PND1) & BIT(7) ? 1:0)

#define PINR_PND1_CPND()			p33_or_1byte(P3_PINR_PND1, BIT(6));
*/

/************************P3_PCNT_SET0*****************************/
#define SET_SOFT_RESET_FLAG(flag)	p33_tx_1byte(P3_SFLAG0, flag)

#define GET_SOFT_RESET_FLAG(flag)	p33_rx_1byte(P3_SFLAG0)

#define SOFT_RESET_FLAG_CLEAR()		p33_tx_1byte(P3_SFLAG0, 0);

//
//
//                 io_wakeup
//
//
//
//******************************************************************
/* digital
 */
//ie
#define P33_SET_WKUP_P_IE(data)		P33_CON_SET(P3_WKUP_P_IE0, 0, 8, data & 0xff)
#define P33_GET_WKUP_P_IE()			P33_CON_GET(P3_WKUP_P_IE0)
#define P33_OR_WKUP_P_IE(data)		p33_fast_access(P3_WKUP_P_IE0, data & 0xff, 1)
#define P33_AND_WKUP_P_IE(data)		p33_fast_access(P3_WKUP_P_IE0, data & 0xff, 0)

#define P33_SET_WKUP_N_IE(data)		P33_CON_SET(P3_WKUP_N_IE0, 0, 8, data & 0xff)
#define P33_GET_WKUP_N_IE()			P33_CON_GET(P3_WKUP_N_IE0)
#define P33_OR_WKUP_N_IE(data)		p33_fast_access(P3_WKUP_N_IE0, data & 0xff, 1)
#define P33_AND_WKUP_N_IE(data)		p33_fast_access(P3_WKUP_N_IE0, data & 0xff, 0)

//pnd
#define P33_GET_WKUP_P_PND()		(P33_CON_GET(P3_WKUP_P_PND0))
#define P33_GET_WKUP_N_PND()		(P33_CON_GET(P3_WKUP_N_PND0))
#define P33_SET_WKUP_P_CPND(data)	p33_fast_access(P3_WKUP_P_CPND0, data & 0xff, 1)
#define P33_SET_WKUP_N_CPND(data)	p33_fast_access(P3_WKUP_N_CPND0, data & 0xff, 1)

//flt
#define P33_SET_WKUP_FLT_EN(data)	P33_CON_SET(P3_WKUP_FLT_EN0, 0, 8, data & 0xff)
#define P33_OR_WKUP_FLT_EN(data)	p33_fast_access(P3_WKUP_FLT_EN0, data & 0xff, 1)
#define P33_AND_WKUP_FLT_EN(data)	p33_fast_access(P3_WKUP_FLT_EN0, data & 0xff, 0)
#define P33_SET_WKUP_CLK_SEL(data)	P33_CON_SET(P3_WKUP_CLK_SEL, 0, 2, data & 0x3)

//******************************************************************
/* analog
//ie
#define P33_SET_AWKUP_P_IE(data)	P33_CON_SET(P3_AWKUP_P_IE, 0, 8, data & 0xff)
#define P33_GET_AWKUP_P_IE()		P33_CON_GET(P3_AWKUP_P_IE)
#define P33_OR_AWKUP_P_IE(data)		p33_fast_access(P3_AWKUP_P_IE, data & 0xff, 1)
#define P33_AND_AWKUP_P_IE(data)	p33_fast_access(P3_AWKUP_P_IE, data & 0xff, 0)

#define P33_SET_AWKUP_N_IE(data)	P33_CON_SET(P3_AWKUP_N_IE, 0, 8, data & 0xff)
#define P33_GET_AWKUP_N_IE()		P33_CON_GET(P3_AWKUP_N_IE)
#define P33_OR_AWKUP_N_IE(data)		p33_fast_access(P3_AWKUP_N_IE, data & 0xff, 1)
#define P33_AND_AWKUP_N_IE(data)	p33_fast_access(P3_AWKUP_N_IE, data & 0xff, 0)

//pnd
#define P33_GET_AWKUP_P_PND()		(P33_CON_GET(P3_AWKUP_P_PND))
#define P33_GET_AWKUP_N_PND()		(P33_CON_GET(P3_AWKUP_N_PND))
#define P33_SET_AWKUP_P_CPND(data)	p33_fast_access(P3_AWKUP_P_CPND, data & 0xff, 1)
#define P33_SET_AWKUP_N_CPND(data)	p33_fast_access(P3_AWKUP_N_CPND, data & 0xff, 1)

#define P33_SET_AWKUP_FLT_EN(data)	P33_CON_SET(P3_AWKUP_FLT_EN, 0, 8, data & 0xff)
#define P33_OR_AWKUP_FLT_EN(data)	p33_fast_access(P3_AWKUP_FLT_EN, data & 0xff, 1)
#define P33_AND_AWKUP_FLT_EN(data)	p33_fast_access(P3_AWKUP_FLT_EN, data & 0xff, 0)
#define P33_SET_AWKUP_CLK_SEL(data)	P33_CON_SET(P3_AWKUP_CLK_SEL, 0, 2, data & 0x3)
 */

#endif

