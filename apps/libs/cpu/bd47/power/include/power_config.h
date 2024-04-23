/**@file  	    power_config.h
* @brief        flowing control
* @details
* @author		sunlicheng
* @date     	2022-08-31
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */
#ifndef __POWER_CONFIG_H__
#define __POWER_CONFIG_H__

#define q32s 1

#define CACHE_TYPE_I	1
#define CACHE_TYPE_D	2

//
//
//                    hardware
//
//
//******************************************************************
#define CONFIG_RTC_ENABLE				0
#define CONFIG_SDTAP_ENABLE				0
#define CONFIG_MCLR_ENABLE				1
#define CONFIG_PSRAM_ENABLE				0
#define CONFIG_EVDD_ENABLE				0
#define CONFIG_CHARGE_ENABLE			0
#define CONFIG_XOSC_FASTUP				0
#define PCONFIG_USB_ENABLE				1
#define PCONFIG_CTMU_ENABLE     		0
#define PCONFIG_PWMLED_ENABLE   		0
#define PCONFIG_SD_ENABLE				0
#define CONFIG_P33_IO_PINR_ENABLE		1
#define PCONFIG_LPCTMU_ENABLE			0
#define PCONFIG_MMU_ENABLE				0
#define PCONFIG_ERC_ENABLE				1
#define PCONFIG_FPU_ENABLE				0

#define PCONFIG_CORE_TYPE				q32s
#define PCONFIG_CACHE_TYPE				CACHE_TYPE_I
#define PCONFIG_SP_SWITCH				0

//
//
//                    hardware flowing
//
//
//******************************************************************
/* p11
 */
#define CONFIG_P11_CPU_ENABLE			0
#define CONFIG_P11_ENABLE				0

#define CONFIG_BTOSC_ENABLE				1
#define PCONFIG_PDOWN_PLL_FLOWING		1
#define PCONFIG_FLASH_4BYTE				0
#define PCONFIG_FLASH_PG_VDDIO			0


//
//
//                    flowing
//
//
//******************************************************************
#define CONFIG_MULTI_CORE				0
#define CONFIG_POFF_ENABLE				1
#define CONFIG_PDOWN_ENABLE				1
#define CONFIG_SOFF_ENABLE				1
#define PCONFIG_PHW_DEV_SIMPLE 			1


//
//
//                    debug flowing
//
//
//******************************************************************
#define PCONFIG_PDEBUG_UART				0
#define PCONFIG_PDEBUG_IO				0



#endif
