#ifndef __AT_CHAR_CMDS_H
#define __AT_CHAR_CMDS_H

#if CONFIG_APP_AT_CHAR_COM
#include <stdint.h>
#include "app_config.h"

//默认参数
#define   G_VERSION                               "JL_test"
#define   CONFIG_VERSION                          "2021_02_04"
#define   ADV_INTERVAL_DEFAULT                    2048

#define   PARSE_BUFFER_SIZE                       256
#define   BT_UART_FIFIO_BUFFER_SIZE               0x100
//errid
enum {
    ERR_AT_CMD = 1,
    //add here
};

enum {
    AT_CMD_OPT_NULL = 0,
    AT_CMD_OPT_SET, //设置
    AT_CMD_OPT_GET, //查询
};

enum {
    STR_ID_NULL = 0,
    STR_ID_HEAD_AT_CMD,
    STR_ID_HEAD_AT_CHL,

    STR_ID_OK = 0x10,
    STR_ID_ERROR,

    STR_ID_GVER = 0x20,
    STR_ID_GCFGVER,
    STR_ID_NAME,
    STR_ID_LBDADDR,
    STR_ID_BAUD,

    STR_ID_ADV,
    STR_ID_ADVPARAM,
    STR_ID_ADVDATA,
    STR_ID_SRDATA,
    STR_ID_CONNPARAM,

    STR_ID_SCAN,
    STR_ID_TARGETUUID,
    STR_ID_CONN,
    STR_ID_DISC,
    STR_ID_OTA,
    STR_ID_CONN_CANNEL,
    STR_ID_POWER_OFF,
    STR_ID_LOW_POWER,
};

typedef struct {
    volatile uint8_t len;//长度不包含结束符
    uint8_t next_offset;
    uint8_t data[0];     //带结束符0
} at_param_t;

typedef struct {
    uint16_t str_id;
    uint16_t str_len;
    const char *str;
} str_info_t;

typedef struct {
    u16 services_uuid16;
    u16 characteristic_uuid16;
    u16 opt_type; //属性
} target_uuid_info_t;

void at_send_disconnect(uint8_t cid);
void at_send_connected(uint8_t cid);
void at_send_string(char *str);
void at_cmd_init(void);
void at_send_rx_cid_data(uint8_t cid, uint8_t *packet, uint16_t size);
uint32_t hex_2_str(uint8_t *hex, uint32_t hex_len, uint8_t *str);
uint32_t str_2_hex(uint8_t *str, uint32_t str_len, uint8_t *hex);
void at_respond_send_err(int err_id);
void at_cmd_send(uint8_t *packet, int size);
#endif
#endif
