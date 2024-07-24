#include <stdlib.h>
#include "msg.h"
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
#include "app_power_mg.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "app_modules.h"
#include "user_cfg.h"
#include "ble_at_char_server.h"
#include "ble_at_char_client.h"
#include "at_char_uart.h"
#include "at_char_cmds.h"
#include "app_at_char_com.h"
#include "config_transport.h"

#define LOG_TAG_CONST       AT_CHAR_COM
#define LOG_TAG             "[AT_CHAR_CMD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if CONFIG_APP_AT_CHAR_COM

#define INPUT_STR_INFO(id,string)  {.str_id = id, .str = string, .str_len = sizeof(string)-1,}
#define AT_STRING_SEND(a) at_cmd_send((uint8_t *)a,strlen(a))
#define AT_PARAM_NEXT_P(a) (at_param_t*)&parse_buffer[a->next_offset]

NOT_KEEP_RAM
static uint8_t parse_buffer[PARSE_BUFFER_SIZE] __attribute__((aligned(4)));

uint8_t at_get_low_power_mode(void);
void at_set_low_power_mode(uint8_t enable);
static void black_list_check(uint8_t sta, uint8_t *peer_addr);
//------------------------------------------

//0-7:主机通道(主动连出通道); 8:从机通道; 9:AT指令通道
uint8_t cur_atcom_cid = 9;

static u32 uart_baud = TCFG_AT_UART_BAUDRATE;

//------------------------------------------
static const char at_head_at_cmd[]     = "AT+";
static const char at_head_at_chl[]     = "AT>";
static const char at_str_enter[]       = "\r\n";
static const char at_str_ok[]          = "OK";
static const char at_str_err[]         = "ERR";

static const char at_str_gver[]        = "GVER";
static const char at_str_gcfgver[]     = "GCFGVER";
static const char at_str_name[]        = "NAME";
static const char at_str_lbdaddr[]     = "LBDADDR";
static const char at_str_baud[]        = "BAUD";

static const char at_str_adv[]         = "ADV";
static const char at_str_advparam[]    = "ADVPARAM";
static const char at_str_advdata[]     = "ADVDATA";
static const char at_str_srdata[]      = "SRDATA";
static const char at_str_connparam[]   = "CONNPARAM";

static const char at_str_scan[]        = "SCAN";
static const char at_str_targetuuid[]  = "TARGETUUID";
static const char at_str_conn[]        = "CONN";
static const char at_str_disc[]        = "DISC";
static const char at_str_ota[]         = "OTA";
static const char at_str_conn_cannel[] = "CONN_CANNEL";
static const char at_str_power_off[]   = "POWEROFF";
static const char at_str_lowpower[]    = "LOWPOWER";

static const char specialchar[]        = {'+', '>', '=', '?', '\r', ','};

static const str_info_t at_head_str_table[] = {
    INPUT_STR_INFO(STR_ID_HEAD_AT_CMD, at_head_at_cmd),
    INPUT_STR_INFO(STR_ID_HEAD_AT_CHL, at_head_at_chl),
};

static const str_info_t at_cmd_str_table[] = {
    INPUT_STR_INFO(STR_ID_GVER, at_str_gver),
    INPUT_STR_INFO(STR_ID_GCFGVER, at_str_gcfgver),
    INPUT_STR_INFO(STR_ID_NAME, at_str_name),
    INPUT_STR_INFO(STR_ID_LBDADDR, at_str_lbdaddr),
    INPUT_STR_INFO(STR_ID_BAUD, at_str_baud),

    INPUT_STR_INFO(STR_ID_ADV, at_str_adv),
    INPUT_STR_INFO(STR_ID_ADVPARAM, at_str_advparam),
    INPUT_STR_INFO(STR_ID_ADVDATA, at_str_advdata),
    INPUT_STR_INFO(STR_ID_SRDATA, at_str_srdata),
    INPUT_STR_INFO(STR_ID_CONNPARAM, at_str_connparam),

    INPUT_STR_INFO(STR_ID_SCAN, at_str_scan),
    INPUT_STR_INFO(STR_ID_TARGETUUID, at_str_targetuuid),
    INPUT_STR_INFO(STR_ID_CONN, at_str_conn),
    INPUT_STR_INFO(STR_ID_DISC, at_str_disc),
    INPUT_STR_INFO(STR_ID_OTA, at_str_ota),
    INPUT_STR_INFO(STR_ID_CONN_CANNEL, at_str_conn_cannel),
    INPUT_STR_INFO(STR_ID_POWER_OFF, at_str_power_off),
    INPUT_STR_INFO(STR_ID_LOW_POWER, at_str_lowpower),
};

