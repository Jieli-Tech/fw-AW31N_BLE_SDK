/*********************************************************************************************
    *   Filename        : ble_trans.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2035  @ , All Rights Reserved.
*********************************************************************************************/
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
#include <stdlib.h>
#include "app_modules.h"
#include "ble_trans.h"
#include "ble_trans_profile.h"
#include "led_control.h"
#include "common_uart_control.h"
#include "user_cfg.h"
#if RCSP_BTMATE_EN
#include "custom_cfg.h"
#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "code_v2/update_loader_download.h"
#endif

#if (CONFIG_APP_LE_TRANS)
#define LOG_TAG_CONST       BLE_TRANS
#define LOG_TAG             "[BLE_TRANS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "log.h"

static uint8_t  trans_adv_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t  trans_scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t  trans_test_read_write_buf[4];
static uint8_t  trans_adv_name_ok = 0;//name 优先存放在ADV包
static uint16_t trans_con_handle;
static uint32_t trans_recieve_test_count;
static uint32_t trans_send_test_count;
static adv_cfg_t trans_server_adv_config;
//-------------------------------------------------------------------------------------
static uint16_t trans_att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int trans_att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int trans_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param);
void ble_module_enable(uint8_t en);
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
//输入passkey 加密
#define PASSKEY_ENABLE                     0

static const sm_cfg_t trans_sm_init_config = {
    .slave_security_auto_req = 0,
    .slave_set_wait_security = 0,

#if PASSKEY_ENABLE
    .io_capabilities = IO_CAPABILITY_DISPLAY_ONLY,
#else
    .io_capabilities = IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
#endif

    .authentication_req_flags = SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION,
    .min_key_size = 7,
    .max_key_size = 16,
    .sm_cb_packet_handler = NULL,
};

const gatt_server_cfg_t trans_server_init_cfg = {
    .att_read_cb = &trans_att_read_callback,
    .att_write_cb = &trans_att_write_callback,
    .event_packet_handler = &trans_event_packet_handler,
};

static gatt_ctrl_t trans_gatt_control_block = {
    //public
    .mtu_size = ATT_LOCAL_MTU_SIZE,
    .cbuffer_size = ATT_SEND_CBUF_SIZE,
    .multi_dev_flag	= 0,

    //config
#if CONFIG_BT_GATT_SERVER_NUM
    .server_config = &trans_server_init_cfg,
#else
    .server_config = NULL,
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    .client_config = &trans_client_init_cfg,
#else
    .client_config = NULL,
#endif

#if CONFIG_BT_SM_SUPPORT_ENABLE
    .sm_config = &trans_sm_init_config,
#else
    .sm_config = NULL,
#endif
    //cbk,event handle
    .hci_cb_packet_handler = NULL,
};

/*************************************************************************************************/
/*!
 *  \brief      串口接收转发到BLE
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void trans_uart_rx_to_ble(uint8_t *packet, uint32_t size)
{
    if (trans_con_handle && ble_comm_att_check_send(trans_con_handle, size) &&
        ble_gatt_server_characteristic_ccc_get(trans_con_handle, ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE)) {
        ble_comm_att_send_data(trans_con_handle, ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, packet, size, ATT_OP_AUTO_READ_CCC);
        log_info_hexdump(packet, size);
    } else {
        log_info("drop uart data!!!\n");
    }
}

/*************************************************************************************************/
/*!
 *  \brief      发送请求连接参数表
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void trans_send_connetion_updata_deal(uint16_t conn_handle)
{
    if (trans_connection_update_enable) {
        if (0 == ble_gatt_server_connetion_update_request(conn_handle, trans_connection_param_table, CONN_PARAM_TABLE_CNT)) {
            trans_connection_update_enable = 0;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief      回连状态，使能所有profile
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note      配对绑定的方式，主机回连不是在使能server的通知开关，需要自己打开
 */
