/**@file  		lrc_hw.h
* @author
* @date			2023-10-07
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2030  @ , All Rights Reserved.
* @brief	    hardware api
* @details
//=======================

 */
#ifndef __LVD_HW_H__
#define __LVD_HW_H__

/************************P3_VLVD_CON*****************************/
#define GET_VLVD_PND()         		((P33_CON_GET(P3_VLVD_CON) & BIT(7)) ? 1 : 0)

#define VLVD_PND_CLR()       		p33_fast_access(P3_VLVD_CON, BIT(6), 1)

#define VLVD_VOL_SEL(sel)			P33_CON_SET(P3_VLVD_CON, 3, 3, sel)

#define P33_VLVD_OE(en)				p33_fast_access(P3_VLVD_CON, BIT(2), en)

#define VLVD_EXPIN_EN(en)			p33_fast_access(P3_VLVD_CON, BIT(1), en)

#define P33_VLVD_EN(en)         	p33_fast_access(P3_VLVD_CON, BIT(0), en)

#define IS_LVD_EN()					(P33_CON_GET(P3_VLVD_CON) & BIT(0))

#endif