//------------------------------------------
static uint16_t at_str_length(const uint8_t *packet, uint8_t end_char)
{
    uint16_t len = 0;
    while (*packet++ != end_char) {
        len++;
    }
    return len;
}

static at_param_t *parse_param_split(const uint8_t *packet, uint8_t split_char, uint8_t end_char)
{
    uint8_t char1;
    int i = 0;
    at_param_t *par = (at_param_t *)parse_buffer;

    if (*packet == end_char) {
        return NULL;
    }

    log_info("%s:%s", __FUNCTION__, packet);

    par->len = 0;

    while (1) {
        char1 = packet[i++];
        if (char1 == end_char) {
            par->data[par->len] = 0;
            par->next_offset = 0;
            break;
        } else if (char1 == split_char) {
            par->data[par->len] = 0;
            par->len++;
            par->next_offset = &par->data[par->len] - parse_buffer;

            //init next par
            par = (at_param_t *)&par->data[par->len];
            par->len = 0;
        } else {
            par->data[par->len++] = char1;
        }

        if (&par->data[par->len] - parse_buffer >= PARSE_BUFFER_SIZE) {
            log_error("parse_buffer over");
            par->next_offset = 0;
            break;
        }
    }

#if 0
    par = parse_buffer;
    log_info("param_split:");
    while (par) {
        log_info("len=%d,par:%s", par->len, par->data);
        if (par->next_offset) {
            par = AT_PARAM_NEXT_P(par);
        } else {
            break;
        }
    }
#endif

    return (void *)parse_buffer;
}

//返回实际应该比较的长度
static uint8_t compara_specialchar(uint8_t *packet)
{
    int i = 0;

    while (1) {
        for (int j = 0; j < sizeof(specialchar); j++) {
            if (packet[i] == specialchar[j]) {
                if (j < 2) { //如果是+或>则应返回3
                    return 3;
                } else {
                    return i;
                }
            }
        }
        i++;
    }
}

static const str_info_t *at_check_match_string(uint8_t *packet, uint16_t size, const str_info_t *str_table, int table_size)
{
    int max_count = table_size / sizeof(str_info_t);
    const str_info_t *str_p = str_table;
    uint8_t compara_len = 0;

    compara_len = compara_specialchar(packet);

    while (max_count--) {
        if (str_p->str_len <= size) {

            if (str_p->str_len == compara_len) {
                if (0 == memcmp(packet, str_p->str, str_p->str_len)) {
                    return str_p;
                }
            }
        }
        str_p++;
    }
    return NULL;
}

static uint8_t key_str_to_hex(uint8_t str_val)
{
    uint8_t key_value = str_val;

    if ((key_value >= '0') && (key_value <= '9')) {
        key_value = key_value - '0';
    } else if ((key_value >= 'a') && (key_value <= 'f')) {
        key_value = key_value - 'a' + 0x0a;
    } else if ((key_value >= 'A') && (key_value <= 'F')) {
        key_value = key_value - 'A' + 0x0a;
    } else {
        key_value = 0;
    }
    return key_value;
}

static uint8_t key_hex_to_char(uint8_t hex_val)
{
    uint8_t key_value = hex_val;

    if (key_value <= 9) {
        key_value = key_value + '0';
    } else if ((key_value >= 0x0a) && (key_value <= 0xf)) {
        key_value = key_value + 'A';
    } else {
        key_value = '0';
    }
    return key_value;
}

void at_cmd_send(uint8_t *packet, int size)
{
    log_info("###at_cmd_send(%d):", size);
    log_info_hexdump(packet, size);
    ct_uart_send_packet(packet, size);
    ct_uart_send_packet((uint8_t *)at_str_enter, 2);
}

void at_cmd_send_no_end(const uint8_t *packet, int size)
{
    ct_uart_send_packet((uint8_t *)at_str_enter, 2);
    ct_uart_send_packet(packet, size) ;
}

