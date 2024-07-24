#ifndef SPPLE_APP_CONFIG_H
#define SPPLE_APP_CONFIG_H

#include "le_common.h"

#ifdef CONFIG_RELEASE_ENABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#define CONFIG_DEBUG_ENABLE    //DEBUG总打印,关闭可优化代码空间

#ifdef CONFIG_DEBUG_ENABLE
#define CONFIG_SDK_DEBUG_LOG   //使能输出堆栈,堆,malloc等信息
#endif

//apps example 选择,只能选1个,要配置对应的board_config.h
#define CONFIG_APP_LE_TRANS               1 // LE's slave
#define CONFIG_APP_MULTI                  0 //蓝牙LE多连 支持2.4g code
#define CONFIG_APP_NONCONN_24G            0 //2.4G 非连接收发
#define CONFIG_APP_DONGLE                 0 //usb + 蓝牙(ble 主机),PC hid设备, 使用需要配置板级board_aw31n_dongle.h
#define CONFIG_APP_AT_CHAR_COM            0 //AT com 字符串格式命令
#define CONFIG_APP_IDLE                   0 // 空闲任务

//host 和 btctrl 消息池大小
#define CONFIG_BT_API_MSG_BUFSIZE         (0xa0)//api cmd 消息池大小
#define CONFIG_HOST_MSG_BUFSIZE           (0x200)//host 消息池大小
#define CONFIG_CTRL_MSG_BUFSIZE           (0x100)//btctrl 消息池大小
#define CFG_BT_MSG_BUFSZIE                (CONFIG_BT_API_MSG_BUFSIZE + CONFIG_HOST_MSG_BUFSIZE + CONFIG_CTRL_MSG_BUFSIZE)

//V5.0 扩展广播/扫描使能
#define CONFIG_BT_EXT_ADV_MODE            0

#if CONFIG_APP_LE_TRANS
//SDK 应用内存分配,谨慎修改
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x660)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xEC0)  //nv malloc堆的大小


//配置双模同名字，同地址
#define CONFIG_APP_SPP_LE_TO_IDLE          0 //SPP_AND_LE To IDLE Use
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长

//蓝牙BLE配置
#define CONFIG_BT_GATT_COMMON_ENABLE       1 //配置使用gatt公共模块
#define CONFIG_BT_SM_SUPPORT_ENABLE        0 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          0 //配置主机client个数 (app not support,应用不支持使能)
#define CONFIG_BT_GATT_SERVER_NUM          1 //配置从机server个数
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数

//BLE 从机扩展搜索对方服务功能,需要打开GATT CLIENT
#if CONFIG_BT_GATT_CLIENT_NUM
#define TRANS_CLIENT_SEARCH_PROFILE_ENABLE  1/*配置模块搜索指定的服务*/

#if !TRANS_CLIENT_SEARCH_PROFILE_ENABLE && CONFIG_BT_SM_SUPPORT_ENABLE /*定制搜索ANCS&AMS服务*/
#define TRANS_ANCS_EN                       1/*配置搜索主机的ANCS 服务,要开配对绑定*/
#define TRANS_AMS_EN                        0/*配置搜索主机的ANCS 服务,要开配对绑定*/
#endif
#endif//#if CONFIG_BT_GATT_CLIENT_NUM

#elif CONFIG_APP_DONGLE
#define CONFIG_BLE_CONNECT_SLOT            0 //BLE高回报率设置, 支持私有协议,所有周期单位为us,适配1k回报率鼠标需要开此宏
//SDK 应用内存分配,谨慎修改
#if CONFIG_BLE_CONNECT_SLOT
#define SYS_STACK_SIZE                    (0x900)  //中断堆栈大小
#else
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#endif
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x660 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xFC0)  //nv malloc堆的大小

#define CONFIG_BT_GATT_COMMON_ENABLE       1
#define CONFIG_BT_SM_SUPPORT_ENABLE        1 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          1
#define CONFIG_BT_COMPOSITE_EQUIPMENT      0
#define CONFIG_BT_GATT_SERVER_NUM          0 //
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长---TODO

#elif CONFIG_APP_MULTI
//SDK 应用内存分配,谨慎修改
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x660 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xEC0)  //nv malloc堆的大小

#define CONFIG_BT_GATT_COMMON_ENABLE       1
#define CONFIG_BT_SM_SUPPORT_ENABLE        0
#define CONFIG_BT_GATT_CLIENT_NUM          1 //range(0~1)
#define CONFIG_BT_GATT_SERVER_NUM          0 //range(0~1)
//2.4G模式: 0---ble, 非0---2.4G配对码; !!!主从欲连接,需保持配对码一致
//!!!初始化之后任意非连接时刻修改配对码API:rf_set_conn_24g_coded
#define CFG_USE_24G_CODE_ID_ADV            0
#define CFG_USE_24G_CODE_ID_SCAN           0
#define CFG_RF_24G_CODE_ID_SCAN            (0xAF9A9357) //<=24bits 主机扫描2.4G配对码, 可用void access_addr_generate(u8 *aa);生成
#define CFG_RF_24G_CODE_ID_ADV             (0xAF9A9357) //<=24bits 从机广播2.4G配对码, 可用void access_addr_generate(u8 *aa);生成
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //range(0~1)
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长

#elif CONFIG_APP_AT_CHAR_COM
//SDK 应用内存分配,谨慎修改
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x660 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xEC0)  //nv malloc堆的大小



#define CONFIG_BT_GATT_COMMON_ENABLE       1//
#define CONFIG_BT_SM_SUPPORT_ENABLE        0//
#define CONFIG_BT_GATT_CLIENT_NUM          0//max is 1
#define CONFIG_BT_GATT_SERVER_NUM          1//max is 1
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM)
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长
#define TCFG_AUTO_SHUT_DOWN_TIME		   0

