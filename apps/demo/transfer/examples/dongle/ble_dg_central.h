#ifndef _BLE_DG_CENTRAL_H
#define _BLE_DG_CENTRAL_H
#include "le_gatt_common.h"
#include "app_config.h"
#include "usb_suspend_resume.h"

#if (CONFIG_APP_DONGLE)

#if CONFIG_BLE_HIGH_SPEED
//ATT发送的包长,    note: 23 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE        (247)
#else
#define ATT_LOCAL_MTU_SIZE        (64)
#endif

//ATT缓存的buffer支持缓存数据包个数
#if RCSP_BTMATE_EN
#if CONFIG_BLE_HIGH_SPEED
//ATT发送的包长,    note: 23 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE        (247)
#define ATT_PACKET_NUMS_MAX       (5)
#else
#define ATT_LOCAL_MTU_SIZE        (64)
#define ATT_PACKET_NUMS_MAX       (2 * 8 * CONFIG_BT_GATT_CLIENT_NUM)
#endif
#else
#define ATT_LOCAL_MTU_SIZE        (64)
#define ATT_PACKET_NUMS_MAX       (10)
#endif

//ATT缓存的buffer大小,  note: need >= 23,可修改
#define ATT_SEND_CBUF_SIZE        (ATT_PACKET_NUMS_MAX * (ATT_PACKET_HEAD_SIZE + ATT_LOCAL_MTU_SIZE))

/* NOT_KEEP_RAM */
/* uint8_t gatt_ram_buffer[ATT_SEND_CBUF_SIZE + ATT_LOCAL_MTU_SIZE] __attribute__((aligned(4))); */

//搜索类型
#define SET_SCAN_TYPE       SCAN_ACTIVE
//搜索 周期大小
#define SET_SCAN_INTERVAL   ADV_SCAN_MS(24) // unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW     ADV_SCAN_MS(8)  // unit: 0.625ms ,<= SET_SCAN_INTERVAL

//连接周期
#if CONFIG_BLE_CONNECT_SLOT
#define BASE_INTERVAL_MIN   (1000) // 最小的interval
#define SET_CONN_INTERVAL   (BASE_INTERVAL_MIN) //(unit:us)
#else
#define BASE_INTERVAL_MIN   (6)//最小的interval
#define SET_CONN_INTERVAL   (BASE_INTERVAL_MIN*3) //(unit:1.25ms)
#endif
//连接latency
#define SET_CONN_LATENCY    0  //(unit:conn_interval)
//连接超时
#define SET_CONN_TIMEOUT    100 //(unit:10ms)

//建立连接超时
#define SET_CREAT_CONN_TIMEOUT    0 //(unit:ms)

//配对信息
#define   CLIENT_PAIR_BOND_ENABLE    CONFIG_BT_SM_SUPPORT_ENABLE
#define   PAIR_BOND_TAG              0x53

//dongle 上电开配对管理,若配对失败,没有配对设备，停止搜索
#define POWER_ON_RECONNECT_START   (1)   // 上电先回连
#define POWER_ON_SWITCH_TIME       (8000)//unit ms,切换搜索回连周期
#define MATCH_DEVICE_RSSI_LEVEL    (-50)  //RSSI 阈值

/*设备match id 对应搜索表 dg_match_device_table的 顺序位置*/
#define NAME1_DEV_ID                 (SUPPORT_MAX_GATT_CLIENT + 0)
#define NAME2_DEV_ID                 (SUPPORT_MAX_GATT_CLIENT + 1)
#define USER_CONFIG_DEV_ID           (SUPPORT_MAX_GATT_CLIENT + 2)

#define     DG_PAIR_RECONNECT_SEARCH_PROFILE    0 //回连是否搜索profile
#define     MOUSE_USB_PACKET_LEN                6

enum dongle_connect_state {
    DG_NOT_CONNECT = 0,
    DG_CONNECT_COMPLETE,
    DG_SEARCH_PROFILE_COMPLETE
};

struct ctl_pair_info_t {
    uint8_t head_tag;
    uint8_t match_dev_id;
    uint8_t pair_flag;
    uint8_t peer_address_info[7];
    uint16_t conn_handle;
    uint16_t conn_interval;
    uint16_t conn_latency;
    uint16_t conn_timeout;
    uint16_t write_handle;
};

extern uint8_t dg_central_get_match_id(uint16_t conn_handle);
extern uint8_t dg_central_get_ota_is_support(uint16_t conn_handle);
extern uint8_t dg_central_is_succ_connection(void);
extern uint8_t *dg_central_get_conn_address(uint16_t conn_handle);
extern void dg_cenrtal_uuid_count_set(void);
extern int ble_dongle_send_data(uint16_t con_handle, uint8_t *data, uint16_t len);
void ble_module_enable(uint8_t en);
int dg_central_clear_pair(void);
void dg_central_usb_status_handler(const usb_dev usb_id, usb_slave_status status);
#endif
#endif
