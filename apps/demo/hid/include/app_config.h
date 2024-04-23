#ifndef HID_APP_CONFIG_H
#define HID_APP_CONFIG_H

#include "le_common.h"


#ifdef CONFIG_RELEASE_ENABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#define CONFIG_DEBUG_ENABLE  //DEBUG总打印


//app case 选择,只选1,要配置对应的board_config.h
#define CONFIG_APP_KEYBOARD                 1//hid按键 ,default case
#define CONFIG_APP_KEYFOB                   0//自拍器,
#define CONFIG_APP_MOUSE_SINGLE             0//单模BLE鼠标 需搭配CONFIG_BOARD_AW313A_MOUSE板级
#define CONFIG_APP_KEYPAGE                  0//翻页器
#define CONFIG_APP_REMOTE_CONTROL           0//遥控器，需搭配CONFIG_BOARD_AW31A_RC板级
#define CONFIG_APP_IDLE                     0//IDLE

//edr sniff模式选择; sniff参数需要调整,移植可具体参考app_keyboard.c
#if CONFIG_APP_MOUSE_SINGLE
#define SNIFF_MODE_RESET_ANCHOR             1//键盘鼠标sniff模式,固定小周期发包,多按键响应快
#else
#define SNIFF_MODE_RESET_ANCHOR             0//待机固定500ms sniff周期,待机功耗较低,按键唤醒有延时
#endif

#define CFG_APP_RUN_BY_WHILE                1//设置app_core跑裸机

//add in bt_ble.h
#if 1
#define CONFIG_HOGP_COMMON_ENABLE          1 //公共的hogp

//蓝牙BLE配置
#define DOUBLE_BT_SAME_NAME                0 //同名字
#define CONFIG_BT_GATT_COMMON_ENABLE       1 //配置使用gatt公共模块
#define CONFIG_BT_SM_SUPPORT_ENABLE        1 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          0 //配置主机client个数(app not support)
#define CONFIG_BT_GATT_SERVER_NUM          1 //配置从机server个数
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长--TODO
#define TCFG_HID_AUTO_SHUTDOWN_TIME       (0 * 60)      //HID无操作自动关机(单位：秒)
#endif


//*********************************************************************************//
//                                  蓝牙NVRAM配置                                  //
//                                  !!!禁止修改!!!                                 //
//*********************************************************************************//
#define LBUF_HEAD                   0x10
#define BLE_LINK_MAX                ((CONFIG_BT_GATT_CONNECTION_NUM == 0) ? 1 : CONFIG_BT_GATT_CONNECTION_NUM)
//NK_RAM
#define BTCTLER_K_RAM_LL_MSG        (0x200 + LBUF_HEAD)               //ll_msg
#define BTCTLER_NK_RAM_SIZE         (((528 + LBUF_HEAD * 4) * BLE_LINK_MAX) + 0x400 + BTCTLER_K_RAM_LL_MSG)//增加0x400作为BT nv_ram占用申请
//NV_RAM
//----------init state
#define BTCTLER_K_RAM_RX_TX_DMA     (1168 + LBUF_HEAD)                //TX/RX DMA
#define BTCTLER_K_RAM_HW            (764  + LBUF_HEAD)                //HW4 HW_ENTITY
#ifdef SUPPORT_RESERVATION_RAM
#define BTCTLER_K_RAM_HCI_CON       ((451 + LBUF_HEAD) * BLE_LINK_MAX)//le_hci_connection_t
#else
#define BTCTLER_K_RAM_HCI_CON       ((712 + LBUF_HEAD) * BLE_LINK_MAX)//le_hci_connection_t
#endif
#define BTCTLER_K_RAM_HW_REGS       ((40  + LBUF_HEAD) * BLE_LINK_MAX)//ble4_hw->regs;
//--------init slave
#define BTCTLER_K_ADV_MALLOC        ((12  + LBUF_HEAD) + (76 + LBUF_HEAD))//ll_adv_hdl+le_adv_link
//--------init master
#define BTCTLER_K_RAM_GATT_CLIENT   (128  + LBUF_HEAD)                //gatt_clients_ram
#define BTCTLER_K_SCAN_MALLOC       ((32  + LBUF_HEAD) + (20 + LBUF_HEAD))//ll_scan_hdl+le_scan_link
#define BTCTLER_K_RAM_LL_INIT       ((32  + LBUF_HEAD) + 60 + LBUF_HEAD)//ll_init_hdl+le_init_link
//----------conn state
#if CONFIG_APP_MOUSE_SINGLE
#define BTCTLER_K_RAM_CONN_STATE    ((492 + LBUF_HEAD) + LBUF_HEAD * 3  + 64)//le_link(link_layer) + one_shot + reserved
#else
#define BTCTLER_K_RAM_CONN_STATE    ((492 + LBUF_HEAD) + LBUF_HEAD * 3)//le_link(link_layer) + one_shot
#endif
#if CONFIG_BT_SM_SUPPORT_ENABLE
#define SM_RAM_CONN                 ((480 + LBUF_HEAD) + (56 + LBUF_HEAD))
#else
#define SM_RAM_CONN                 0
#endif


