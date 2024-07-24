#ifndef CONFIG_BOARD_AW318N_DEMO_H
#define CONFIG_BOARD_AW318N_DEMO_H

#include "board_aw318n_dongle_global_build_cfg.h"

#ifdef CONFIG_BOARD_AW318N_DONGLE
// dongle板级，开启USB功能
//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
#define TCFG_UART0_TX_PORT  				IO_PORTA_03                            //串口发送脚配置
#define TCFG_UART0_BAUDRATE  				1000000                                //串口波特率配置

#define TCFG_COMMON_UART_ENABLE             DISABLE_THIS_MOUDLE                    //通用数据转串口模块使能
#define COMMON_UART_TX_PIN                  IO_PORTA_06                            //串口接收脚配置
#define COMMON_UART_RX_PIN                  IO_PORTA_06                            //串口发送脚配置
#define COMMON_UART_INDEX                   UART_NUM_1
#define TCFG_COMMON_UART_BAUDRATE  	        1000000                                //串口波特率配置

//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表
//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define KEY_AD_EN				            ENABLE           //<AD按键使能
#define AD_KEY_IO		                    IO_PORTA_08 // 可用的IO见adc_ch_io_table

#define EXTERN_R_UP     100//220 -> 22k,外挂上拉电阻,0使用内部上拉,内部上拉为10k

#if EXTERN_R_UP
#define R_UP       EXTERN_R_UP
#else
#define R_UP       100    //内部上拉为10K，有20%误差
#endif

#define ADC10_33   (0x3ffL)

// 根据具体电路配置
#define ADC10_30   (ADC10_33 * 1000   / (1000 + R_UP))     //100K
#define ADC10_27   (ADC10_33 * 510    / (510  + R_UP))     //51K
#define ADC10_23   (ADC10_33 * 240    / (240  + R_UP))     //24K
#define ADC10_20   (ADC10_33 * 150    / (150  + R_UP))     //15K
#define ADC10_17   (ADC10_33 * 100    / (100  + R_UP))     //10K
#define ADC10_13   (ADC10_33 * 68     / (68   + R_UP))     //6.8K
#define ADC10_10   (ADC10_33 * 47     / (47   + R_UP))     //4.7K
#define ADC10_07   (ADC10_33 * 22     / (22   + R_UP))     //2.2K
#define ADC10_04   (ADC10_33 * 10     / (10   + R_UP))     //1K
#define ADC10_00   (0)

#define AD_NOKEY        ((ADC10_33 + ADC10_30) / 2)
#define ADKEY1_0		((ADC10_30 + ADC10_27) / 2)
#define ADKEY1_1		((ADC10_27 + ADC10_23) / 2)
#define ADKEY1_2		((ADC10_23 + ADC10_20) / 2)
#define ADKEY1_3		((ADC10_20 + ADC10_17) / 2)
#define ADKEY1_4		((ADC10_17 + ADC10_13) / 2)
#define ADKEY1_5		((ADC10_13 + ADC10_10) / 2)
#define ADKEY1_6		((ADC10_10 + ADC10_07) / 2)
#define ADKEY1_7		((ADC10_07 + ADC10_04) / 2)
#define ADKEY1_8		((ADC10_04 + ADC10_00) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

#define TCFG_ADC_VBAT_CH_EN                 ENABLE
#define TCFG_ADC_VTEMP_CH_EN                ENABLE

//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#define KEY_IO_EN         	                DISABLE                  //<IO按键使能
#define MOUSE_KEY_SCAN_MODE                 DISABLE_THIS_MOUDLE
#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO

#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTA_00        //IO按键端口
#define TCFG_IOKEY_POWER_ONE_PORT_VALUE		0x0                //power port键值
#define TCFG_IOKEY_PREV_CONNECT_WAY			ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_PREV_ONE_PORT			IO_PORTA_01
#define TCFG_IOKEY_PREV_ONE_PORT_VALUE		0x1              //prev port键值

//*********************************************************************************//
//                                 matrix key 配置                                 //
//*********************************************************************************//
#define KEY_MATRIX_EN                       DISABLE
#define MATRIX_KEY_ROW1                     IO_PORTA_00
#define MATRIX_KEY_ROW2                     IO_PORTA_01
#define MATRIX_KEY_ROW3                     IO_PORTA_02