uint32_t hex_2_str(uint8_t *hex, uint32_t hex_len, uint8_t *str)
{
    uint32_t str_len = 0;
    for (uint32_t i = 0; i < hex_len; i++) { //hex to string
        if (hex[i] < 0x10) {
            sprintf((char *)str + i * 2, "0%x", hex[i]);
        } else {
            sprintf((char *)str + i * 2, "%x", hex[i]);
        }
    }
    str_len = hex_len * 2;
    return str_len;
}

uint32_t str_2_hex(uint8_t *str, uint32_t str_len, uint8_t *hex)
{
    uint32_t hex_len = 0;
    uint32_t i = 0;
    for (i = 0; i < (str_len / 2); i++) {
        hex[i] = key_str_to_hex(str[i * 2]);
        hex[i] <<= 4;
        hex[i] += key_str_to_hex(str[i * 2 + 1]);
        log_info("hex----> %x", hex[i]);
    }
    hex_len = str_len / 2;
    return hex_len;
}

//字符转换十进制，返回值,
static int func_char_to_dec(uint8_t *char_buf, uint8_t end_char)
{
    uint32_t cnt = 0;
    int value = 0;
    uint32_t negative_flag = 0;
    uint8_t val;

    while (char_buf[cnt] != end_char) {
        val = char_buf[cnt];
        cnt++;

        if (val == ' ') {
            continue;
        } else if (val == '-') {
            negative_flag = 1;
        } else {
            if ((val >= '0') && (val <= '9')) {
                value = value * 10 + (val - '0');
            }
        }
    }

    if (value && negative_flag) {
        value = value * (-1);
    }
    return value;
}

//------------------------------------------
void at_respond_send_err(int err_id)
{
    char buf[32];
    sprintf(buf, "ERR:%d", err_id);
    AT_STRING_SEND(buf);
}