#elif CONFIG_APP_NONCONN_24G
//SDK 应用内存分配,谨慎修改
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x300 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xA00)  //nv malloc堆的大小


//配置收发角色
#define CONFIG_TX_MODE_ENABLE              1 //发射器
#define CONFIG_RX_MODE_ENABLE              0 //接收器

#define CONFIG_BT_GATT_COMMON_ENABLE       0
#define CONFIG_BT_SM_SUPPORT_ENABLE        0
#define CONFIG_BT_GATT_CLIENT_NUM          0
#define CONFIG_BT_GATT_SERVER_NUM          0
#define CONFIG_BT_GATT_CONNECTION_NUM      0
#define CONFIG_BLE_HIGH_SPEED              0

#else
#undef TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE              0

//SDK 应用内存分配,谨慎修改
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0)  //nv malloc堆的大小


#define CONFIG_BT_GATT_COMMON_ENABLE       0
#define CONFIG_BT_SM_SUPPORT_ENABLE        0
#define CONFIG_BT_GATT_CLIENT_NUM          0
#define CONFIG_BT_GATT_SERVER_NUM          0
#define CONFIG_BT_GATT_CONNECTION_NUM      0
#define CONFIG_BLE_HIGH_SPEED              0

#endif


//phy auto config
#define CONFIG_SET_1M_PHY                 1
#define CONFIG_SET_2M_PHY                 2
#define CONFIG_SET_CODED_S2_PHY           3
#define CONFIG_SET_CODED_S8_PHY           4

#if CONFIG_BLE_HIGH_SPEED
#define CONFIG_BLE_PHY_SET                CONFIG_SET_2M_PHY //SET 2M_PHY for protect
#else
#define CONFIG_BLE_PHY_SET                CONFIG_SET_1M_PHY //default
#endif

#ifndef CONFIG_BLE_CONNECT_SLOT
#define CONFIG_BLE_CONNECT_SLOT            0 //BLE高回报率设置, 支持私有协议
#endif


#if CONFIG_BT_GATT_CONNECTION_NUM > 8
#error "SUPPORT MAX IS 8 !!!"
#endif

#define MY_MALLOC_SELECT                   1 //1--使用heap_buf malloc, 0--使用nv_ram_malloc

#include "board_config.h"
//
// #include "usb_common_def.h"

#include "btcontroller_mode.h"

#include "user_cfg_id.h"

//需要app(BLE)升级
#if CONFIG_APP_OTA_EN
#define SYS_STACK_SIZE_ALL                (SYS_STACK_SIZE + 0x200)  //中断堆栈大小
#define USR_STACK_SIZE_ALL                (USR_STACK_SIZE + 0)      //user堆栈大小
#else
#define SYS_STACK_SIZE_ALL                SYS_STACK_SIZE
#define USR_STACK_SIZE_ALL                USR_STACK_SIZE
#endif


//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_BTMATE_EN                    1
#define UPDATE_MD5_ENABLE                 0
#else
#define RCSP_BTMATE_EN                    0
#define UPDATE_MD5_ENABLE                 0
#endif

#define APP_PRIVATE_PROFILE_CFG

#if CONFIG_BT_EXT_ADV_MODE
#define APP_TO_ALLOW_EXT_ADV
#endif

#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
#define TCFG_NORMAL_SET_DUT_MODE                  0
#if TCFG_NORMAL_SET_DUT_MODE
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

//close key
#undef KEY_AD_EN
#define KEY_AD_EN                                 0

#undef KEY_IO_EN
#define KEY_IO_EN                                 0

#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN						      0

#undef UPDATE_V2_EN
#define UPDATE_V2_EN                              0     // 升级功能使能

#undef TESTBOX_BT_UPDATE_EN
#define TESTBOX_BT_UPDATE_EN                      0     // 测试盒升级

#endif //#if TCFG_NORMAL_SET_DUT_MODE

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能
#else

#define TCFG_NORMAL_SET_DUT_MODE                  0

//close key
#undef KEY_AD_EN
#define KEY_AD_EN                                0

#undef KEY_IO_EN
#define KEY_IO_EN                                0

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能

#undef  TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能

#undef  TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN						      0

#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

#undef TCFG_AUDIO_DAC_LDO_VOLT
#define TCFG_AUDIO_DAC_LDO_VOLT				DACVDD_LDO_2_65V

#undef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15

#undef  TCFG_PWMLED_ENABLE
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE

#undef  TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE

#undef  TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE

#undef TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE			    0

#undef TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME	        0

#undef TCFG_POWER_ON_NEED_KEY
#define TCFG_POWER_ON_NEED_KEY		        0

// #undef TCFG_UART0_ENABLE
// #define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE

#undef UPDATE_V2_EN
#define UPDATE_V2_EN                            0      // 升级功能使能

#undef TESTBOX_BT_UPDATE_EN
#define TESTBOX_BT_UPDATE_EN                    0      // 测试盒升级

#endif


#define BT_FOR_APP_EN                     0

//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_BTMATE_EN                    1
#define UPDATE_MD5_ENABLE                 0
#else
#define RCSP_BTMATE_EN                    0
#define UPDATE_MD5_ENABLE                 0
#endif

#if TCFG_CLOCK_SYS_HZ == 160000000 && TCFG_CLOCK_SYS_PLL_HZ != 240000000
#error "SYS_HZ and SYS_PLL_ZH NO MATCH"
#endif

/*
#ifdef CONFIG_SDFILE_ENABLE
#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFILE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#else
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif

#endif
*/

#define CONFIG_BT_RX_BUFF_SIZE  (0)
#define CONFIG_BT_TX_BUFF_SIZE  (0)

#define FLOW_CONTROL             0  //AT 字符串口流控


#endif

