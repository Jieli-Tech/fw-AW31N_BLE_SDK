#ifndef  __GPADC_HW_H__
#define  __GPADC_HW_H__
//bd47
#include "gpadc_hw_v12.h"
#include "typedef.h"
#include "gpio.h"
#include "clock.h"
#include "asm/power_interface.h"

#define ADC_CH_MASK_TYPE_SEL	0xffff0000
#define ADC_CH_MASK_CH_SEL	    0x000000ff
#define ADC_CH_MASK_PMU_VBG_CH_SEL   0x0000ff00

#define ADC_CH_TYPE_BT     	(0x0<<16)
#define ADC_CH_TYPE_PMU    	(0x1<<16)
#define ADC_CH_TYPE_SYSPLL  (0x2<<16)
#define ADC_CH_TYPE_X32K    (0x3<<16)
#define ADC_CH_TYPE_IO		(0x10<<16)


#define ADC_CH_BT_    	        (ADC_CH_TYPE_BT | 0x0)
#define ADC_CH_PMU_WBG04  	(ADC_CH_TYPE_PMU | (0x0<<8) | 0x0)//WBG04
#define ADC_CH_PMU_MBG08  	(ADC_CH_TYPE_PMU | (0x1<<8) | 0x0)//MBG08
#define ADC_CH_PMU_LVDBG  	(ADC_CH_TYPE_PMU | (0x2<<8) | 0x0)//LVDBG
#define ADC_CH_PMU_MVBG08  	(ADC_CH_TYPE_PMU | (0x3<<8) | 0x0)//MBG08
#define ADC_CH_PMU_VP17_VMAX_BUF (ADC_CH_TYPE_PMU | 0x1)
// #define ADC_CH_PMU_	    (ADC_CH_TYPE_PMU | 0x2)
// #define ADC_CH_PMU_      (ADC_CH_TYPE_PMU | 0x3)
#define ADC_CH_PMU_VTEMP	    (ADC_CH_TYPE_PMU | 0x4)
#define ADC_CH_PMU_VPWR_4 	    (ADC_CH_TYPE_PMU | 0x5) // 1/4VPWR
#define ADC_CH_PMU_IOVDD_4 	    (ADC_CH_TYPE_PMU | 0x6) // 1/4IOVDD
#define ADC_CH_PMU_IOVDD_2 	    (ADC_CH_TYPE_PMU | 0x7) // 1/2IOVDD
// #define ADC_CH_PMU_ 	        (ADC_CH_TYPE_PMU | 0x8)
#define ADC_CH_PMU_DVDD_POR  	(ADC_CH_TYPE_PMU | 0x9)
#define ADC_CH_PMU_DCVDD		(ADC_CH_TYPE_PMU | 0xa)
#define ADC_CH_PMU_DVDD		    (ADC_CH_TYPE_PMU | 0xb)
// #define ADC_CH_PMU_  	    (ADC_CH_TYPE_PMU | 0xc)
#define ADC_CH_PMU_WVDD  	    (ADC_CH_TYPE_PMU | 0xd)
// #define ADC_CH_PMU_  	    (ADC_CH_TYPE_PMU | 0xe)
// #define ADC_CH_PMU_  	    (ADC_CH_TYPE_PMU | 0xf)
// #define ADC_CH_AUDIO_	        (ADC_CH_TYPE_AUDIO | 0x0)
#define ADC_CH_SYS_PLL_		    (ADC_CH_TYPE_SYSPLL | 0x0)
#define ADC_CH_X32K_		    (ADC_CH_TYPE_X32K | 0x0)
#define ADC_CH_IO_PA0       (ADC_CH_TYPE_IO | 0x0)
#define ADC_CH_IO_PA1       (ADC_CH_TYPE_IO | 0x1)
#define ADC_CH_IO_PA2       (ADC_CH_TYPE_IO | 0x2)
#define ADC_CH_IO_PA3       (ADC_CH_TYPE_IO | 0x3)
#define ADC_CH_IO_PA4       (ADC_CH_TYPE_IO | 0x4)
#define ADC_CH_IO_PA5       (ADC_CH_TYPE_IO | 0x5)
#define ADC_CH_IO_PA6       (ADC_CH_TYPE_IO | 0x6)
#define ADC_CH_IO_PF2       (ADC_CH_TYPE_IO | 0x7)
#define ADC_CH_IO_PA8       (ADC_CH_TYPE_IO | 0x8)
#define ADC_CH_IO_PA9       (ADC_CH_TYPE_IO | 0x9)
#define ADC_CH_IO_PA10      (ADC_CH_TYPE_IO | 0xa)
#define ADC_CH_IO_PA11      (ADC_CH_TYPE_IO | 0xb)
#define ADC_CH_IO_FSPG      (ADC_CH_TYPE_IO | 0xc)
#define ADC_CH_IO_DP        (ADC_CH_TYPE_IO | 0xd)
#define ADC_CH_IO_DM        (ADC_CH_TYPE_IO | 0xe)