//------------------------------------------
static void at_packet_handler(uint8_t *packet, int size)
{
    at_param_t *par;
    const str_info_t *str_p;
    int ret = -1;
    uint8_t operator_type = AT_CMD_OPT_NULL; //
    uint8_t *parse_pt = packet;
    int parse_size = size;
    char buf[128] = {0};

#if FLOW_CONTROL
    if (cur_atcom_cid < 8) {
#if CONFIG_BT_GATT_CLIENT_NUM
        log_info("###le_client_data(%d):", size);
        /* put_buf(packet, size); */
        do {
            ret = ble_at_client_send_data(cur_atcom_cid, packet, size);
            os_time_dly(1);
        } while (ret != 0);
#endif
        return;
    } else if (cur_atcom_cid == 8) {
        log_info("###le_server_data(%d):", size);
        /* put_buf(packet, size); */
        do {
            ret = le_att_server_send_data(cur_atcom_cid, packet, size);
            os_time_dly(1);
        } while (ret != 0);
        return;
    }
#else
    if (cur_atcom_cid < 8) {
#if CONFIG_BT_GATT_CLIENT_NUM
        log_info("###le_client_data(%d):", size);
        /* put_buf(packet, size); */
        ble_at_client_send_data(cur_atcom_cid, packet, size);
#endif
        return;
    } else if (cur_atcom_cid == 8) {
#if CONFIG_BT_GATT_SERVER_NUM
        log_info("###le_server_data(%d):", size);
        /* put_buf(packet, size); */
        ble_at_server_send_data(cur_atcom_cid, packet, size);
#endif
        return;
    }
#endif
    else {
        ;
    }

    str_p = at_check_match_string(parse_pt, parse_size, at_head_str_table, sizeof(at_head_str_table));
    if (!str_p) {
        log_info("###1unknow at_head:%s", packet);
        at_respond_send_err(ERR_AT_CMD);
        return;
    }
    parse_pt   += str_p->str_len;
    parse_size -= str_p->str_len;

    if (str_p->str_id == STR_ID_HEAD_AT_CMD) {
        str_p = at_check_match_string(parse_pt, parse_size, at_cmd_str_table, sizeof(at_cmd_str_table));
        if (!str_p) {
            log_info("###2unknow at_cmd:%s", packet);
            at_respond_send_err(ERR_AT_CMD);
            return;
        }

        parse_pt    += str_p->str_len;
        parse_size -= str_p->str_len;
        if (parse_pt[0] == '=') {
            operator_type = AT_CMD_OPT_SET;
        } else if (parse_pt[0] == '?') {
            operator_type = AT_CMD_OPT_GET;
        }
        parse_pt++;
    }

    log_info("str_id:%d", str_p->str_id);
    put_buf(parse_pt, parse_size);

    par = parse_param_split(parse_pt, ',', '\r');

    switch (str_p->str_id) {
    case STR_ID_HEAD_AT_CHL:
        uint8_t tmp_cid = func_char_to_dec(par->data, '\0');
        if (tmp_cid == 9) {
            black_list_check(0, NULL);
        }
        log_info("STR_ID_HEAD_AT_CHL:%d\n", tmp_cid);
        AT_STRING_SEND("OK");  //响应
        cur_atcom_cid = tmp_cid;
        break;

    case STR_ID_ERROR:
        log_info("STR_ID_ERROR\n");
        break;

    case STR_ID_GVER:                   //2.1;
        log_info("STR_ID_GVER\n");
        AT_STRING_SEND(G_VERSION);
        AT_STRING_SEND("OK");  //响应
        break;

    case STR_ID_GCFGVER:                    //2.2
        log_info("STR_ID_GCFGVER\n");
        AT_STRING_SEND(CONFIG_VERSION);
        AT_STRING_SEND("OK");  //响应
        break;

    case STR_ID_NAME:
        log_info("STR_ID_NAME\n");
#if CONFIG_BT_GATT_SERVER_NUM
        if (operator_type == AT_CMD_OPT_SET) { //2.4
            ble_at_server_set_name(par->data, par->len);
            AT_STRING_SEND("OK");
        } else {                            //2.3
            sprintf(buf, "+NAME:");
            uint8_t len = 0;
            len = strlen(buf);

            len = ble_at_server_get_name((uint8_t *)&buf[0] + len) + len;
            at_cmd_send((uint8_t *)buf, len);
            AT_STRING_SEND("OK");
        }
#endif
        break;

    case STR_ID_LBDADDR:                        //2.5
        log_info("STR_ID_LBDADDR\n");
#if CONFIG_BT_GATT_SERVER_NUM
        if (operator_type == AT_CMD_OPT_GET) {
            char _buf[30] = "+LBDADDR:";
            uint8_t ble_addr[6] = {0};
            uint8_t len = 0;

            len = strlen(_buf);
            sprintf(buf, "+LBDADDR:");
            ble_at_server_get_address(ble_addr);
            hex_2_str(ble_addr, 6, (uint8_t *)&buf[0] + len);

            at_cmd_send((uint8_t *)buf, len + 12);
            AT_STRING_SEND("OK");
        } else {
            at_respond_send_err(ERR_AT_CMD);
        }
#endif
        break;

    case STR_ID_BAUD:
        log_info("STR_ID_BAUD\n");
        if (operator_type == AT_CMD_OPT_SET) {
            uart_baud = func_char_to_dec(par->data, '\0');
            log_info("set baud= %d", uart_baud);
            if (uart_baud == 9600 || uart_baud == 19200 || uart_baud == 38400 || uart_baud == 115200 ||
                uart_baud == 230400 || uart_baud == 460800 || uart_baud == 921600) {
                if (ct_uart_change_baud(uart_baud) > 0) {
                    AT_STRING_SEND("OK");
                } else {
                    at_respond_send_err(ERR_AT_CMD);
                }
            } else {
                at_respond_send_err(ERR_AT_CMD);
            }
        } else {                            //2.6
            sprintf(buf, "+BAUD:%d", uart_baud);
            at_cmd_send((uint8_t *)buf, strlen(buf));
            AT_STRING_SEND("OK");
        }

        break;

    case STR_ID_POWER_OFF:  //2.18
        log_info("STR_ID_POWER_OFF\n");
        AT_STRING_SEND("OK");
        // TODO ,需要返回错误码
        sys_timeout_add(NULL,  app_power_set_soft_poweroff, 100);
        break;

    case STR_ID_LOW_POWER:  //2.18
        log_info("STR_ID_LOW_POWER\n");
        uint8_t lp_state;
        if (operator_type == AT_CMD_OPT_SET) { //2.9
            AT_STRING_SEND("OK");
            lp_state = func_char_to_dec(par->data, '\0');
            log_info("set lowpower: %d\n", lp_state);
            at_set_low_power_mode(lp_state);
        } else {
            lp_state = at_get_low_power_mode();
            log_info("get lowpower: %d\n", lp_state);
            sprintf(buf, "+LOWPOWER:%d", lp_state);
            at_cmd_send((uint8_t *)buf, strlen(buf));
            AT_STRING_SEND("OK");
        }
        break;

    case STR_ID_ADV:
        log_info("STR_ID_ADV\n");
#if CONFIG_BT_GATT_SERVER_NUM
        uint8_t adv_state;
        if (operator_type == AT_CMD_OPT_SET) { //2.9
            adv_state = func_char_to_dec(par->data, '\0');
            ret = ble_at_server_adv_enable(adv_state);
            if (ret == 0) {
                AT_STRING_SEND("OK");
            } else {
                at_respond_send_err(ERR_AT_CMD);
            }
        } else {                            //2.8
            adv_state = ble_at_server_get_adv_state();  //0广播关闭,1打开

            sprintf(buf, "+ADV:%d", adv_state);
            at_cmd_send((uint8_t *)buf, strlen(buf));
            AT_STRING_SEND("OK");
        }
#endif
        break;


    case STR_ID_ADVPARAM:
        log_info("STR_ID_ADVPARAM\n");

#if CONFIG_BT_GATT_SERVER_NUM
        uint16_t adv_interval;
        if (operator_type == AT_CMD_OPT_SET) { //2.11
            adv_interval = func_char_to_dec(par->data, '\0');
            log_info("set_adv_interval: %d", adv_interval);
            ble_at_server_set_adv_interval(adv_interval);
            //ret = ble_op_set_adv_param(adv_interval, ADV_IND, ADV_CHANNEL_ALL);
            AT_STRING_SEND("OK");
        } else {                            //2.10
            adv_interval = ble_at_server_get_adv_interval();
            sprintf(buf, "+ADVPARAM:%d", adv_interval);
            at_cmd_send((uint8_t *)buf, strlen(buf));
            AT_STRING_SEND("OK");
        }
#endif
        break;

    case STR_ID_ADVDATA:
        log_info("STR_ID_ADVDATA\n");

#if CONFIG_BT_GATT_SERVER_NUM
        /* uint8_t i = 0; */
        if (operator_type == AT_CMD_OPT_SET) {
            uint8_t adv_data[35] = {0};
            uint8_t adv_data_len = 0;  //广播hex数据的长度

            //将par->data转换成hex
            adv_data_len = str_2_hex(par->data, par->len, adv_data);
            if (adv_data_len > 31) {
                ret = 1;
            } else {
                ret = ble_at_server_set_adv_data(adv_data, adv_data_len);
            }

            if (ret == 0) {
                AT_STRING_SEND("OK");
            } else {
                // TODO ,需要返回错误码
                at_respond_send_err(ERR_AT_CMD);
            }

        } else { //2.12
            uint8_t adv_data_len = 0;  //广播hex数据的长度
            uint8_t *adv_data = ble_at_server_get_adv_data(&adv_data_len);

            sprintf(buf, "+ADVDATA:");
            uint8_t len = 0;
            len = strlen(buf);
            if (adv_data_len) {
                hex_2_str(adv_data, adv_data_len, (uint8_t *)&buf[len]);
            }
            at_cmd_send((uint8_t *)buf, len + adv_data_len * 2);
            AT_STRING_SEND("OK");
        }
#endif
        break;

    case STR_ID_SRDATA:
        log_info("STR_ID_SRDATA\n");

#if CONFIG_BT_GATT_SERVER_NUM
        /* uint8_t i = 0; */
        if (operator_type == AT_CMD_OPT_SET) {
            uint8_t scan_rsp_data[35] = {0};
            uint8_t scan_rsp_data_len = 0;   //hex长度

            scan_rsp_data_len = str_2_hex(par->data, par->len, scan_rsp_data);

            if (scan_rsp_data_len > 31) {
                ret = 1;
            } else {
                ret = ble_at_server_set_rsp_data(scan_rsp_data, scan_rsp_data_len);
            }

            if (ret == 0) {
                AT_STRING_SEND("OK");
            } else {
                // TODO ,需要返回错误码
                at_respond_send_err(ERR_AT_CMD);
            }

        } else { //2.14
            uint8_t rsp_data_len = 0;  //广播hex数据的长度
            uint8_t *rsp_data = ble_at_server_get_rsp_data(&rsp_data_len);

            sprintf(buf, "+SRDATA:");
            uint8_t len = strlen(buf);

            if (rsp_data_len) {
                hex_2_str(rsp_data, rsp_data_len, (uint8_t *)&buf[0] + len);
            }

            log_info("rsp_data_len  = %d", rsp_data_len);
            at_cmd_send((uint8_t *)buf, len + rsp_data_len * 2);
            AT_STRING_SEND("OK");
        }
#endif
        break;

    case STR_ID_CONNPARAM:
        log_info("STR_ID_CONNPARAM\n");

#if CONFIG_BT_GATT_CLIENT_NUM
        uint16_t conn_param[4] = {0}; //interva_min, interva_max, conn_latency, conn_timeout;
        uint8_t i = 0;

        if (operator_type == AT_CMD_OPT_SET) {
            while (par) {  //遍历所有参数
                log_info("len=%d,par:%s", par->len, par->data);
                conn_param[i] = func_char_to_dec(par->data, '\0');  //获取参数
                if (par->next_offset) {
                    par = AT_PARAM_NEXT_P(par);
                } else {
                    break;
                }
                i++;
            }

            ret = ble_at_client_set_conn_param(conn_param);
            log_info("\n conn_param = %d %d %d %d", conn_param[0], conn_param[1], conn_param[2], conn_param[3]);
            if (ret == 0) {
                AT_STRING_SEND("OK");
            } else {
                // TODO ,需要返回错误码
                at_respond_send_err(ERR_AT_CMD);
            }

        } else {                    //2.16
            sprintf(buf, "+CONNPARAM:");
            uint8_t len = strlen(buf);

            ble_at_client_get_conn_param(conn_param);

            for (i = 0; i < sizeof(conn_param) / sizeof(conn_param[0]); i++) {
                sprintf(&buf[0] + len, "%d", conn_param[i]);
                len = strlen(buf);
                buf[len] = ',';
                len += 1;
            }
            len -= 1;     //清掉最后一个逗号

            at_cmd_send((uint8_t *)buf, len);
            AT_STRING_SEND("OK");
        }
#endif

        break;

    case STR_ID_SCAN:
        log_info("STR_ID_SCAN\n");
#if CONFIG_BT_GATT_CLIENT_NUM

        ret = ble_at_client_scan_enable(func_char_to_dec(par->data, '\0'));

        if (ret == 0) {
            AT_STRING_SEND("OK");
        } else {
            // TODO ,需要返回错误码
            at_respond_send_err(ERR_AT_CMD);
        }

#endif
        break;

    case STR_ID_TARGETUUID:  //2.19 TODO
        log_info("STR_ID_TARGETUUID\n");
#if CONFIG_BT_GATT_CLIENT_NUM
        uint16_t target_uuid_info[3];
        uint8_t par_index = 0;
        if (operator_type == AT_CMD_OPT_SET) {
            while (par) {  //遍历所有参数
                log_info("len=%d,par:%s", par->len, par->data);
                uint16_t uint16_res;
                str_2_hex(par->data, 2, (uint8_t *)&uint16_res + 1);
                str_2_hex(par->data + 2, 2, (uint8_t *)&uint16_res); //先填高位,在填低位
                target_uuid_info[par_index] = uint16_res;
                if (par->next_offset) {
                    par = AT_PARAM_NEXT_P(par);
                } else {
                    break;
                }
                par_index++;
            }

            ble_at_client_set_target_uuid16(target_uuid_info[0], target_uuid_info[1], target_uuid_info[2]);
            AT_STRING_SEND("OK");
        } else {
            at_respond_send_err(ERR_AT_CMD);
        }

#endif
        break;

    case STR_ID_CONN:           //2.20
        log_info("STR_ID_CONN\n");
#if CONFIG_BT_GATT_CLIENT_NUM
        struct create_conn_param_t create_conn_par;
        str_2_hex(par->data, par->len, create_conn_par.peer_address);
        put_buf(create_conn_par.peer_address, 6);

        ble_at_client_scan_enable(0);
        if (!ble_at_client_creat_connection(create_conn_par.peer_address, 0)) {
            AT_STRING_SEND("OK");
        } else {
            at_respond_send_err(ERR_AT_CMD);
        }
#endif
        break;

    case STR_ID_CONN_CANNEL:           //2.20
        log_info("STR_ID_CONN_CANNEL\n");
#if CONFIG_BT_GATT_CLIENT_NUM
        if (!ble_at_client_creat_cannel()) {
            AT_STRING_SEND("OK");
        } else {
            at_respond_send_err(ERR_AT_CMD);
        }
#endif
        break;

    case STR_ID_DISC:           //2.21
        log_info("STR_ID_DISC\n");
        u8 _tmp_cid = func_char_to_dec(par->data, '\0');
        if (_tmp_cid < 7) {
#if CONFIG_BT_GATT_CLIENT_NUM
            ble_at_client_disconnect(_tmp_cid);
#endif
        } else if (_tmp_cid == 8) {
#if CONFIG_BT_GATT_SERVER_NUM
            ble_at_server_disconnect();
#endif
        } else {
            // TODO ,需要返回错误码
            at_respond_send_err(ERR_AT_CMD);
            break;
        }
        AT_STRING_SEND("OK");
        break;

    case STR_ID_OTA:
        log_info("STR_ID_OTA\n");
        /* ret = 0; */
        AT_STRING_SEND(at_str_ok);
        break;

    default:
        break;
    }
}