#if (CONFIG_BT_GATT_CLIENT_NUM || CONFIG_BT_GATT_SERVER_NUM)
#define BTCTLER_NV_MEMORY_SIZE      ((((BTCTLER_K_RAM_RX_TX_DMA + BTCTLER_K_RAM_HW + \
                BTCTLER_K_RAM_HCI_CON + BTCTLER_K_RAM_HW_REGS * 2 + BTCTLER_K_RAM_GATT_CLIENT + BTCTLER_K_SCAN_MALLOC + \
                BTCTLER_K_RAM_LL_INIT + BTCTLER_K_RAM_CONN_STATE + 16 * 2) * CONFIG_BT_GATT_CLIENT_NUM) + \
                ((BTCTLER_K_RAM_RX_TX_DMA + BTCTLER_K_RAM_HW + BTCTLER_K_RAM_HCI_CON + \
                  BTCTLER_K_RAM_HW_REGS + BTCTLER_K_ADV_MALLOC + BTCTLER_K_RAM_CONN_STATE + 16 * 2) * CONFIG_BT_GATT_SERVER_NUM)) + CONFIG_BT_SM_SUPPORT_ENABLE * SM_RAM_CONN + MY_MALLOC_TOTAL)
#else
//特殊应用：eg：no_conn_24g
#define BTCTLER_NV_MEMORY_SIZE      (((BTCTLER_K_RAM_RX_TX_DMA + BTCTLER_K_RAM_HW + \
                BTCTLER_K_RAM_HCI_CON + BTCTLER_K_RAM_HW_REGS * 2 + BTCTLER_K_RAM_GATT_CLIENT + BTCTLER_K_SCAN_MALLOC + \
                BTCTLER_K_RAM_LL_INIT + BTCTLER_K_RAM_CONN_STATE + 16 * 2) * 1) + \
                CONFIG_BT_SM_SUPPORT_ENABLE * SM_RAM_CONN + MY_MALLOC_TOTAL)
#endif
// #endif


//APP应用默认配置
//TODO
// #define TCFG_AEC_ENABLE                     1

// #define TCFG_MEDIA_LIB_USE_MALLOC		    1


// #include "usb_common_def.h"

#include "board_config.h"

#include "btcontroller_mode.h"

#include "user_cfg_id.h"

#if MY_MALLOC_SELECT
#define MY_MALLOC_TOTAL                     0
#else
#define MY_MALLOC_TOTAL                     300 //设置lbuf大小,为了节约空间和蓝牙lbuf整合
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

#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
#define TCFG_NORMAL_SET_DUT_MODE                  0
#if TCFG_NORMAL_SET_DUT_MODE
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0
#endif

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能
#else

#undef  TCFG_BD_NUM
#define TCFG_BD_NUM						          1

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

#undef TCFG_UART0_ENABLE
#define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE

#endif


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
#define CONFIG_BT_RX_BUFF_SIZE  (0)
#define CONFIG_BT_TX_BUFF_SIZE  (0)

#if (CONFIG_BT_MODE != BT_NORMAL)
////bqb 如果测试3M tx buf 最好加大一点
#undef  CONFIG_BT_TX_BUFF_SIZE
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024)

#endif
#define BT_NORMAL_HZ	            CONFIG_BT_NORMAL_HZ
//*********************************************************************************//
//                                 时钟切换配置                                    //
//*********************************************************************************//

#define BT_NORMAL_HZ	            CONFIG_BT_NORMAL_HZ
#define BT_CONNECT_HZ               CONFIG_BT_CONNECT_HZ

#define BT_A2DP_HZ	        	    CONFIG_BT_A2DP_HZ
#define BT_TWS_DEC_HZ	        	CONFIG_TWS_DEC_HZ

//#define MUSIC_DEC_CLOCK			    CONFIG_MUSIC_DEC_CLOCK
//#define MUSIC_IDLE_CLOCK		    CONFIG_MUSIC_IDLE_CLOCK

#define BT_CALL_HZ		            CONFIG_BT_CALL_HZ
#define BT_CALL_ADVANCE_HZ          CONFIG_BT_CALL_ADVANCE_HZ
#define BT_CALL_16k_HZ	            CONFIG_BT_CALL_16k_HZ
#define BT_CALL_16k_ADVANCE_HZ      CONFIG_BT_CALL_16k_ADVANCE_HZ

//*********************************************************************************//
//                                 升级配置                                        //
//*********************************************************************************//
#if (defined(CONFIG_CPU_BR30))
//升级LED显示使能
//#define UPDATE_LED_REMIND
//升级提示音使能
//#define UPDATE_VOICE_REMIND
#endif

#if (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))
//升级IO保持使能
//#define DEV_UPDATE_SUPPORT_JUMP           //目前只有br23\br25支持
#endif
#endif




