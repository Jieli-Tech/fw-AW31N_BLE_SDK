#ifndef HID_APP_CONFIG_H
#define HID_APP_CONFIG_H

#include "le_common.h"
#include "board_config.h"

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

//app case 选择,只选1,要配置对应的board_config.h
#define CONFIG_APP_KEYBOARD                 1//hid按键 ,default case
#define CONFIG_APP_KEYFOB                   0//自拍器,
#define CONFIG_APP_MOUSE_SINGLE             0//单模鼠标（ble） 需搭配CONFIG_BOARD_AW313A_MOUSE_SINGLE板级
#define CONFIG_APP_MOUSE_DUAL               0//三模鼠标（ble&2.4g&usb） 需搭配CONFIG_BOARD_AW313A_MOUSE板级
#define CONFIG_APP_MOUSE_LOW_LATENCY        0//低延时从机，高回报率测试，需搭配CONFIG_BOARD_AW313A_MOUSE板级
#define CONFIG_APP_KEYPAGE                  0//翻页器
#define CONFIG_APP_REMOTE_CONTROL           0//遥控器，需搭配CONFIG_BOARD_AW31A_RC板级
#define CONFIG_APP_IDLE                     0//IDLE

//add in bt_ble.h
#define CONFIG_HOGP_COMMON_ENABLE           1 //公共的hogp

#if(CONFIG_APP_MOUSE_DUAL) || (CONFIG_APP_MOUSE_LOW_LATENCY)

#define CONFIG_BT_API_MSG_BUFSIZE         (0xa0)//api cmd 消息池大小
#define CONFIG_HOST_MSG_BUFSIZE           (0x100)//host 消息池大小
#define CONFIG_CTRL_MSG_BUFSIZE           (0x100)//btctrl 消息池大小
#define CFG_BT_MSG_BUFSZIE                (CONFIG_BT_API_MSG_BUFSIZE + CONFIG_HOST_MSG_BUFSIZE + CONFIG_CTRL_MSG_BUFSIZE)
#define SYS_STACK_SIZE                    (0x800)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x660 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xEC0)  //nv malloc堆的大小


#elif(!CONFIG_APP_IDLE)
//SDK 应用内存分配,谨慎修改
//host 和 btctrl 消息池大小
#define CONFIG_BT_API_MSG_BUFSIZE         (0xa0)//api cmd 消息池大小
#define CONFIG_HOST_MSG_BUFSIZE           (0x100)//host 消息池大小
#define CONFIG_CTRL_MSG_BUFSIZE           (0x100)//btctrl 消息池大小
#define CFG_BT_MSG_BUFSZIE                (CONFIG_BT_API_MSG_BUFSIZE + CONFIG_HOST_MSG_BUFSIZE + CONFIG_CTRL_MSG_BUFSIZE)
#define SYS_STACK_SIZE                    (0x600)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x500)  //用户堆栈大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0x460 + CFG_BT_MSG_BUFSZIE)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0xDC0)  //nv malloc堆的大小

#else
//SDK 应用内存分配,谨慎修改
#define CONFIG_BT_API_MSG_BUFSIZE         (0)//api cmd 消息池大小
#define CONFIG_HOST_MSG_BUFSIZE           (0)//host 消息池大小
#define CONFIG_CTRL_MSG_BUFSIZE           (0)//btctrl 消息池大小
#define SYS_HEAP_SIZE                     (0x2C0)  //malloc堆的大小
//bt ram, no bt set 0
#define BT_NK_RAM_SIZE                    (0)  //nk malloc堆的大小
#define BT_NV_RAM_SIZE                    (0)  //nv malloc堆的大小

#define SYS_STACK_SIZE                    (0x500)  //中断堆栈大小
#define USR_STACK_SIZE                    (0x480)  //用户堆栈大小

#endif



//蓝牙BLE配置
#define DOUBLE_BT_SAME_NAME                0 //同名字
#define CONFIG_BT_GATT_COMMON_ENABLE       1 //配置使用gatt公共模块
#if CONFIG_APP_MOUSE_LOW_LATENCY
#define CONFIG_BT_SM_SUPPORT_ENABLE        0 //配置是否支持加密
#else
#define CONFIG_BT_SM_SUPPORT_ENABLE        1 //配置是否支持加密
#endif
#define CONFIG_BT_GATT_CLIENT_NUM          0 //配置主机client个数(app not support)
#define CONFIG_BT_GATT_SERVER_NUM          1 //配置从机server个数
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长--TODO
#if (CONFIG_BLE_HIGH_SPEED && TCFG_LOWPOWER_LOWPOWER_SEL)
#error "Please close low power if enable high speed"
#endif
/*
1、根据连接周期去限制包长,包长计算:TODO
2、more data打开会占用下个周期
3、所有周期单位为us
*/
#if (CONFIG_APP_MOUSE_DUAL) || (CONFIG_APP_MOUSE_LOW_LATENCY)
#define CONFIG_BLE_CONNECT_SLOT            1 //BLE高回报率设置, 支持私有协议
#define TCFG_HID_AUTO_SHUTDOWN_TIME       (0 * 60)      //HID无操作自动关机(单位：秒)
#else
#define CONFIG_BLE_CONNECT_SLOT            0 //BLE高回报率设置, 支持私有协议
#define TCFG_HID_AUTO_SHUTDOWN_TIME       (0 * 60)      //HID无操作自动关机(单位：秒)
#endif



