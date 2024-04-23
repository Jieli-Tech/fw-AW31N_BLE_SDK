/**@file  	    misc_hw.h
* @brief        hw sfr layer
* @details
* @author		sunlicheng
* @date     	2022-08-31
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */

#ifndef __MISC_HW__
#define __MISC_HW__

//
//
//                    pmu
//
//
//
//******************************************************************
#define CPU_SLEEP()							\
		JL_CLOCK->PWR_CON |= BIT(2);

#define SYS_WK_DLY(en)						\
		if(en){ 							\
			JL_CLOCK->PWR_CON &= ~BIT(3); 	\
		}else{ 								\
    		JL_CLOCK->PWR_CON |= BIT(3); 	\
		}

#if CONFIG_P11_ENABLE

//----------------------------------------------------------------
#define P11_RST     		(JL_PMU->PMU_STA & BIT(5))
#define P11_KEY     		(JL_PMU->PMU_STA & BIT(0))

//----------------------------------------------------------------
#if CONFIG_P11_CPU_ENABLE
void  LLP_KICK_START();
void  LP_KICK_START();
void  PF_KICK_START();
void P11_SF_KICK_START();
void P33_RESUME();
void LLP_WAIT_WKUP_1();
void LP_WAIT_WKUP_1();
void PF_WAIT_WKUP();
void LP_WAIT_WKUP_2();
#endif

#define SOFF_KST			P33_SF_KICK_START()
#define LP_TMR_KST(x)       SFR(JL_PMU->PMU_CON, 9, 1, x)

#else

//----------------------------------------------------------------
#define LP_KST              JL_PMU->PMU_CON |= BIT(6)

#define PDOWN_KST			p33_tx_1byte(P3_PMU_CON4, pd_con4_init_sw_kst_lp);\
							LP_KST\

#define POFF_KST			p33_tx_1byte(P3_PMU_CON4, pd_con4_init_sw_kst_lp);\
							LP_KST\

#define SOFF_KST			LP_KST
#define LP_TMR_KST(a)		LP_KST

#define P33_WKEN()			JL_PMU->PMU_CON |= BIT(0)
#define LP_TMR0_WKEN()		JL_PMU->PMU_CON |= BIT(1)
#define LP_TMR1_WKEN()		JL_PMU->PMU_CON |= BIT(2)

#endif




//
//
//                    peripherals
//
//
//
//******************************************************************
#define GET_SDTAP_EN()		(JL_SDTAP->CON & BIT(0))

#define SDTAP_IO_USB		1
#define GET_SDTAP_CH()		((JL_IOMAP->CON0 & 3 << 17) >> 17)

#define GET_PSRAM_EN()		(JL_PSRAM->PSRAM_CON0 & BIT(1))

#define LRCT_TS(x)        SFR(JL_LRCT->CON,  1,  3,  x)


#endif
