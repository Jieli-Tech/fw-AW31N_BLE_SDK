#ifndef __BLE_HOGP__
#define __BLE_HOGP__
#include "includes.h"
#include "le_gatt_common.h"

int ble_hid_data_send(u8 report_id, u8 *data, u16 len);
void ble_module_enable(u8 en);
int ble_hid_is_connected(void);
void le_hogp_set_output_callback(void *cb);
void le_hogp_set_ReportMap(u8 *map, u16 size);
void le_hogp_disconnect(void);
void ble_hid_key_deal_test(u16 key_msg);
void le_hogp_set_pair_allow(void);
int ble_hid_is_connected(void);
void le_hogp_set_output_callback(void *cb);
void le_hogp_set_pair_config(u8 pair_max, u8 is_allow_cover);
bool le_hogp_get_is_paired();
extern u8 bt_get_pwr_max_level();
extern void set_config_vendor_le_bb(u32 vendor_le_bb);
#endif