#define MATRIX_KEY_ROL1                     IO_PORTA_08
#define MATRIX_KEY_ROL2                     IO_PORTA_09
#define MATRIX_KEY_ROL3                     IO_PORTA_10

//*********************************************************************************//
//                                 irkey 配置                                      //
//*********************************************************************************//
#define TCFG_IR_ENABLE                      DISABLE
#define IR_KEY_IO			                IO_PORTA_10
#define IR_WORK_FRQ                         38000
#define IR_WORK_DUTY                        5000

//*********************************************************************************//
//                                 charge 配置 TODO                                      //
//*********************************************************************************//
#define TCFG_CHARGE_ENABLE		            DISABLE
#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4199
#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_DIV10
#define TCFG_CHARGE_MA						CHARGE_mA_90
#define TCFG_CHARGE_TRICKLE_MA              CHARGE_mA_9

//*********************************************************************************//
//                                  FLASH配置                                      //
//*********************************************************************************//
#define TFG_EXT_FLASH_EN				    DISABLE
#define TFG_SPI_HW_NUM					    1
#define TFG_SPI_WORK_MODE				    SPI_MODE_BIDIR_1BIT
#define TFG_SPI_READ_DATA_WIDTH			    2
#define TFG_SPI_CS_PORT_SEL				    IO_PORTA_08
#define TFG_SPI_CLK_PORT_SEL			    IO_PORTA_09
#define TFG_SPI_DO_PORT_SEL				    IO_PORTA_10
#define TFG_SPI_DI_PORT_SEL				    IO_PORTA_11
#define SPI_SD_IO_REUSE                     0

//*********************************************************************************//
//                                  SD配置                                         //
//*********************************************************************************//
#define TFG_SD_EN				            DISABLE
#define TFG_SDPG_ENABLE                     DISABLE

//*********************************************************************************//
//                                  VM_SFC配置                                     //
//*********************************************************************************//
#define VM_SFC_ENABLE                       ENABLE

//*********************************************************************************//
//                                  VM Version配置                                 //
//*********************************************************************************//
#define USE_NEW_VM   					   	1
#define USE_OLD_VM      					2
#define SYS_MEMORY_SELECT   				USE_NEW_VM

//*********************************************************************************//
//                                  USB配置                                    //
//*********************************************************************************//
#if HAS_USB_EN
#define TCFG_PC_ENABLE						ENABLE  //PC模块使能
#define TCFG_USB_MSD_CDROM_ENABLE           DISABLE
#define TCFG_USB_EXFLASH_UDISK_ENABLE       DISABLE  // 外掛FLASH UDISK
#define TCFG_UDISK_ENABLE					DISABLE  // U盘模块使能
#define SD_CDROM_EN                         DISABLE
#define USBSLAVE_CTL_MIC                    DISABLE//usb slave config配置
#else
#define TCFG_PC_ENABLE						DISABLE  //PC模块使能
#define TCFG_USB_MSD_CDROM_ENABLE           DISABLE
#define TCFG_USB_EXFLASH_UDISK_ENABLE       DISABLE  //外掛FLASH UDISK
#define TCFG_UDISK_ENABLE					DISABLE //U盘模块使能
#define SD_CDROM_EN                         DISABLE
#define USBSLAVE_CTL_MIC                    DISABLE//usb slave config配置
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
#define FLASH_CACHE_ENABLE                  1
#else
#define FLASH_CACHE_ENABLE                  0
#endif
#define TCFG_USB_PORT_CHARGE                DISABLE
#define TCFG_OTG_USB_DEV_EN                 0  //USB0 = BIT(0)  USB1 = BIT(1)

#if TCFG_PC_ENABLE
#define USB_DEVICE_EN       //Enable USB SLAVE MODE
#endif
#if TCFG_UDISK_ENABLE
#define	USB_DISK_EN        //是否可以读U盘
#endif

#include "usb_std_class_def.h"
#include "usb_common_def.h"
#if TCFG_PC_ENABLE || TCFG_UDISK_ENABLE
#undef   USB_DEVICE_CLASS_CONFIG
#define  USB_DEVICE_CLASS_CONFIG            (HID_CLASS)  //配置usb从机模式支持的class
#else
#undef   USB_DEVICE_CLASS_CONFIG
#define  USB_DEVICE_CLASS_CONFIG            (0)
#endif

