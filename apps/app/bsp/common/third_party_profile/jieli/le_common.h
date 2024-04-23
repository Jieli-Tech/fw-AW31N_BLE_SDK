/*********************************************************************************************
    *   Filename        : le_counter.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2017-01-17 15:17

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

#ifndef    __LE_COMMON_H_
#define    __LE_COMMON_H_

#include "btcontroller_mode.h"
#include "typedef.h"
#include <stdint.h>
#include "bt_include/bluetooth.h"
// #include "app_config.h"
#include "bt_include/le/le_common_define.h"
#include "bt_include/third_party/common/ble_user.h"


//--------------------------------------------
#ifdef CONFIG_DEBUG_ENABLE
#define LE_DEBUG_PRINT_EN               1     // log switch
#else
#define LE_DEBUG_PRINT_EN               0     // log switch
#endif
//--------------------------------------------

#define TRANS_DATA_EN               1//数传demo
#define RCSP_ADV_EN                 0//not app
#define SMART_BOX_EN                0//not app
#define ANCS_CLIENT_EN              0//not app
#define LL_SYNC_EN                  0//not app
#define TUYA_DEMO_EN                0//not app
#define BLE_CLIENT_EN               0
#define TRANS_MULTI_BLE_EN          0
#define AI_APP_PROTOCOL             0//not app
#define BLE_WIRELESS_CLIENT_EN      0//not app
#define BLE_WIRELESS_SERVER_EN      0//not app
#define BLE_WIRELESS_1T1_TX_EN      0//无线麦1t1周期广播tx验证
#define BLE_WIRELESS_1T1_RX_EN      0//无线麦1t1周期广播rx验证
#define BLE_WIRELESS_1TN_TX_EN      0//not app
#define BLE_WIRELESS_1TN_RX_EN      0//not app
#define LE_AUDIO_EN                 0//not app
#define DEF_BLE_DEMO_MESH           0//not app
#define APP_ONLINE_DEBUG            0//not app

/*毫秒 转换到 0.625ms 单位*/
#define ADV_SCAN_MS(_ms)                ((_ms) * 8 / 5)


typedef struct {
    u8 create_conn_mode;   //cli_creat_mode_e
    u8 bonding_flag;       //连接过后会绑定，默认快连，不搜索设备
    u8 compare_data_len;   //匹配信息长度
    const u8 *compare_data;//匹配信息，若是地址内容,由高到低位
    u8 filter_pdu_bitmap;     /*过滤指定的pdu包,不做匹配操作; bit map,event type*/
} client_match_cfg_t;

//搜索匹配连接方式
typedef enum {
    CLI_CREAT_BY_ADDRESS = 0,//指定地址创建连接
    CLI_CREAT_BY_NAME,//指定设备名称创建连接
    CLI_CREAT_BY_TAG,//匹配厂家标识创建连接
} cli_creat_mode_e;

enum {
    COMMON_EVENT_EDR_REMOTE_TYPE = 1,
    COMMON_EVENT_BLE_REMOTE_TYPE,
    COMMON_EVENT_SHUTDOWN_ENABLE,
    COMMON_EVENT_SHUTDOWN_DISABLE,
    COMMON_EVENT_MODE_DETECT,
};


