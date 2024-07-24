/*********************************************************************************************
    *   Filename        : ble_at_char_server.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2023-10-05 10:09

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
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
#include "app_comm_proc.h"
#include "le_gatt_common.h"
#include "ble_at_char_server.h"
#include "ble_at_char_profile.h"
#include "at_char_cmds.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if CONFIG_APP_AT_CHAR_COM && CONFIG_BT_GATT_SERVER_NUM

#define LOG_TAG         "[ble_at_server]"
#include "log.h"

// 广播周期 (unit:0.625ms)
#define ADV_INTERVAL_MIN          (160 * 5)//
//配对信息表
#define PER_PAIR_BOND_ENABLE      CONFIG_BT_SM_SUPPORT_ENABLE
#define PER_PAIR_BOND_TAG         0x53
#define DIRECT_ADV_MAX_CNT        2
#define EIR_TAG_STRING            0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'

//---------------
//连接参数更新请求设置
//是否使能参数请求更新,0--disable, 1--enable
static uint8_t ble_at_server_connection_update_enable = 1; ///0--disable, 1--enable
//参数表
static const struct conn_update_param_t ble_at_server_connection_param_table[] = {
    {16, 24, 10, 600},//11
    {12, 28, 10, 600},//3.7
    {8,  20, 10, 600},
};

//共可用的参数组数
#define CONN_PARAM_TABLE_CNT      (sizeof(ble_at_server_connection_param_table)/sizeof(struct conn_update_param_t))
//----------------------
static const char ble_at_server_user_tag_string[] = {EIR_TAG_STRING};
static const char ble_at_server_rsp_data[] = {'b', 'l', 'e', '_', 'a', 't', '_', 't', 'e', 's', 't'};
static char ble_at_server_device_name[BT_NAME_LEN_MAX] = "ble_at_test";
static const uint8_t ble_at_server_cur_dev_cid = 8; //fixed

static uint8_t ble_at_server_vm_name_buf[BT_NAME_LEN_MAX + 1];
static uint8_t ble_at_server_adv_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t ble_at_server_scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t ble_at_server_read_write_buf[4];
static uint8_t ble_at_server_pair_bond_enable;
static uint8_t ble_at_server_pair_bond_info[8]; //tag + addr_type + address
static uint8_t ble_at_server_direct_adv_count;  //定向广播次数
static uint8_t ble_at_server_cur_peer_addr_info[7];//当前连接对方地址信息
static uint8_t ble_at_server_adv_name_ok;//name 优先存放在ADV包

static uint8_t ble_at_server_gap_device_name_len;
static uint8_t ble_at_server_adv_data_len;
static uint8_t ble_at_server_scan_rsp_data_len;

static uint16_t ble_at_server_conn_handle;
static uint16_t ble_at_server_interval = ADV_INTERVAL_MIN;

static adv_cfg_t ble_at_server_adv_config;

//------------------------------------------------------
static void ble_at_server_adv_config_set(void);
static uint16_t ble_at_server_att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int ble_at_server_att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int ble_at_server_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param);

const gatt_server_cfg_t ble_at_server_init_cfg = {
    .att_read_cb = &ble_at_server_att_read_callback,
    .att_write_cb = &ble_at_server_att_write_callback,
    .event_packet_handler = &ble_at_server_event_packet_handler,
};


/*************************************************************************************************/
/*!
 *  \brief   vm 绑定对方信息读写
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int ble_at_server_pair_vm_do(uint8_t *info, uint8_t info_len, uint8_t rw_flag)
{
#if PER_PAIR_BOND_ENABLE
    int ret;
    int vm_len = info_len;

    log_info("-conn_pair_info vm_do:%d\n", rw_flag);
    if (rw_flag == 0) {
        ret = syscfg_read(CFG_BLE_BONDING_REMOTE_INFO, (uint8_t *)info, vm_len);
        if (!ret) {
            log_info("-null--\n");
            memset(info, 0xff, info_len);
        }
        if (info[0] != PER_PAIR_BOND_TAG) {
            return -1;
        }
    } else {
        syscfg_write(CFG_BLE_BONDING_REMOTE_INFO, (uint8_t *)info, vm_len);
    }
#endif
    return 0;
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

//清配对信息
int ble_at_server_clear_pair(void)
{
#if PER_PAIR_BOND_ENABLE
    ble_gatt_server_disconnect_all();
    memset(ble_at_server_pair_bond_info, 0, sizeof(ble_at_server_pair_bond_info));
    ble_at_server_pair_vm_do(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info), 1);
    if (BLE_ST_ADV == ble_gatt_server_get_work_state()) {
        ble_gatt_server_adv_enable(0);
        ble_at_server_adv_config_set();
        ble_gatt_server_adv_enable(1);
    }
#endif
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief   配置连接参数更新
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_server_send_connetion_update(uint16_t conn_handle)
{
    if (ble_at_server_connection_update_enable) {
        if (0 == ble_gatt_server_connetion_update_request(conn_handle, ble_at_server_connection_param_table, CONN_PARAM_TABLE_CNT)) {
            ble_at_server_connection_update_enable = 0;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief   回连状态，主动使能通知
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_server_resume_all_ccc_enable(uint16_t conn_handle, uint8_t update_request)
{
    log_info("resume_all_ccc_enable\n");

#if RCSP_BTMATE_EN
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);
#endif

    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_INDICATE);
    ble_gatt_server_characteristic_ccc_set(conn_handle, ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE, ATT_OP_NOTIFY);

    if (update_request) {
        ble_at_server_send_connetion_update(conn_handle);
    }
}

/*************************************************************************************************/
/*!
 *  \brief   处理gatt_common 模块返回的事件，hci & gatt
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int ble_at_server_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param)
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
        ble_at_server_conn_handle = little_endian_read_16(packet, 0);
        log_info("connection_handle:%04x\n", ble_at_server_conn_handle);
        log_info("peer_address_info:");
        log_info_hexdump(&ext_param[7], 7);
        memcpy(ble_at_server_cur_peer_addr_info, &ext_param[7], 7);
        ble_at_server_connection_update_enable = 1;
        ble_at_server_pair_bond_enable = 0;
        break;

    case GATT_COMM_EVENT_DISCONNECT_COMPLETE:
        log_info("disconnect_handle:%04x,reason= %02x\n", little_endian_read_16(packet, 0), packet[2]);

        ble_at_server_conn_handle = 0;

        //TODO 可以推msg到app处理
#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif

        if (ble_at_server_conn_handle == little_endian_read_16(packet, 0)) {
            ble_at_server_conn_handle = 0;
        }

        if (packet[2] == 8) {
            if (ble_at_server_pair_bond_info[0] == PER_PAIR_BOND_TAG) {
                ble_at_server_direct_adv_count = DIRECT_ADV_MAX_CNT;
                ble_at_server_adv_config_set();
            }
        }
        at_send_disconnect(ble_at_server_cur_dev_cid);
        break;

    case GATT_COMM_EVENT_ENCRYPTION_CHANGE:
        log_info("ENCRYPTION_CHANGE:handle=%04x,state=%d,process =%02x", little_endian_read_16(packet, 0), packet[2], packet[3]);
        if (packet[3] == LINK_ENCRYPTION_RECONNECT) {
            log_info("reconnect...\n");
            ble_at_server_resume_all_ccc_enable(little_endian_read_16(packet, 0), 1);
            if (memcmp(&ble_at_server_pair_bond_info[1], ble_at_server_cur_peer_addr_info, 7)) {
                log_info("update peer\n");
                memcpy(&ble_at_server_pair_bond_info[1], ble_at_server_cur_peer_addr_info, 7);
                ble_at_server_pair_bond_info[0] = PER_PAIR_BOND_TAG;
                ble_at_server_pair_vm_do(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info), 1);
            }
        } else {
            log_info("first pair...\n");
#if PER_PAIR_BOND_ENABLE
            memcpy(&ble_at_server_pair_bond_info[1], ble_at_server_cur_peer_addr_info, 7);
            ble_at_server_pair_bond_info[0] = PER_PAIR_BOND_TAG;
            ble_at_server_pair_vm_do(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info), 1);
            log_info("bonding remote");
            put_buf(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info));
#endif
        }
        break;

    case GATT_COMM_EVENT_CONNECTION_UPDATE_COMPLETE:
        log_info("conn_param update_complete:%04x\n", little_endian_read_16(packet, 0));
        break;

    case GATT_COMM_EVENT_DIRECT_ADV_TIMEOUT:
        log_info("DIRECT_ADV_TIMEOUT:%d", ble_at_server_direct_adv_count);
        if (ble_at_server_direct_adv_count) {
            ble_at_server_direct_adv_count--;
        } else {
            ble_at_server_adv_config_set();
        }
        break;

    case GATT_COMM_EVENT_SERVER_STATE:
        log_info("server_state: handle=%02x,%02x\n", little_endian_read_16(packet, 1), packet[0]);
        break;


    case GATT_COMM_EVENT_CONNECTION_UPDATE_REQUEST_RESULT:
        break;

    case GATT_COMM_EVENT_MTU_EXCHANGE_COMPLETE:
        log_info("con_handle= %02x, ATT MTU = %u\n", little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
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
// @param offset defines start of attribute value
static uint16_t ble_at_server_att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
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
        att_value_len = sizeof(ble_at_server_read_write_buf);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &ble_at_server_read_write_buf[offset], buffer_size);
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
static int ble_at_server_att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    uint16_t tmp16;

    uint16_t handle = att_handle;

    log_info("write_callback,conn_handle =%04x, handle =%04x,size =%d\n", connection_handle, handle, buffer_size);

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        at_send_rx_cid_data(ble_at_server_cur_dev_cid, buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
        at_send_string("OK");
        at_send_connected(ble_at_server_cur_dev_cid);
        break;

    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
        ble_at_server_send_connetion_update(connection_handle);
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        ble_gatt_server_characteristic_ccc_set(connection_handle, handle, buffer[0]);
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        tmp16 = sizeof(ble_at_server_read_write_buf);
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }
        memcpy(&ble_at_server_read_write_buf[offset], buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:
        log_info("\n-ae01_rx(%d):", buffer_size);
        put_buf(buffer, buffer_size);

        //收发测试，自动发送收到的数据;for test
        if (ble_comm_att_check_send(connection_handle, buffer_size)) {
            log_info("-loop send1\n");
            ble_comm_att_send_data(connection_handle, ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
        }
        break;

    case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
        log_info("\n-ae_rx(%d):", buffer_size);
        put_buf(buffer, buffer_size);

        //收发测试，自动发送收到的数据;for test
        if (ble_comm_att_check_send(connection_handle, buffer_size)) {
            log_info("-loop send2\n");
            ble_comm_att_send_data(connection_handle, ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
        }
        break;

#if RCSP_BTMATE_EN
    case ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE:
        ble_op_latency_skip(connection_handle, 0xffff); //
        ble_gatt_server_set_update_send(connection_handle, ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE, ATT_OP_AUTO_READ_CCC);
#endif
        /* ble_at_server_send_connetion_update(connection_handle); */
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
        put_buf(buffer, buffer_size);
        break;

    default:
        break;
    }
    return 0;
}
/*************************************************************************************************/
/*!
 *  \brief   生成adv包数据，写入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int ble_at_server_make_set_adv_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = ble_at_server_adv_data;

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, FLAGS_GENERAL_DISCOVERABLE_MODE | FLAGS_EDR_NOT_SUPPORTED, 1);
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF30, 2);

    /* const char *gap_name = ble_at_server_device_name; */
    /* uint8_t name_len = strlen(gap_name); */
    /* uint8_t vaild_len = ADV_RSP_PACKET_MAX - (offset + 2); */
    /* if (name_len < vaild_len) { */
    /*     offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_name, name_len); */
    /*     ble_at_server_adv_name_ok = 1; */
    /* } else { */
    /*     ble_at_server_adv_name_ok = 0; */
    /* } */

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***ble_at_server_adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("ble_at_server_adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    ble_at_server_adv_data_len = offset;
    ble_at_server_adv_config.adv_data_len = offset;
    ble_at_server_adv_config.adv_data = ble_at_server_adv_data;
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief   生成rsp包数据，写入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int ble_at_server_make_set_rsp_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = ble_at_server_scan_rsp_data;

#if RCSP_BTMATE_EN
    uint8_t  tag_len = sizeof(ble_at_server_user_tag_string);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, (void *)ble_at_server_user_tag_string, tag_len);