//phy auto config
#define CONFIG_SET_1M_PHY                 1
#define CONFIG_SET_2M_PHY                 2
#define CONFIG_SET_CODED_S2_PHY           3
#define CONFIG_SET_CODED_S8_PHY           4

#if CONFIG_BLE_HIGH_SPEED
#define CONFIG_BLE_PHY_SET                CONFIG_SET_2M_PHY //SET 2M_PHY for protect
#define PACKET_DATE_LEN                   251
//Change ram
#define BT_NK_RAM_SIZE_ALL                (BT_NK_RAM_SIZE + 0x600)
#define BT_NV_RAM_SIZE_ALL                (BT_NV_RAM_SIZE + (PACKET_DATE_LEN) * (CONFIG_BT_GATT_CONNECTION_NUM * 3 + 1) + 0x700)
#else
#define CONFIG_BLE_PHY_SET                CONFIG_SET_1M_PHY //default
#define PACKET_DATE_LEN                   27                //default
#define BT_NK_RAM_SIZE_ALL                BT_NK_RAM_SIZE
#define BT_NV_RAM_SIZE_ALL                BT_NV_RAM_SIZE
#endif


#define MY_MALLOC_SELECT                    1 //1--使用heap_buf malloc, 0--使用nv_ram_malloc

// #include "usb_common_def.h"


#include "btcontroller_mode.h"

#include "user_cfg_id.h"

//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_BTMATE_EN                    1
#define UPDATE_MD5_ENABLE                 0
#else
#define RCSP_BTMATE_EN                    0
#define UPDATE_MD5_ENABLE                 0
#endif

#define APP_PRIVATE_PROFILE_CFG

#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
// DUT模式选择,二选一有且仅选一种
#define TCFG_NORMAL_SET_DUT_MODE                 0 // DUT测试模式,默认上电进初始化
#define TCFG_NORMAL_SET_DUT_MODE_API             0 // DUT api 模式,需要自行调用api测试,见ble_test_api.c
#if TCFG_NORMAL_SET_DUT_MODE || TCFG_NORMAL_SET_DUT_MODE_API
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL               0


#if TCFG_NORMAL_SET_DUT_MODE
//close key
#undef KEY_AD_EN
#define KEY_AD_EN                                0

#undef KEY_IO_EN
#define KEY_IO_EN                                0
#endif
#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN					         0

#undef UPDATE_V2_EN
#define UPDATE_V2_EN                             0           // 升级功能使能

#undef TESTBOX_BT_UPDATE_EN
#define TESTBOX_BT_UPDATE_EN                     0           // 测试盒升级

#undef TCFG_HID_AUTO_SHUTDOWN_TIME
#define TCFG_HID_AUTO_SHUTDOWN_TIME              0
#endif //#if TCFG_NORMAL_SET_DUT_MODE

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能
#else

#define TCFG_NORMAL_SET_DUT_MODE                  0
#define TCFG_NORMAL_SET_DUT_MODE_API              0

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
#define UPDATE_V2_EN                            0           // 升级功能使能

#undef TESTBOX_BT_UPDATE_EN
#define TESTBOX_BT_UPDATE_EN                    0           // 测试盒升级

#undef TCFG_HID_AUTO_SHUTDOWN_TIME
#define TCFG_HID_AUTO_SHUTDOWN_TIME             0

#endif

//需要app(BLE)升级
#if CONFIG_APP_OTA_EN
#define SYS_STACK_SIZE_ALL                (SYS_STACK_SIZE + 0x200)  //中断堆栈大小
#define USR_STACK_SIZE_ALL                (USR_STACK_SIZE + 0)      //user堆栈大小
#else
#if TCFG_NORMAL_SET_DUT_MODE || TCFG_NORMAL_SET_DUT_MODE_API
#define SYS_STACK_SIZE_ALL                SYS_STACK_SIZE + 0x32
#else
#define SYS_STACK_SIZE_ALL                SYS_STACK_SIZE
#endif
#define USR_STACK_SIZE_ALL                USR_STACK_SIZE
#endif

#include "asm/power/power_defined.h"
#if TCFG_CLOCK_OSC_1PIN_EN && TCFG_LOWPOWER_LOWPOWER_SEL
#error "CLOCK_OSC_1PIN_EN and LOWPOWER_LOWPOWER_SEL NO MATCH"
#endif

/*
#ifdef CONFIG_SDFILE_ENABLE
#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFI[MaP*LE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#else
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif

#endif
*/

#define CONFIG_BT_RX_BUFF_SIZE  (0)
#define CONFIG_BT_TX_BUFF_SIZE  (0)

#endif