//for test
#if 0
static const uint8_t profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | WRITE | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x03, 0x00, 0x00, 0x2a,

    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae30
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x30, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0005 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x04, 0x06, 0x00, 0x01, 0xae,
    // 0x0006 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x06, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0007 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xae,
    // 0x0008 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x08, 0x00, 0x02, 0xae,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae03, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x000a CHARACTERISTIC ae03 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x04, 0x0b, 0x00, 0x03, 0xae,
    // 0x000b VALUE ae03 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x0b, 0x00, 0x03, 0xae,

    /* CHARACTERISTIC,  ae04, NOTIFY, */
    // 0x000c CHARACTERISTIC ae04 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x03, 0x28, 0x10, 0x0d, 0x00, 0x04, 0xae,
    // 0x000d VALUE ae04 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x0d, 0x00, 0x04, 0xae,
    // 0x000e CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x0e, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae05, INDICATE, */
    // 0x000f CHARACTERISTIC ae05 INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x03, 0x28, 0x20, 0x10, 0x00, 0x05, 0xae,
    // 0x0010 VALUE ae05 INDICATE
    0x08, 0x00, 0x20, 0x00, 0x10, 0x00, 0x05, 0xae,
    // 0x0011 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x11, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae10, READ | WRITE | DYNAMIC, */
    // 0x0012 CHARACTERISTIC ae10 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x12, 0x00, 0x03, 0x28, 0x0a, 0x13, 0x00, 0x10, 0xae,
    // 0x0013 VALUE ae10 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x13, 0x00, 0x10, 0xae,


    //////////////////////////////////////////////////////
    //
    // 0x0040 PRIMARY_SERVICE  ae3a
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x40, 0x00, 0x00, 0x28, 0x3a, 0xae,

    /* CHARACTERISTIC,  ae3b, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0041 CHARACTERISTIC ae3b WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x41, 0x00, 0x03, 0x28, 0x04, 0x42, 0x00, 0x3b, 0xae,
    // 0x0042 VALUE ae3b WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x42, 0x00, 0x3b, 0xae,

    /* CHARACTERISTIC,  ae3c, NOTIFY, */
    // 0x0043 CHARACTERISTIC ae3c NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x43, 0x00, 0x03, 0x28, 0x10, 0x44, 0x00, 0x3c, 0xae,
    // 0x0044 VALUE ae3c NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x44, 0x00, 0x3c, 0xae,
    // 0x0045 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x45, 0x00, 0x02, 0x29, 0x00, 0x00,

    //////////////////////////////////////////////////////
    //
    // 0x0046 PRIMARY_SERVICE  1801
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x46, 0x00, 0x00, 0x28, 0x01, 0x18,

    /* CHARACTERISTIC,  2a05, INDICATE, */
    // 0x0047 CHARACTERISTIC 2a05 INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x47, 0x00, 0x03, 0x28, 0x20, 0x48, 0x00, 0x05, 0x2a,
    // 0x0048 VALUE 2a05 INDICATE
    0x08, 0x00, 0x20, 0x00, 0x48, 0x00, 0x05, 0x2a,
    // 0x0049 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x49, 0x00, 0x02, 0x29, 0x00, 0x00,

    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae00
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x80, 0x00, 0x00, 0x28, 0x00, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0040 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x81, 0x00, 0x03, 0x28, 0x04, 0x82, 0x00, 0x01, 0xae,
    // 0x0041 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x82, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0042 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x83, 0x00, 0x03, 0x28, 0x10, 0x84, 0x00, 0x02, 0xae,
    // 0x0043 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x84, 0x00, 0x02, 0xae,
    // 0x0044 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x85, 0x00, 0x02, 0x29, 0x00, 0x00,

    // END
    0x00, 0x00,
};
//
// characteristics <--> handles
//
#define ATT_CHARACTERISTIC_ae01_02_VALUE_HANDLE 0x0082
#define ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE 0x0084
#define ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE 0x0085

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_ae04_01_VALUE_HANDLE 0x000d
#define ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE 0x000e
#define ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE 0x0010
#define ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE 0x0011
#define ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE 0x0013

#define ATT_CHARACTERISTIC_ae3b_01_VALUE_HANDLE 0x0042
#define ATT_CHARACTERISTIC_ae3c_01_VALUE_HANDLE 0x0044
#define ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE 0x0045
#define ATT_CHARACTERISTIC_2a05_01_VALUE_HANDLE 0x0048
#define ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE 0x0049

#endif

//主机事件
typedef enum {
    CLI_EVENT_MATCH_DEV = 1,//搜索到匹配的设备
    CLI_EVENT_CONNECTED, //设备连接成功
    CLI_EVENT_DISCONNECT,//设备连接断开
    CLI_EVENT_MATCH_UUID,//搜索到匹配的UUID
    CLI_EVENT_SEARCH_PROFILE_COMPLETE, //搜索profile服务结束
    CLI_EVENT_CONNECTION_UPDATE,//设备连接参数更新成功
} le_client_event_e;


typedef struct {
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,services_uuid128
    u16 services_uuid16;
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,characteristic_uuid128
    u16 characteristic_uuid16;
    u8  services_uuid128[16];
    u8  characteristic_uuid128[16];
    u16 opt_type; //属性
    u8 read_long_enable: 1; //en
    u8 read_report_reference: 1; //en
    u8 res_bits: 6; //
} target_uuid_t;

// enum {
//     BLE_PRIV_MSG_PAIR_CONFIRM = 0xF0,
//     BLE_PRIV_PAIR_ENCRYPTION_CHANGE,
// };//ble_state_e

//搜索操作记录的 handle
#define OPT_HANDLE_MAX   16
typedef struct {
    //匹配的UUID
    target_uuid_t *search_uuid;
    //可操作的handle
    u16 value_handle;
} opt_handle_t;

//最大匹配的设备个数
#define CLIENT_MATCH_CONN_MAX    3



typedef struct {
    //搜索匹配信息
    const client_match_cfg_t *match_dev_cfg[CLIENT_MATCH_CONN_MAX];
    //加密保定配置 0 or 1
    u8 security_en;
    //搜索服务的个数
    u8 search_uuid_cnt; // <= OPT_HANDLE_MAX
    //搜索服务
    const target_uuid_t *search_uuid_table;
    //回调处理接收到的 notify or indicate 数据
    void (*report_data_callback)(att_data_report_t *data_report, target_uuid_t *search_uuid);
    //主机一些事件回调处理
    void (*event_callback)(le_client_event_e event, u8 *packet, int size);
} client_conn_cfg_t;

#endif


