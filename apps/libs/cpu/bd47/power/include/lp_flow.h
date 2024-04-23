#ifndef __LP_FLOW_H__
#define __LP_FLOW_H__

/********************************************************************************/
/*
 *          --------低功耗模式配置
 *
 *                  P.S 仅内部开发人员配置
 */

#define LP_OSC_DIV(z)       (z > 1000000L ? 64 : 1)
#define LP_FREQ(z)          (z / LP_OSC_DIV(z))
#define LP_nS(z)            (1000000000L / LP_FREQ(z))

// stable time(uS)
#define TSTB0               10L
#define TSTB1               10L
#define TSTB2               10L
#define TSTB3               10L
#define TSTB4               500L
#define TSTB5               1000L   //OSC wait timeout
#define TSTB6               100L    //Pll wait stable timeout
#define LP_TN(x,z)          ((x * 1000L) / LP_nS(z))

#define NSTB0(z)            LP_TN(TSTB0, z)
#define NSTB1(z)            LP_TN(TSTB1, z)
#define NSTB2(z)            LP_TN(TSTB2, z)
#define NSTB3(z)            LP_TN(TSTB3, z)
#define NSTB4(z)            LP_TN(TSTB4, z)
#define NSTB5(z)            LP_TN(TSTB5, z)
#define NSTB6(z)            LP_TN(TSTB6, z)

/*
prp：message backup
stb0：clock disable
stb1：analog disable
stb2：pdown rest disable、soff power disable
sbt3：for voltage down or power disable(wkup signal can only use after stb3)
stb4：voltage and power restore
stb5：analog enable
stb6：clock enable
 */

static inline u8 LP_NK(u16 x)
{
    u16 i = 15;

    for (i = 15; i > 1 ; i--) {
        if (x > ((u16)(1 << (i - 1)))) {
            return i;
        }
    }

    if (x > 1) {
        return 1;
    }

    return 0;
}

// #define LP_NK(x)   (x > 16384 ? 15 : \
//                     x > 8192  ? 14 : \
//                     x > 4096  ? 13 : \
//                     x > 2048  ? 12 : \
//                     x > 1024  ? 11 : \
//                     x >  512  ? 10 : \
//                     x >  256  ?  9 : \
//                     x >  128  ?  8 : \
//                     x >   64  ?  7 : \
//                     x >   32  ?  6 : \
//                     x >   16  ?  5 : \
//                     x >    8  ?  4 : \
//                     x >    4  ?  3 : \
//                     x >    2  ?  2 : \
//                     x >    1  ?  1 : 0)
//
#define KSTB0(z)   (LP_NK(NSTB0(z)))
#define KSTB1(z)   (LP_NK(NSTB1(z)))
#define KSTB2(z)   (LP_NK(NSTB2(z)))
#define KSTB3(z)   (LP_NK(NSTB3(z)))
#define KSTB4(z)   (LP_NK(NSTB4(z)))
#define KSTB5(z)   (LP_NK(NSTB5(z)))
#define KSTB6(z)   (LP_NK(NSTB6(z)))

//-----------------------------------------------------------
/*t1 t2*/
#define LOWPOWER_SUSPEND_US					200L

#define LOWPOWER_ENTER_US					500L

#define LOWPOWER_ENTER_RCH_US          		246L	//关闭、切换系统时钟，备份ROM模块状态

//-----------------------------------------------------------
/*t3 t4*/

#define ROM_POFF_REBOOT_US					99L
//RC预留时间，根据rc频率来设置
#define LOWPOWER_EXIT_RCH_US		    	(ROM_POFF_REBOOT_US + 166)	//恢复ROM模块状态，准备释放ROM模拟模块

//晶振起振预留时间
#define XOSC_RESUME_RESERVE_TIME_US			0L
#define MAX_XOSC_RESUME_TIME_US 			10000L
#define MIN_XOSC_RESUME_TIME_US				1000L

//超时增加恢复时间10ms
#define RECOVER_TIME_APPEND_US				10000L

#define LOWPOWER_PLL_REV_US					360L

#define LOWPOWER_EXIT_US					1500L

/*
 蓝牙系统预留恢复时间800us，抖动时间200us，若剩余时间少于600us，
 则表示此次低功耗超时
 */