/*************************************************************************************************/
static void trans_resume_all_ccc_enable(uint16_t conn_handle, uint8_t update_request)
{
    log_info("resume_all_ccc_enable\n");

#if RCSP_BTMATE_EN
    /* ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY); */
#endif
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_INDICATE);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);

    if (update_request) {
        trans_send_connetion_updata_deal(conn_handle);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      反馈检查对方的操作系统
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note 参考识别手机系统
 */
/*************************************************************************************************/
static void trans_check_remote_result(uint16_t con_handle, remote_type_e remote_type)
{
    if (REMOTE_TYPE_IOS == remote_type) {
        log_info("trans_check %02x: remote_type = %02x, is ios", con_handle, remote_type);
    } else {
        log_info("trans_check %02x: remote_type = %02x, not ios", con_handle, remote_type);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      处理gatt 返回的事件（hci && gatt）
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int trans_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param)
{
    /* log_info("event: %02x,size= %d\n",event,size); */
    switch (event) {
    case GATT_COMM_EVENT_CAN_SEND_NOW:
        break;

    case GATT_COMM_EVENT_SERVER_INDICATION_COMPLETE:
        log_info("INDICATION_COMPLETE:con_handle= %04x,att_handle= %04x\n", \
                 little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
        break;


    case GATT_COMM_EVENT_CONNECTION_COMPLETE:
#if TCFG_LED_ENABLE
        led_set_connect_flag(1);
        // 完成连接灭灯
        led_operate(LED_CLOSE);
#endif
        trans_con_handle = little_endian_read_16(packet, 0);
        trans_connection_update_enable = 1;

        log_info("connection_handle:%04x\n", little_endian_read_16(packet, 0));
        log_info("connection_handle:%04x, rssi= %d\n", trans_con_handle, ble_vendor_get_peer_rssi(trans_con_handle));
        log_info("peer_address_info:");
        log_info_hexdump(&ext_param[7], 7);

        log_info("con_interval = %d\n", little_endian_read_16(ext_param, 14 + 0));
        log_info("con_latency = %d\n", little_endian_read_16(ext_param, 14 + 2));
        log_info("cnn_timeout = %d\n", little_endian_read_16(ext_param, 14 + 4));

#if ATT_MTU_REQUEST_ENALBE
        att_server_set_exchange_mtu(trans_con_handle);/*主动请求MTU长度交换*/
#endif

#if TCFG_COMMON_UART_ENABLE
        //注册串口传输数据回调，串口数据直通到蓝牙
        common_uart_regiest_receive_callback(trans_uart_rx_to_ble);
#endif

#if ATT_CHECK_REMOTE_REQUEST_ENALBE
        att_server_set_check_remote(trans_con_handle, trans_check_remote_result);
#endif
        break;

    case GATT_COMM_EVENT_DISCONNECT_COMPLETE:
        //TODO 可以推msg到app处理
#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif

#if TCFG_LED_ENABLE
        led_set_connect_flag(0);
        led_operate(LED_WAIT_CONNECT);
#endif
        log_info("disconnect_handle:%04x,reason= %02x\n", little_endian_read_16(packet, 0), packet[2]);
        if (trans_con_handle == little_endian_read_16(packet, 0)) {
#if CONFIG_BT_GATT_CLIENT_NUM
            trans_client_search_remote_stop(trans_con_handle);
#endif
            trans_con_handle = 0;
        }
        break;

    case GATT_COMM_EVENT_ENCRYPTION_CHANGE:
        log_info("ENCRYPTION_CHANGE:handle=%04x,state=%d,process =%d", little_endian_read_16(packet, 0), packet[2], packet[3]);
        if (packet[3] == LINK_ENCRYPTION_RECONNECT) {
            trans_resume_all_ccc_enable(little_endian_read_16(packet, 0), 1);
        }
        break;

    case GATT_COMM_EVENT_CONNECTION_UPDATE_COMPLETE:
        log_info("conn_param update_complete:%04x\n", little_endian_read_16(packet, 0));
        log_info("update_interval = %d\n", little_endian_read_16(ext_param, 6 + 0));
        log_info("update_latency = %d\n", little_endian_read_16(ext_param, 6 + 2));
        log_info("update_timeout = %d\n", little_endian_read_16(ext_param, 6 + 4));
        break;

    case GATT_COMM_EVENT_CONNECTION_UPDATE_REQUEST_RESULT:
        break;

    case GATT_COMM_EVENT_MTU_EXCHANGE_COMPLETE:
        log_info("con_handle= %02x, ATT MTU = %u\n", little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
        break;

    case GATT_COMM_EVENT_SERVER_STATE:
        log_info("server_state: handle=%02x,%02x\n", little_endian_read_16(packet, 1), packet[0]);
        break;

    case GATT_COMM_EVENT_SM_PASSKEY_INPUT: {
        uint32_t key = little_endian_read_32(packet, 2);
        r_printf("input_key:%6u\n", key);
    }
    break;

    default:
        break;
    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      处理client 读操作
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note      profile的读属性uuid 有配置 DYNAMIC 关键字，就有read_callback 回调
 */
/*************************************************************************************************/
// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param con_handle of hci le connection
// @param attribute_handle to be read
// @param offset defines start of attribute value
// @param buffer
// @param buffer_size
static uint16_t trans_att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    log_info("read_callback,conn_handle =%04x, handle=%04x,buffer=%08x\n", connection_handle, handle, (uint32_t)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE: {
        const char *gap_name = ble_comm_get_gap_name();
        att_value_len = strlen(gap_name);

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s\n", gap_name);
        }
    }
    break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        att_value_len = sizeof(trans_test_read_write_buf);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &trans_test_read_write_buf[offset], buffer_size);
            att_value_len = buffer_size;
        }
        break;

    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = ble_gatt_server_characteristic_ccc_get(connection_handle, handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;

    case ATT_CHARACTERISTIC_2a50_01_VALUE_HANDLE:
        log_info("read PnP_ID\n");
        att_value_len = sizeof(trans_PNP_ID);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }
        if (buffer) {
            memcpy(buffer, &trans_PNP_ID[offset], buffer_size);
            att_value_len = buffer_size;
        }
        break;

    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;
}


/*************************************************************************************************/
/*!
 *  \brief      处理client write操作
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note      profile的写属性uuid 有配置 DYNAMIC 关键字，就有write_callback 回调
 */
/*************************************************************************************************/
// ATT Client Write Callback for Dynamic Data
// @param con_handle of hci le connection
// @param attribute_handle to be written
// @param transaction - ATT_TRANSACTION_MODE_NONE for regular writes, ATT_TRANSACTION_MODE_ACTIVE for prepared writes and ATT_TRANSACTION_MODE_EXECUTE
// @param offset into the value - used for queued writes and long attributes
// @param buffer
// @param buffer_size
// @param signature used for signed write commmands
// @returns 0 if write was ok, ATT_ERROR_PREPARE_QUEUE_FULL if no space in queue, ATT_ERROR_INVALID_OFFSET if offset is larger than max buffer

static int trans_att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    uint16_t tmp16;
    uint16_t handle = att_handle;

#if !TEST_TRANS_CHANNEL_DATA
    log_info("write_callback,conn_handle =%04x, handle =%04x,size =%d\n", connection_handle, handle, buffer_size);
#endif

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
        trans_send_connetion_updata_deal(connection_handle);
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        ble_gatt_server_characteristic_ccc_set(connection_handle, handle, buffer[0]);
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        tmp16 = sizeof(trans_test_read_write_buf);
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }
        memcpy(&trans_test_read_write_buf[offset], buffer, buffer_size);
        log_info("\n-ae10_rx(%d):", buffer_size);
        log_info_hexdump(buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:

#if TEST_TRANS_CHANNEL_DATA
        /* putchar('R'); */
        trans_recieve_test_count += buffer_size;
        break;
#endif

        log_info("\n-ae01_rx(%d):", buffer_size);
        log_info_hexdump(buffer, buffer_size);

#if TCFG_COMMON_UART_ENABLE
        // ble数据直通串口
        common_uart_send_data(buffer, buffer_size);
#endif

        // test主从连接数据控制
#if BLE_SLAVE_CLIENT_LED_OP_EN
        on_off_opcode_t *op = (on_off_opcode_t *)buffer;
        led_onoff_op(op);
#endif

        /* // 收发测试，自动发送收到的数据;for test */
        /* if (ble_comm_att_check_send(connection_handle, buffer_size) && */
        /*         ble_gatt_server_characteristic_ccc_get(trans_con_handle, ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE)) { */
        /*     log_info("-loop send1\n"); */
        /*     ble_comm_att_send_data(connection_handle, ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC); */
        /* } */
        break;

    case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
        log_info("\n-ae03_rx(%d):", buffer_size);
        log_info_hexdump(buffer, buffer_size);

        //收发测试，自动发送收到的数据;for test
        if (ble_comm_att_check_send(connection_handle, buffer_size) && \
            ble_gatt_server_characteristic_ccc_get(trans_con_handle, ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE)) {
            log_info("-loop send2\n");
            ble_comm_att_send_data(connection_handle, ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
        }
        break;

#if RCSP_BTMATE_EN
    case ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE:
        ble_op_latency_skip(connection_handle, 0xffff); //
        ble_gatt_server_set_update_send(connection_handle, ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE, ATT_OP_AUTO_READ_CCC);
#if (defined(BT_CONNECTION_VERIFY) && (0 == BT_CONNECTION_VERIFY))
        JL_rcsp_auth_reset();       //hid设备试能nofity的时候reset auth保证APP可以重新连接
#endif
        rcsp_init();
        rcsp_dev_select(RCSP_BLE);
#endif
        /* trans_send_connetion_updata_deal(connection_handle); */
        log_info("------write ccc:%04x,%02x\n", handle, buffer[0]);
        ble_gatt_server_characteristic_ccc_set(connection_handle, handle, buffer[0]);
        break;

#if RCSP_BTMATE_EN
    case ATT_CHARACTERISTIC_ae01_02_VALUE_HANDLE:
        log_info("rcsp_read:%x\n", buffer_size);
        ble_gatt_server_receive_update_data(NULL, buffer, buffer_size);
        break;
#endif

    case ATT_CHARACTERISTIC_ae3b_01_VALUE_HANDLE:
        log_info("\n-ae3b_rx(%d):", buffer_size);
        log_info_hexdump(buffer, buffer_size);
        break;

    default:
        break;
    }
    return 0;
}


/*************************************************************************************************/
/*!
 *  \brief      组织adv包数据，放入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int trans_make_set_adv_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = trans_adv_data;

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, FLAGS_GENERAL_DISCOVERABLE_MODE | FLAGS_EDR_NOT_SUPPORTED, 1);
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF30, 2);

    const char *gap_name = ble_comm_get_gap_name();
    uint8_t name_len = strlen(gap_name);
    uint8_t vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len < vaild_len) {
        offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_name, name_len);
        trans_adv_name_ok = 1;
    } else {
        trans_adv_name_ok = 0;
    }

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***trans_adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("trans_adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    trans_server_adv_config.adv_data_len = offset;
    trans_server_adv_config.adv_data = trans_adv_data;
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      组织rsp包数据，放入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int trans_make_set_rsp_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = trans_scan_rsp_data;

#if RCSP_BTMATE_EN
    uint8_t  tag_len = sizeof(user_tag_string);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, (void *)user_tag_string, tag_len);
#endif

    if (!trans_adv_name_ok) {
        const char *gap_name = ble_comm_get_gap_name();
        uint8_t name_len = strlen(gap_name);
        uint8_t vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
        if (name_len > vaild_len) {
            name_len = vaild_len;
        }
        offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_name, name_len);
    }

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
    trans_server_adv_config.rsp_data_len = offset;
    trans_server_adv_config.rsp_data = trans_scan_rsp_data;
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      配置广播参数
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note      开广播前配置都有效
 */
/*************************************************************************************************/
static void trans_adv_config_set(void)
{
    int ret = 0;
    ret |= trans_make_set_adv_data();
    ret |= trans_make_set_rsp_data();

    trans_server_adv_config.adv_interval = ADV_INTERVAL_MIN;
    trans_server_adv_config.adv_auto_do = 1;
    trans_server_adv_config.adv_type = ADV_IND;
    trans_server_adv_config.adv_channel = ADV_CHANNEL_ALL;
    memset(trans_server_adv_config.direct_address_info, 0, 7);

    if (ret) {
        log_info("adv_setup_init fail!!!\n");
        return;
    }
    ble_gatt_server_set_adv_config(&trans_server_adv_config);
}

/*************************************************************************************************/
/*!
 *  \brief      server init初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void trans_server_init(void)
{
    log_info("%s", __FUNCTION__);
    ble_gatt_server_set_profile(trans_profile_data, sizeof(trans_profile_data));
    trans_adv_config_set();
}

/*************************************************************************************************/
/*!
 *  \brief      断开连接
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void trans_disconnect(void)
{
    log_info("%s", __FUNCTION__);
    if (trans_con_handle) {
        ble_comm_disconnect(trans_con_handle);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      协议栈初始化前调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/

void bt_ble_before_start_init(void)
{
    log_info("%s", __FUNCTION__);
    ble_comm_init(&trans_gatt_control_block);
}


/*************************************************************************************************/
/*!
 *  \brief   测试发数
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void trans_test_send_data(void)
{
#if TEST_TRANS_CHANNEL_DATA
    static uint32_t count = 0;
    static uint32_t send_index;

    int i, ret = 0;
    int send_len = TEST_PAYLOAD_LEN;
    uint32_t time_index_max = 1000 / TEST_TRANS_TIMER_MS;

    if (!trans_con_handle) {
        return;
    }

    send_index++;

#if TEST_TRANS_NOTIFY_HANDLE
    do {
        if (ble_comm_att_check_send(trans_con_handle, send_len) && ble_gatt_server_characteristic_ccc_get(trans_con_handle, TEST_TRANS_NOTIFY_HANDLE + 1)) {
            count++;
            ret = ble_comm_att_send_data(trans_con_handle, TEST_TRANS_NOTIFY_HANDLE, &count, send_len, ATT_OP_AUTO_READ_CCC);
            if (!ret) {
                /* putchar('T'); */
                trans_send_test_count += send_len;
            }
        } else {
            break;
        }
    } while (ret == 0);
