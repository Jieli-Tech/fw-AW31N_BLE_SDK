#ifndef _LE_AT_CHAR_SERVER_H
#define _LE_AT_CHAR_SERVER_H

#include <stdint.h>

void ble_test_auto_adv(uint8_t en);
void ble_at_server_init(void);
void ble_at_server_exit(void);
int ble_at_server_send_data(uint8_t cid, uint8_t *packet, uint16_t size);
int ble_at_server_get_name(uint8_t *name);
int ble_at_server_set_name(uint8_t *name, uint8_t len);
int ble_at_server_get_address(uint8_t *addr);
int ble_at_server_set_address(uint8_t *addr);
int ble_at_server_disconnect(void);
int ble_at_server_set_adv_data(uint8_t *data, uint8_t len);
uint8_t *ble_at_server_get_adv_data(uint8_t *len);
int ble_at_server_set_rsp_data(uint8_t *data, uint8_t len);
uint8_t *ble_at_server_get_rsp_data(uint8_t *len);
int ble_at_server_adv_enable(uint8_t enable);
int ble_at_server_set_adv_interval(uint16_t value);
int ble_at_server_get_adv_interval(void);
int ble_at_server_get_adv_state(void);

#endif