#define BT_SYS_RESUME_TIME_US				600L
#define BT_SYS_RESUME_TIME_EXTRA_US			200L

//重设时间CPU 运行时间
#define RESET_RESERVE_TIME_US               300L
//重设时间恢复时间
#define RESET_RECOVER_TIME_US              	(BT_SYS_RESUME_TIME_US + BT_SYS_RESUME_TIME_EXTRA_US)

#define LOWPOWER_ENTER_EXTRA_TIME_US		(0L)
#define LOWPOWER_EXIT_EXTRA_TIME_US			(0L)

//-----------------------------------------------------------
/*by total*/
/*
	t1时间组成：
	1.suspend，挂起蓝牙、系统
    2.enter，进入低功耗准备、SDK模拟模块关闭，备份SDK模块
	3.down，关闭/切换时钟系统，备份rom模块状态

	t2时间组成：p11/p33电源流程

	t3时间组成：p11/p33电源流程

	t4时间组成
	1.reboot，rom启动流程
	2.down_exit，恢复rom模块状态，准备释放rom模拟模块
	3.btosc_rev，晶振起振流程
	4.pll_rev，PLL恢复流程
	5.exit，恢复SDK模块、退出低功耗耗
	6.resume，恢复蓝牙、系统


	<<<<<<<<<<<<t1>>>>>>>>>>>>>><<<<<<<t2>>>>>>>>>>><<<<<<<<<t3>>>>>>>>>
	|-suspend-||-enter-||-down-||-----p11/p33------||------p11/p33------|

	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<t4>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	 |-reboot-||-down_exit-||-btosc_rev-||-pll_rev-||-exit-||-resume-|


 */
//t1
#define LP_PREPARE_TIME_US                  (LOWPOWER_SUSPEND_US + LOWPOWER_ENTER_US)
//t2
#define P11_PDOWN_TIME_US                   (0 + LOWPOWER_ENTER_EXTRA_TIME_US)

//////////////POWERDOWN
//t3
#define POWERDOWN_LRC_SLEEP_RECOVER_TIME_US           (0)
//t4
#define POWERDONW_LRC_P11_PUP_TIME_US                 (LOWPOWER_PLL_REV_US + LOWPOWER_EXIT_US + BT_SYS_RESUME_TIME_US + BT_SYS_RESUME_TIME_EXTRA_US + LOWPOWER_EXIT_EXTRA_TIME_US)

//////////////POWEROFF
//t3
#define POWEROFF_LRC_SLEEP_RECOVER_TIME_US           (0)
//t4
#define POWEROFF_LRC_P11_PUP_TIME_US                 (LOWPOWER_PLL_REV_US + LOWPOWER_EXIT_US + BT_SYS_RESUME_TIME_US + BT_SYS_RESUME_TIME_EXTRA_US + LOWPOWER_EXIT_EXTRA_TIME_US)

//
//
//                  soff p33 control
//
//
//
//******************************************************************
#define sf_con0_init                            \
        /*                       */ ( 0<<7 )  | \
        /*                       */ ( 0<<6 )  | \
        /* SOFF_WKUP_CFG 1bit RW */ ( 0<<5 )  | \
        /*              1bit RW  */ ( 0<<4 )  | \
        /* DIS EN       2bit RW  */ ( 2<<2 )  | \
        /* PD MD        1bit RW  */ ( 1<<1 )  | \
        /* PD_EN        1bit RW  */ ( 1<<0 )

#define sf_con1_rc250k_init                            \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 0<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define sf_con1_lrc_init                            \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 4<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

//#define sf_con2_init                  0

//#define sf_con3_init                  0

#define sf_con4_init                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 1<<0 )


//#define sf_con5_init                  0

#define SOFF_PRP_INIT        0x0002

#define sf_prp0_init                           \
        /* PRP[15:08]   8bit WO */ ((SOFF_PRP_INIT >>  8) & 0xff)
#define sf_prp1_init                           \
        /* PRP[07:00]   8bit WO */ ((SOFF_PRP_INIT )      & 0xff)