#endif

    if (send_index >= time_index_max) {
        if (trans_send_test_count) {
            log_info(">>>>>> send_rate= %d byte/s\n", trans_send_test_count);
        }
        send_index = 0;
        trans_send_test_count = 0;

        if (trans_recieve_test_count) {
            log_info("<<<<<<< recieve_rate= %d byte/s\n", trans_recieve_test_count);
            trans_recieve_test_count = 0;
        }
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      控制应答对方READ/WRITE行为的响应包RESPONSE的回复
 *
 *  \param      [in]流控使能 en: 1-停止收数 or 0-继续收数
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int trans_flow_enable(uint8_t en)
{
    int ret = -1;

#if ATT_DATA_RECIEVT_FLOW
    if (trans_con_handle) {
        att_server_flow_hold(trans_con_handle, en);
        ret = 0;
        log_info("trans_flow_enable:%d\n", en);
    }
#endif

    return ret;
}

//for test
static void trans_timer_flow_test(void)
{
    static uint8_t sw = 0;
    if (trans_con_handle) {
        sw = !sw;
        trans_flow_enable(sw);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      模块初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_init(void)
{
    log_info("%s\n", __FUNCTION__);
    log_info("ble_file: %s", __FILE__);

    ble_comm_set_config_name(bt_get_local_name(), 1);

    trans_con_handle = 0;
    trans_server_init();

#if ATT_DATA_RECIEVT_FLOW
    log_info("att_server_flow_enable\n");
    att_server_flow_enable(1);
    sys_timer_add(0, trans_timer_flow_test, 5000);
#endif

    ble_module_enable(1);

#if TEST_TRANS_CHANNEL_DATA
    sys_timer_add(0, trans_test_send_data, TEST_TRANS_TIMER_MS);
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      模块退出
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_exit(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_module_enable(0);
    ble_comm_exit();
}

/*************************************************************************************************/
/*!
 *  \brief      模块开发使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_module_enable(uint8_t en)
{
    ble_comm_module_enable(en);
}


void trans_key_deal_test(uint8_t key_type, uint8_t key_value)
{
#if TCFG_LED_ENABLE
    led_operate(LED_KEY_UP);
#endif
    log_info(">>>>>>>>>>trans key test");
}

/*************************************************************************************************/
/*!
 *  \brief      testbox 按键测试
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
/* void ble_server_send_test_key_num(uint8_t key_num) */
/* { */
/*     if (trans_con_handle) { */
/*         if (get_remote_test_flag()) { */
/*             ble_op_test_key_num(trans_con_handle, key_num); */
/*         } else { */
/*             log_info("-not conn testbox\n"); */
/*         } */
/*     } */
/* } */


#endif