#endif

    if (!ble_at_server_adv_name_ok) {
        ble_at_server_get_name((uint8_t *)ble_at_server_device_name);
        const char *gap_name = ble_at_server_device_name;
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
    ble_at_server_scan_rsp_data_len = offset;
    ble_at_server_adv_config.rsp_data_len = offset;
    ble_at_server_adv_config.rsp_data = ble_at_server_scan_rsp_data;
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief   广播数据构建
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/

//广播参数设置
static void ble_at_server_adv_config_set(void)
{
    int ret = 0;
    ret |= ble_at_server_make_set_adv_data();
    ret |= ble_at_server_make_set_rsp_data();

    ble_at_server_adv_config.adv_interval = ble_at_server_interval;
    ble_at_server_adv_config.adv_auto_do = 1;
    ble_at_server_adv_config.adv_channel = ADV_CHANNEL_ALL;
    memcpy(ble_at_server_adv_config.direct_address_info, &ble_at_server_pair_bond_info[1], 7);

    if (ble_at_server_direct_adv_count) {
        ble_at_server_adv_config.adv_type = ADV_DIRECT_IND;
        ble_at_server_direct_adv_count--;
    } else {
        ble_at_server_adv_config.adv_type = ADV_IND;
    }

    if (ret) {
        log_info("adv_setup_init fail!!!\n");
        return;
    }
    ble_gatt_server_set_adv_config(&ble_at_server_adv_config);
}

//server init
void ble_at_server_init(void)
{
    log_info("%s", __FUNCTION__);

#if PER_PAIR_BOND_ENABLE
    if (0 == ble_at_server_pair_vm_do(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info), 0)) {
        log_info("server already bond dev");
        put_buf(ble_at_server_pair_bond_info, sizeof(ble_at_server_pair_bond_info));
        ble_at_server_direct_adv_count = DIRECT_ADV_MAX_CNT;
    }
#endif

    ble_gatt_server_set_profile(at_char_profile_data, sizeof(at_char_profile_data));
    /* ble_at_server_adv_config_set(); */

}

