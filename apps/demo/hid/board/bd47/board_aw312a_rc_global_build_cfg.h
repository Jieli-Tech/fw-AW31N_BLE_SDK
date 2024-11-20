#ifndef CONFIG_BOARD_AW31A_RC_POST_CFG_H
#define CONFIG_BOARD_AW31A_RC_POST_CFG_H

#ifdef CONFIG_BOARD_AW31A_RC

/* Following Macros Affect Periods Of Both Code Compiling And Post-build */

#define CONFIG_DOUBLE_BANK_ENABLE               0       //单双备份选择(若打开了改宏,FLASH结构变为双备份结构，适用于接入第三方协议的OTA， PS: JL-OTA同样支持双备份升级, 需要根据实际FLASH大小同时配置CONFIG_FLASH_SIZE)
#define CONFIG_DB_UPDATE_DATA_GENERATE_EN       0       // generate db_update_data.bin

#define CONFIG_ONLY_GRENERATE_ALIGN_4K_CODE     1    	    //ufw只生成1份4K对齐的代码

#define CONFIG_UPDATE_JUMP_TO_MASK              0   	//配置升级到loader的方式0为直接reset,1为跳转(适用于芯片电源由IO口KEEP住的方案,需要注意检查跳转前是否将使用DMA的硬件模块全部关闭)

#if CONFIG_DOUBLE_BANK_ENABLE
//flash size vaule definition
#define FLASH_SIZE_256K							0x40000
#define FLASH_SIZE_512K							0x80000
#define FLASH_SIZE_1M							0x100000
#define FLASH_SIZE_2M							0x200000
#define FLASH_SIZE_4M							0x400000

#define CONFIG_FLASH_SIZE                       FLASH_SIZE_2M    //配置FLASH大小
#endif

#define UPDATE_V2_EN                            1           // 升级功能使能
#define TESTBOX_BT_UPDATE_EN                    1           // 测试盒升级

#define TESTBOX_UART_UPDATE_EN                  0           // 测试盒串口升级
#define TCFG_UART_UPDATE_PORT		            IO_PORTA_00 // 测试盒串口IO口

#define CONFIG_APP_OTA_EN                       0           // 是否支持RCSP升级(JL-OTA)

#define HAS_USB_EN                              0
#define HAS_NORFS_EN                            0


#endif
#endif

