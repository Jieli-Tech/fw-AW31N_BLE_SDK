#ifndef __P33_LPTMR_HW_H__
#define __P33_LPTMR_HW_H__

#define pd_tmr0_init                            \
        /* TO FLAG      1bit RO  */ ( 0<<7 )  | \
        /* CLR TO PND   1bit RW  */ ( 1<<6 )  | \
        /* WKUP FLAG    1bit RO  */ ( 0<<5 )  | \
        /* CLR WKUP PND 1bit RW  */ ( 1<<4 )  | \
        /* TIMER WKUP EN1bit RW  */ ( 1<<3 )  | \
        /* RSC WKUP EN  1bit RW  */ ( 1<<2 )  | \
        /* CONTINUE     1bit RW  */ ( 0<<1 )  | \
        /* EN           1bit RW  */ ( 1<<0 )


#define LP_TMR0_EN(en)				p33_fast_access(P3_LP_TMR0_CON, BIT(0), en)
#define LP_TMR0_CTU(en)				p33_fast_access(P3_LP_TMR0_CON, BIT(1), en)
#define LP_TMR0_WKUP_IE(en) 		p33_fast_access(P3_LP_TMR0_CON, BIT(2), en)
#define LP_TMR0_TO_IE(en)			p33_fast_access(P3_LP_TMR0_CON, BIT(3), en)
#define LP_TMR0_CLR_MSYS_WKUP(a)	p33_fast_access(P3_LP_TMR0_CON, BIT(4), 1)
#define LP_TMR0_MSYS_WKUP()			(p33_rx_1byte(P3_LP_TMR0_CON) & BIT(5))
#define LP_TMR0_CLR_MSYS_TO(a)		p33_fast_access(P3_LP_TMR0_CON, BIT(6), 1)
#define LP_TMR0_MSYS_TO()			(p33_rx_1byte(P3_LP_TMR0_CON) & BIT(7))

#endif