enum AD_CH {
    AD_CH_BT = ADC_CH_BT_,

    AD_CH_PMU_WBG04 = ADC_CH_PMU_WBG04,
    AD_CH_PMU_MBG08 = ADC_CH_PMU_MBG08,
    AD_CH_PMU_LVDBG = ADC_CH_PMU_LVDBG,
    AD_CH_PMU_MVBG08 = ADC_CH_PMU_MVBG08,
    AD_CH_PMU_VP17_VMAX_BUF = ADC_CH_PMU_VP17_VMAX_BUF,
    AD_CH_PMU_VTEMP = ADC_CH_PMU_VTEMP,
    AD_CH_PMU_VPWR_4,
    AD_CH_PMU_IOVDD_4,
    AD_CH_PMU_IOVDD_2,
    AD_CH_PMU_DVDD_POR = ADC_CH_PMU_DVDD_POR,
    AD_CH_PMU_DCVDD,
    AD_CH_PMU_DVDD,
    AD_CH_PMU_WVDD = ADC_CH_PMU_WVDD,

    // AD_CH_AUDIO = ADC_CH_AUDIO_, //防编译报错，该宏非法
    AD_CH_SYS_PLL = ADC_CH_SYS_PLL_, //防编译报错，该宏非法
    AD_CH_X32K = ADC_CH_X32K_, //防编译报错，该宏非法

    AD_CH_IO_PA0 = ADC_CH_IO_PA0,
    AD_CH_IO_PA1,
    AD_CH_IO_PA2,
    AD_CH_IO_PA3,
    AD_CH_IO_PA4,
    AD_CH_IO_PA5,
    AD_CH_IO_PA6,
    AD_CH_IO_PF2,
    AD_CH_IO_PA8,
    AD_CH_IO_PA9,
    AD_CH_IO_PA10,
    AD_CH_IO_PA11,
    AD_CH_IO_FSPG,
    AD_CH_IO_DP,
    AD_CH_IO_DM,

    AD_CH_IOVDD = ADC_CH_TYPE_IO | 0xffff,
};


#define     ADC_VBG_CENTER        800
#define     ADC_VBG_TRIM_STEP     0
#define     ADC_VBG_DATA_WIDTH    0

//防编译报错
extern const u8 gpadc_battery_mode;
extern const u32 gpadc_ch_power;
extern const u8 gpadc_ch_power_div;
extern const u8 gpadc_power_supply_mode;
#define AD_CH_PMU_VBG   AD_CH_PMU_MBG08
#define AD_CH_LDOREF    AD_CH_PMU_VBG
#define AD_CH_PMU_VPWR   AD_CH_PMU_VPWR_4
#define AD_CH_PMU_VBAT  gpadc_ch_power
#define AD_CH_PMU_VBAT_DIV  gpadc_ch_power_div
#define IO_PORT_FSPG 0xff


#define ADC_PMU_VBG_TEST_SEL(x)     P33_CON_SET(P3_PMU_ADC0, 6, 2, x)
#define ADC_PMU_VBG_TEST_EN(x)      P33_CON_SET(P3_PMU_ADC0, 5, 1, x)
#define ADC_PMU_VBG_BUFFER_EN(x)    P33_CON_SET(P3_PMU_ADC0, 4, 1, x)
#define ADC_PMU_CHANNEL_ADC(x)      P33_CON_SET(P3_PMU_ADC0, 0, 4, x)//PMU_TEST_S3-0
#define ADC_PMU_TOADC_EN(x)         P33_CON_SET(P3_PMU_ADC1, 1, 1, x)
#define ADC_PMU_DET_OE(x)           P33_CON_SET(P3_PMU_ADC1, 0, 1, x)
#define ADC_PMU_CH_CLOSE()  {   ADC_PMU_DET_OE(0);\
                                ADC_PMU_TOADC_EN(0);\
                                ADC_PMU_VBG_TEST_EN(0);\
                            }


#endif  /*GPADC_HW_H*/

