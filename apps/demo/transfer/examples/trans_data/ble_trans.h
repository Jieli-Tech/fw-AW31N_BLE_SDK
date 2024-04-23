// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BLE_TRANS_H
#define _BLE_TRANS_H

#include <stdint.h>
#include "app_config.h"
#include "gatt_common/le_gatt_common.h"

#if (CONFIG_APP_LE_TRANS)
/*
 打开流控使能后,确定使能接口 att_server_flow_enable 被调用
 然后使用过程 通过接口 att_server_flow_hold 来控制流控开关
 注意:流控只能控制对方使用带响应READ/WRITE等命令方式
 例如:ATT_WRITE_REQUEST = 0x12
 */
#define ATT_DATA_RECIEVT_FLOW      0//流控功能使能

//测试NRF连接,工具不会主动发起交换流程,需要手动操作; 但设备可配置主动发起MTU长度交换请求
#define ATT_MTU_REQUEST_ENALBE     0    /*配置1,就是设备端主动发起交换*/

//检测对方的系统类型，ios or 非ios
#define ATT_CHECK_REMOTE_REQUEST_ENALBE     0    /*配置1,就是设备端主动检查*/

//ATT发送的包长,    note: 23 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE        (64) /*一般是主机发起交换,如果主机没有发起,设备端也可以主动发起(ATT_MTU_REQUEST_ENALBE set 1)*/

//ATT缓存的buffer支持缓存数据包个数
#define ATT_PACKET_NUMS_MAX       (2)

//ATT缓存的buffer大小,  note: need >= 23,可修改
#define ATT_SEND_CBUF_SIZE        (ATT_PACKET_NUMS_MAX * (ATT_PACKET_HEAD_SIZE + ATT_LOCAL_MTU_SIZE))

// 广播周期 (unit:0.625ms)
#define ADV_INTERVAL_MIN          (160 * 5)//

#define TEST_TRANS_CHANNEL_DATA      0 /*测试记录收发数据速度*/
#define TEST_TRANS_NOTIFY_HANDLE     ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE /*主动发送hanlde,为空则不测试发数*/
#if CONFIG_BLE_HIGH_SPEED
#define TEST_TRANS_TIMER_MS          5
#else
#define TEST_TRANS_TIMER_MS          500
#endif
#define TEST_PAYLOAD_LEN            (244)/*发送配PDU长度是251的包*/

//共可用的参数组数
#define CONN_PARAM_TABLE_CNT      (sizeof(trans_connection_param_table)/sizeof(struct conn_update_param_t))

#define EIR_TAG_STRING   0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'

//定义的产品信息,for test
#define  PNP_VID_SOURCE   0x02
#define  PNP_VID          0x05ac //0x05d6
#define  PNP_PID          0x022C //
#define  PNP_PID_VERSION  0x011b //1.1.11

//---------------
//连接参数更新请求设置
//是否使能参数请求更新,0--disable, 1--enable
static uint8_t trans_connection_update_enable = 1; ///0--disable, 1--enable
//请求的参数数组表,排队方式请求;哪组对方接受就用那组
static const struct conn_update_param_t trans_connection_param_table[] = {
#if CONFIG_BLE_HIGH_SPEED
    {6, 12,  10, 400},// ios fast
#endif
    {16, 24, 0, 100},//11
    {12, 28, 0, 100},//3.7
    {8,  20, 0, 100},
};
//---------------
static const char user_tag_string[] = {EIR_TAG_STRING};
static const uint8_t trans_PNP_ID[] = {PNP_VID_SOURCE, PNP_VID & 0xFF, PNP_VID >> 8, PNP_PID & 0xFF, PNP_PID >> 8, PNP_PID_VERSION & 0xFF, PNP_PID_VERSION >> 8};

void trans_key_deal_test(uint8_t key_type, uint8_t key_value);
// void trans_client_init(void);
// void trans_client_exit(void);
// int trans_client_search_remote_profile(uint16_t conn_handle);
// int trans_client_search_remote_stop(uint16_t conn_handle);
//
// void trans_ios_services_init(void);
// void trans_ios_services_exit(void);
#endif
#endif
