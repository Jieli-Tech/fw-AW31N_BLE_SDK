#ifndef _BLE_NOCONN_DEAL_H
#define _BLE_NOCONN_DEAL_H
#include "typedef.h"

//------------------------------------------------------
#define CFG_RF_USE_24G_CDOE       1  // 是否使用24识别码

#if CFG_RF_USE_24G_CDOE
#define CFG_RF_ADV_SCAN_CHL       36//0-(默认37，38，39), 其他配置值 1~39
#define CFG_RF_24G_CODE_ID        0x5DA086D2 // 24g 识别码(32bit),发送接收都要匹配:!!!初始化之后任意非连接时刻修改配对码API:rf_set_conn_24g_coded
#else
#define CFG_RF_ADV_SCAN_CHL       0//0-(默认37，38，39), 其他配置值 1~39
#define CFG_RF_24G_CODE_ID        0//
#endif

#define GATT_ROLE_CLIENT          1
#define GATT_ROLE_SERVER          0

//------------------------------------------------------
//TX发送配置
#define TX_DATA_COUNT             100  //发送次数,决定os_time_dly 多久
#define TX_DATA_INTERVAL          20 //发送间隔>=20ms

#define ADV_INTERVAL_VAL          ADV_SCAN_MS(TX_DATA_INTERVAL)//unit: 0.625ms
#define RSP_TX_HEAD               0xff

#define TX_TEST_SEND_MODE         0 //test 定时发送adv

//------------------------------------------------------
//RX接收配置
//搜索类型
#define SET_SCAN_TYPE       SCAN_ACTIVE
//搜索 周期大小
#define SET_SCAN_INTERVAL   ADV_SCAN_MS(200)//unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW     ADV_SCAN_MS(200)//unit: 0.625ms

void noconn_tx_key_test(u8 key_event, u8 key_value);
#endif