static int app_send_user_data(uint16_t handle, uint8_t *data, uint16_t len, uint8_t handle_type)
{
    uint32_t ret = APP_BLE_NO_ERROR;

    if (!ble_at_server_conn_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!multi_att_get_ccc_config(ble_at_server_conn_handle, handle + 1)) {
        log_info("fail,no write ccc!!!,%04x\n", handle + 1);
        return APP_BLE_NO_WRITE_CCC;
    }

    ret = ble_op_multi_att_send_data(ble_at_server_conn_handle, handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}

//-----------------------------------对外接口----------------------------------------------//
int ble_at_server_send_data(uint8_t cid, uint8_t *packet, uint16_t size)
{
    if (cid == ble_at_server_cur_dev_cid) {
        return app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, packet, size, ATT_OP_AUTO_READ_CCC);
    }
    return 0;
}

int ble_at_server_set_address(uint8_t *addr)
{
    le_controller_set_mac(addr);
    return APP_BLE_NO_ERROR;
}

int ble_at_server_get_address(uint8_t *addr)
{
    le_controller_get_mac(addr);
    return APP_BLE_NO_ERROR;
}

int ble_at_server_set_name(uint8_t *name, uint8_t len)
{
    uint8_t ret = 0;
    if (len > BT_NAME_LEN_MAX - 1) {
        len = BT_NAME_LEN_MAX - 1;
    }
    memcpy(ble_at_server_device_name, name, len);
    ble_at_server_gap_device_name_len = len;

    ble_at_server_vm_name_buf[0] = len;
    memcpy(&ble_at_server_vm_name_buf[1], name, len);

    ret = syscfg_write(AT_CHAR_DEV_NAME, ble_at_server_vm_name_buf, len + 1);
    if (ret == len + 1) {
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_BUFF_ERROR;
    }
}