//*********************************************************************************//
//                                  LED 配置                                       //
//*********************************************************************************//
#define TCFG_LED_ENABLE                  DISABLE_THIS_MOUDLE            //LED推灯模块
#define TCFG_LED_PIN1                    IO_PORTA_09                   //LED1使用的IO口
// #define TCFG_LED_PIN2                    IO_PORTA_10                //LED2使用的IO口

//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_PLL_SRC			   PLL_REF_XOSC//系统时钟源选择
#define TCFG_CLOCK_SYS_PLL_HZ			   192000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ				   24000000                     //外界晶振频率设置
#define TCFG_CLOCK_OSC_1PIN_EN             0//1:晶振单脚模式,0:双脚模式
/* #define TCFG_CLOCK_MODE                 CLOCK_MODE_USR//CLOCK_MODE_ADAPTIVE */
#define TCFG_CLOCK_MODE                    CLOCK_MODE_ADAPTIVE
#define TCFG_CLOCK_SYS_HZ                  96000000
#define TCFG_CLOCK_LSB_HZ                  48000000
#define TCFG_CLOCK_DUT_SFC_HZ              64000000 //dut 运行时不能降低sfc，尤其是cache小的芯片，出现通信周期太小导致load代码来不及或者来不及处理rxadj的情况

//*********************************************************************************//
//                                供电模式配置                                     //
//*********************************************************************************//
#define  TCFG_POWER_SUPPLY_MODE		       1//适配芯片硬件电路供电方式，0：IOVDD供电，1：VPWR供电

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
/*
 使用外挂DCDC时，默认1.2V供电，最高频率限制在128M

 外挂dcdc控制引脚使用PA5/PA6/NO_CONFIG_PORT，当选择NO_CONFIG_PORT时，会一直
 使用DCDC模式
 */
#define TCFG_DCDC_PORT						IO_PORTA_05
/*
 外挂dcdc上电时间，低功耗时为了尽可能利用DCDC，流程如下：
 enter_lowpower->ldo_en(1)->extern_dcdc_en(0)->extern_dcdc_en(1)->exit_lowpower->ldo_en(0);
 复用了exit_lowpower时间段，请保证外挂dcdc上电最大2000us
 */
#define TCFG_DCDC_DELAY_US					2000
//#define TCFG_LOWPOWER_POWER_SEL				PWR_DCDC15
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			0 //DEEP_SLEEP_EN            //SNIFF状态下芯片是否进入powerdown
#define TCFG_LOWPOWER_PATTERN               SOFT_MODE                    //选择软关机的方式,TCFG_LOWPOWER_LOWPOWER_SEL为0不支持SOFT_BY_POWER_MODE
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_30V
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V               //弱VDDIO等级配置
#define TCFG_LOWPOWER_OSC_TYPE              OSC_TYPE_LRC
#define TCFG_LOWPOWER_SOFF					1

//TODO by bt
#define LOW_POWER_WARN_VAL                  240
#define LOW_POWER_OFF_VAL                   220

//*********************************************************************************//
//                                 BT 配置                                        //
//*********************************************************************************//
#define TCFG_USER_BLE_ENABLE                ENABLE_THIS_MOUDLE
#define TCFG_USER_EDR_ENABLE                DISABLE_THIS_MOUDLE

//lib_btstack_config.c
#define TCFG_BT_SUPPORT_AAC                 DISABLE_THIS_MOUDLE
//profile
#define USER_SUPPORT_PROFILE_HID            0
#define USER_SUPPORT_PROFILE_HFP            0
#define USER_SUPPORT_PROFILE_SPP            0
#define USER_SUPPORT_PROFILE_HCRP           0

//*********************************************************************************//
//                                  Cache配置                                      //
//*********************************************************************************//
#define CPU_USE_CACHE_WAY_NUMBER            4//cache_way范围:2~4

//*********************************************************************************//
//                                  Other配置                                      //
//*********************************************************************************//
#define TCFG_POWER_MODE_QUIET_ENABLE        0

#define TCFG_SYS_LVD_EN						0   //电量检测使能
#define TCFG_SD0_SD1_USE_THE_SAME_HW        0
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS     0
#define TCFG_SDX_CAN_OPERATE_MMC_CARD       0
#define TCFG_POWER_MODE_QUIET_ENABLE        0
#define TCFG_POWER_MODE_QUIET_ENABLE        0

#define SUPPORT_TEST_BOX_BLE_MASTER_TEST_EN	0

#endif
#endif