void at_send_conn_result(uint8_t cid, uint8_t is_sucess)
{
    if (is_sucess) {
        uint8_t tmp[16];
        AT_STRING_SEND("OK");
        sprintf((char *)tmp, "IM_CONN:%d", cid);
        AT_STRING_SEND((char *)tmp);
        cur_atcom_cid = cid;
    } else {
        at_respond_send_err(ERR_AT_CMD);
    }
}

static void disconnect_timeout(void *prvi)
{
    black_list_check(2, NULL);
}

static void black_list_check(uint8_t sta, uint8_t *peer_addr)
{
    static uint8_t last_addr[6] = {0};
    uint8_t static timeout_flag = 0;
    switch (sta) {
    case 0:
        log_info(" cur_atcom_cid = %d  ", cur_atcom_cid);
        if (cur_atcom_cid == 8 + 9) {
#if CONFIG_BT_GATT_SERVER_NUM
            ble_at_server_disconnect();
#endif
            sys_timeout_add(NULL, disconnect_timeout, 1000L * 60 * 5);
            timeout_flag = 1;
            log_info("i am here");

        }
        break; //断开从机连接,并开始计时

    case 1:
        if (memcmp(last_addr, peer_addr, 6) == 0) {
            if (timeout_flag == 1) {
                ble_at_server_disconnect();
                return;
            }
        }
        memcpy(last_addr, peer_addr, 6);
        break; //5分钟内拒绝重连

    case 2:
        timeout_flag = 0;
        break;  //恢复可重连
    }
}


void at_send_disconnect(uint8_t cid)
{
    uint8_t tmp[16];
    /* uint8_t tmp_id = cur_atcom_cid; */

    sprintf((char *)tmp, "IM_DISC:%d", cid);
    AT_STRING_SEND((char *)tmp);
    cur_atcom_cid = 9;
}

void at_send_connected(uint8_t cid)
{
    uint8_t tmp[16];
    cur_atcom_cid = 9;

    sprintf((char *)tmp, "IM_CONN:%d", cid);
    AT_STRING_SEND((char *)tmp);
    cur_atcom_cid = cid;
}

void at_send_string(char *str)
{
    AT_STRING_SEND(str);
}


void at_send_rx_cid_data(uint8_t cid, uint8_t *packet, uint16_t size)
{
    if (cur_atcom_cid == cid) {
        log_info("cid=%d,send_data(%d):", cid, size);
        ct_uart_send_packet(packet, size);
    } else {
        log_info("lose %d,send_data(%d):", cid, size);
    }
}

void at_cmd_init(void)
{
    log_info("%s,%d\n", __func__, __LINE__);
    at_uart_init(at_packet_handler);

    log_info("at com is ready");
    AT_STRING_SEND("IM_READY");
}

#endif