#define sf_stb0_stb1_init                       \
        /* STB1 SET     4bit RW  */ ( 4<<4 )  | \
        /* STB0 SET     4bit RW  */ ( 1<<0 )

#define sf_stb2_stb3_init                         \
        /* STB3 SET     4bit RW  */ ( 0<<4 )  | \
        /* STB2 SET     4bit RW  */ ( 0<<0 )

#define sf_stb4_stb5_init                         \
        /* STB5 SET     4bit RW  */ ( 6<<4 )  | \
        /* STB4 SET     4bit RW  */ ( 0<<0 )

#define sf_stb6_init                            \
        /* STB6 SET     4bit RW  */ ( 4<<0 )

#define sf_wvdd_auto0_init                      \
        /* WLDO PRD     3bit RW  */ ( 1<<5 )  | \
        /* AUTO EN      1bit RW  */ ( 1<<4 )  | \
        /*              4bit RW  */ ( 0<<0 )

#define sf_wvdd_auto1_init                      \
        /* WLDO LVL LOW 4bit RW  */ ( 2<<4 )  | \
        /* WLDO LEVEL   4bit RW  */ (15<<0 )

#define sf_wvdd_auto2_init                      \
        /* RESERVED     4bit RW  */ ( 0<<4 )  | \
        /* WLDO LVL HIGH4bit RW  */ (10<<0 )



//
//
//                  pdown p33 control
//
//
//
//******************************************************************
#define pd_con0_init                            \
        /*                       */ ( 0<<7 )  | \
        /*                       */ ( 0<<6 )  | \
        /* SOFF_WKUP_CFG 1bit RW */ ( 0<<5 )  | \
        /*              1bit RW  */ ( 0<<4 )  | \
        /* DIS EN       2bit RW  */ ( 2<<2 )  | \
        /* PD MD        1bit RW  */ ( 0<<1 )  | \
        /* PD_EN        1bit RW  */ ( 1<<0 )

#define pd_con1_init_rc250k                     \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 0<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define pd_con1_init_lrc                        \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 4<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define pd_con1_init_erc                        \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 5<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define pd_con1_init_bt                         \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 3<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 1<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define pd_con1_init_rtc                         \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 3<<0 )  /* 0:RC 1:reserved 2:IO 3:reserved 4:LRC */

#define pd_con2_init                  0

#define pd_con3_init                  0

#define pd_con4_init_kst_tmr0                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 1<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 1<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define pd_con4_init_kst_tmr1                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 1<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 1<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define pd_con4_init_sw_kst_lp                  \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 1<<0 )

//#define pd_con5_init                  0


#define pd_wvdd_auto0_init                      \
        /* WLDO PRD     3bit RW  */ ( 7<<5 )  | \
        /* AUTO EN      1bit RW  */ ( 1<<4 )  | \
        /*              4bit RW  */ ( 0<<0 )

#define pd_wvdd_auto1_init                      \
        /* WLDO LVL LOW 4bit RW  */ ( 0<<4 )  | \
        /* WLDO LEVEL   4bit RW  */ (12<<0 )

#define WVDD_LEVEL_DEFAULT			WVDD_VOL_050V

#define PDOWN_PRP_INIT        0x0002

#define pd_prp0_init                           \
        /* PRP[15:08]   8bit WO */ ((PDOWN_PRP_INIT >>  8) & 0xff)
#define pd_prp1_init                           \
        /* PRP[07:00]   8bit WO */ ((PDOWN_PRP_INIT )      & 0xff)

#define PD_STB0_STB1_INIT(a , b)                       \
        /* STB1 SET     4bit RW  */ ( a<<4 )  | \
        /* STB0 SET     4bit RW  */ ( b<<0 )

#define PD_STB2_STB3_INIT(a, b)                         \
        /* STB3 SET     4bit RW  */ ( a<<4 )  | \
        /* STB2 SET     4bit RW  */ ( b<<0 )

#define PD_STB4_STB5_INIT(a, b)                         \
        /* STB5 SET     4bit RW  */ ( a<<4 )  | \
        /* STB4 SET     4bit RW  */ ( b<<0 )

#define PD_STB6_INIT(a)                            \
        /* STB6 SET     4bit RW  */ ( a<<0 )

#endif