int ble_at_server_get_name(uint8_t *name)
{
    uint8_t ret = 0;
    uint8_t len = 0;
    ret = syscfg_read(AT_CHAR_DEV_NAME, ble_at_server_vm_name_buf, 1);
    len = ble_at_server_vm_name_buf[0];
    if (len == 0) { //没有东西,使用默认名字
        goto get_name;
    }

    ret = syscfg_read(AT_CHAR_DEV_NAME, ble_at_server_vm_name_buf, len + 1);
    log_info_hexdump(ble_at_server_vm_name_buf, ret);
    if (ret != len + 1) {
        return 0;
    }
    memcpy(ble_at_server_device_name, &ble_at_server_vm_name_buf[1], len);
    ble_at_server_gap_device_name_len = len;

get_name:
    ble_at_server_gap_device_name_len = strlen(ble_at_server_device_name);
    memcpy(name, ble_at_server_device_name, ble_at_server_gap_device_name_len);
    return ble_at_server_gap_device_name_len;
}

int ble_at_server_disconnect(void)
{
    log_info(">>>>ble_at_server disconnect");
    if (ble_at_server_conn_handle) {
        return ble_comm_disconnect(ble_at_server_conn_handle);
    }
    return -1;
}

int ble_at_server_set_adv_data(uint8_t *data, uint8_t len)
{
    log_info("set_adv_data(%d)", len);
    ble_at_server_adv_data_len = len;
    if (len) {
        log_info_hexdump(data, len);
        if (len > ADV_RSP_PACKET_MAX) {
            log_info("len err!\n");
            return APP_BLE_OPERATION_ERROR;
        }
        memcpy(ble_at_server_adv_data, data, len);
    }
    return APP_BLE_NO_ERROR;
}

