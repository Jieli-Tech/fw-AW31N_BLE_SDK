/**@file  		lrc_hw.h
* @author
* @date			2023-10-07
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2030  @ , All Rights Reserved.
* @brief	    hardware api
* @details
//=======================

 */
#ifndef __LRC_HW_H__
#define __LRC_HW_H__


//-------------------------------------------------------------------
/* ERC
 */
#define ERC_Hz_DEFAULT    (32 * 1000L)

#define ERC_CON0_INIT	0x0b
#define ERC_CON1_INIT	0x16

#define ERC_INIT()		p33_tx_1byte(P3_ERC_CON0, ERC_CON0_INIT); \
        				p33_tx_1byte(P3_ERC_CON1, ERC_CON1_INIT)

#define ERC_EN(en)		p33_fast_access(P3_ERC_CON0, BIT(0), en)

#define IS_ERC_EN()		(p33_rx_1byte(P3_ERC_CON0) & BIT(0))

//-------------------------------------------------------------------
/* LRC
 */
#define LRC_Hz_DEFAULT    (200 * 1000L)

//最低温漂(1.0416%@-40~125C):
#define lrc_con0_init2 \
		/*UNUSED          1bit */ (0<<7) | \
		/*RC20OK_CAPS     3bit */ (2<<4) | \
		/*UNUSED          2bit */ (0<<2) | \
		/*RC20OK_RN_TRIM  1bit */ (1<<1) | \
		/*RC20OK_EN       1bit */ (1<<0)

#define lrc_con1_init2 \
		/*UNUSED          2bit */ (0<<6) | \
		/*RC20OK_RPPS     2bit */ (3<<4) | \
		/*UNUSED          2bit */ (0<<2) | \
		/*RC20OK_RNPS     2bit */ (0<<0)

#define LRC_INIT()		  p33_tx_1byte(P3_LRC_CON0, lrc_con0_init2); \
        				  p33_tx_1byte(P3_LRC_CON1, lrc_con1_init2)

#define LRC_EN(en)		  p33_fast_access(P3_LRC_CON0, BIT(0), en)

#define IS_LRC_EN()		  (p33_rx_1byte(P3_LRC_CON0) & BIT(0))

//--------------------------------------------------------------------
/* LP_RC
 */

//0: lrc 1:erc
#define LPRC_SEL(sel)		p33_fast_access(P3_CLK_CON0, BIT(7), sel)

#if PCONFIG_ERC_ENABLE

#define LPRC_Hz_DEFAULT		ERC_Hz_DEFAULT

#define LPRC_EN(en)	   LPRC_SEL(1); \
					   ERC_EN(en)

#define LPRC_INIT()	   ERC_INIT()

#define IS_LPRC_EN()   IS_ERC_EN()

#else

#define LPRC_Hz_DEFAULT		LRC_Hz_DEFAULT

#define LPRC_EN(en)	  	LPRC_SEL(0); \
						LRC_EN(en)

#define LPRC_INIT()	   LRC_INIT()

#define IS_LPRC_EN()   IS_LRC_EN()

#endif


//--------------------------------------------------------------------
/* RCH
 */
#define RCH_Hz_DEFAULT	16000000

#endif
