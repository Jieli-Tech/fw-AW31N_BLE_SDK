#ifndef _LE_AT_CHAR_CLIENT_H
#define _LE_AT_CHAR_CLIENT_H

#include <stdint.h>

//搜索类型
#define SET_SCAN_TYPE                         SCAN_ACTIVE
//搜索 周期大小
#define SET_SCAN_INTERVAL                     ADV_SCAN_MS(24) // unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW                       ADV_SCAN_MS(8)  // unit: 0.625ms, <= SET_SCAN_INTERVAL

//连接周期
#define BASE_INTERVAL_MIN                     (6)//最小的interval
#define SET_CONN_INTERVAL                     (BASE_INTERVAL_MIN*4) //(unit:1.25ms)
//连接latency
#define SET_CONN_LATENCY                      0  //(unit:conn_interval)
//连接超时
#define SET_CONN_TIMEOUT                      400 //(unit:10ms)

//建立连接超时
#define SET_CREAT_CONN_TIMEOUT                8000 //(unit:ms)

//配对信息表
#define CLIENT_PAIR_BOND_ENABLE               CONFIG_BT_SM_SUPPORT_ENABLE
#define CLIENT_PAIR_BOND_TAG                  0x56

#define AT_CLIENT_WRITE_SEND_DATA            1 //测试发数
#define AT_CLIENT_WRITE_UUID                 0xae01
#define AT_CHAR_SCAN_SNED_ENABLE             0

#define MAX_UUID_MATCH_NUM                   5 // 根据你要搜索的uuid数量

struct ctl_pair_info_t {
    uint16_t conn_handle;
    uint16_t conn_interval;
    uint16_t conn_latency;
    uint16_t conn_timeout;
    uint8_t head_tag;
    uint8_t match_dev_id;
    uint8_t pair_flag;
    uint8_t peer_address_info[7];
};


void ble_at_client_exit(void);
void ble_at_client_init(void);

void ble_test_auto_scan(uint8_t en);
int ble_at_client_scan_enable(uint8_t enable);
int ble_at_client_disconnect(uint8_t id);
int ble_at_client_get_conn_param(uint16_t *conn_param);
int ble_at_client_set_conn_param(uint16_t *conn_param);
int ble_at_client_creat_connection(uint8_t *conn_addr, uint8_t addr_type);
void ble_at_client_send_data(uint8_t cid, uint8_t *packet, uint16_t size);
bool ble_at_client_set_target_uuid16(uint16_t services_uuid16, uint16_t characteristic_uuid16, uint8_t opt_type);
int ble_at_client_creat_cannel(void);
#endif