uint8_t *ble_at_server_get_adv_data(uint8_t *len)
{
    *len = ble_at_server_adv_data_len;
    return ble_at_server_adv_data;
}

int ble_at_server_set_rsp_data(uint8_t *data, uint8_t len)
{
    log_info("set_rsp_data(%d)", len);
    ble_at_server_scan_rsp_data_len = len;
    if (len) {
        log_info_hexdump(data, len);
        if (len > ADV_RSP_PACKET_MAX) {
            log_info("len err!\n");
            return APP_BLE_OPERATION_ERROR;
        }
        memcpy(ble_at_server_scan_rsp_data, data, len);
    }
    return APP_BLE_NO_ERROR;
}

uint8_t *ble_at_server_get_rsp_data(uint8_t *len)
{
    *len = ble_at_server_scan_rsp_data_len;
    return ble_at_server_scan_rsp_data;
}

int ble_at_server_adv_enable(uint8_t enable)
{
    ble_gatt_server_module_enable(enable);
    return 0;
}

int ble_at_server_get_adv_state(void)
{
    if ((ble_gatt_server_get_work_state()) == BLE_ST_ADV) {
        return 1;
    }
    return 0;
}

int ble_at_server_set_adv_interval(uint16_t value)
{
    ble_at_server_interval = value;
    return 0;
}

int ble_at_server_get_adv_interval(void)
{
    return ble_at_server_interval;
}

void ble_test_auto_adv(uint8_t en)
{
    log_info("ble_test_auto_adv = %d", en);
    ble_at_server_adv_config_set();
    /* ble_at_server_set_name(ble_at_server_device_name, 11); */
    ble_at_server_adv_enable(en);
}


//server exit
void ble_at_server_exit(void)
{
    log_info("%s", __FUNCTION__);
}

void ble_at_server_key_test(uint8_t key_type, uint8_t key_value)
{

}
#endif


