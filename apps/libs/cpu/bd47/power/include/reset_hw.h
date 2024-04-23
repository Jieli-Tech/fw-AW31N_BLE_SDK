#ifndef __RESET_HW__
#define __RESET_HW__

//
//
//			for reset_source
//
//
//
/************************P3_PR_PWR*****************************/
#define	P3_SOFT_RESET()				P33_CON_SET(P3_PR_PWR, 4, 1, 1)

#define MCLR_EN(en)					p33_fast_access(P3_PR_PWR, BIT(3), en)

#define IS_MCLR_EN()				((P33_CON_GET(P3_PR_PWR) & BIT(3)) ? 1 : 0)

/************************P3_RST_CON0*****************************/
#define DVDDOK_OE(en) 				p33_fast_access(P3_RST_CON0, BIT(4), en)

#define VLVD_WKUP_EN(en)			p33_fast_access(P3_RST_CON0,  BIT(3), en)

#define VLVD_RST_EN(en)				p33_fast_access(P3_RST_CON0,  BIT(2), en)

#define VLVD_EXPT_EN(en)			p33_fast_access(P3_RST_CON0,  BIT(1), en)

#define MSYS_TO_P33_RST_MASK(en)    p33_fast_access(P3_RST_CON0, BIT(0), en)

#define FAST_PU_SYS(en)           	p33_fast_access(P3_EFU_FLAG, BIT(0), en)

#endif
