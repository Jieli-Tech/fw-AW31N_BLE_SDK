#ifndef _OTA_DG_CENTRAL_H
#define _OTA_DG_CENTRAL_H
#include "typedef.h"

#if CONFIG_APP_DONGLE
#define DONGLE_OTA_VERSION                  0
#define HID_OTA_DEVICE_NUM                  CONFIG_BT_GATT_CLIENT_NUM//支持连接的远端ota设备
#define HID_RX_HANDLER_HEND_TAG             0x4A4C
#define HID_RX_HANDLER_TAIL_TAG             0xED
#define HID_SEND_DATA_TAG_LONG              8//除了data之外的包长度
#define HID_USB_SEND_MAX                    64
#define BLE_FRIST_CONNECTION_CHANNEL        0x50
//USB串口指令
enum {
    //APP_BT_EVENT
    APP_CMD_RCSP_DATA = 0,
    APP_CMD_GET_DONGLE_MASSAGE,
    APP_CMD_GET_CONNECT_DEVICE,//DONGLE_REPLY_SEARCH_DEVICE,
    APP_CMD_RETURN_SUCC,
    APP_CMD_RECONNECT_DEVICE,
    APP_CMD_DISCONNECT_DEVICE,
    APP_CMD_AUTH_FLAG,
    //自定义命令
    APP_CMD_CUSTOM = 0xFF,
};
//通讯通道
//-----channel_0: pc->dongle
//-----channel_1: dongle->pc
//-----channel_2: pc->usb透传
//-----channel_3~9: 远端升级透传
enum {
    HID_RX_HANDLER_CHANNEL_COMMAND = 0x00 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_RESPONSE = 0x10 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_USB = 0x20 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE1 = 0x30 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE2 = 0x40 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE3 = 0x50 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE4 = 0x60 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE5 = 0x70 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE6 = 0x80 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE7 = 0x90 + DONGLE_OTA_VERSION,
    HID_RX_HANDLER_CHANNEL_REMOTE8 = 0xA0 + DONGLE_OTA_VERSION,
};
typedef struct {
    u8 auth_flag;//设备认证信息
    u8 ota_support_flag;//设备是否支持ota
    u8 conn_address[7];//连接地址
} device_massage;

typedef struct {
    u8 reconn_channel;//回连接通道
    u8 reconn_address[7];//回连接地址
    u8 reconn_map[HID_OTA_DEVICE_NUM];
    u8 reonn_channel_change;
} ronn_massage;

typedef struct {
    u8 data[HID_USB_SEND_MAX * 9];
} usb_data_info_t;


int dongle_pc_event_handler(struct dg_ota_event *dg_ota);
int dongle_otg_event_handler(struct dg_ota_event *dg_ota);
void clear_auth(u16 channel);
void judge_is_reonn(u16 channel);
extern int get_reonn_param(void);
extern void dongle_return_online_list(void);
extern void dongle_send_data_to_pc_2(uint16_t con_handle, uint8_t *data, uint16_t len);
extern int reonn_channel_is_change(void);
extern void dongle_ota_init(void);
extern void check_is_reconn_succ(uint8_t state, uint16_t con_handle);

#endif
#endif
