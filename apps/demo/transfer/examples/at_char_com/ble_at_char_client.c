/*********************************************************************************************
    *   Filename        : ble_at_char_client.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2023-10-05 10:09

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include "msg.h"
#include "gpio.h"
#include <stdlib.h>
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
#include "my_malloc.h"
#include "app_modules.h"
#include "le_gatt_common.h"
#include "ble_at_char_client.h"
#include "at_char_cmds.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if CONFIG_APP_AT_CHAR_COM && CONFIG_BT_GATT_CLIENT_NUM

#define LOG_TAG         "[ble_at_client]"
#include "log.h"

static struct ctl_pair_info_t ble_at_client_cur_conn_info;
static struct ctl_pair_info_t ble_at_client_record_bond_info[SUPPORT_MAX_GATT_CLIENT];
static scan_conn_cfg_t ble_at_client_scan_cfg;

static uint8_t  ble_at_client_search_profile = 1; /*配对回连是否搜索profile*/
static uint8_t  ble_at_client_pair_redo; /*配对回连keymiss,是否重新执行配对*/
static uint8_t  ble_at_client_cur_dev_cid = 1;    // 主机at通道,目前仅支持单主机
static uint16_t ble_at_client_write_handle;

static struct conn_update_param_t ble_at_client_conn_param = {
    .interval_min = SET_CONN_INTERVAL,  //(unit:0.625ms)
    .interval_max = SET_CONN_INTERVAL,  //(unit:0.625ms)
    .latency = SET_CONN_LATENCY,       //(unit: interval)
    .timeout = SET_CONN_TIMEOUT,      //(unit:10ms)
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//指定搜索uuid
static target_uuid_t  ble_at_client_search_uuid_table[MAX_UUID_MATCH_NUM] = {

    // for uuid16
    // PRIMARY_SERVICE, ae30
    // CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC,
    // CHARACTERISTIC,  ae02, NOTIFY,

    {
        .services_uuid16 = 0xae30,
        .characteristic_uuid16 = 0xae01,
        .opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    },

    {
        .services_uuid16 = 0xae30,
        .characteristic_uuid16 = 0xae02,
        .opt_type = ATT_PROPERTY_NOTIFY,
    },

    //for uuid128,sample
    //	PRIMARY_SERVICE, 0000F530-1212-EFDE-1523-785FEABCD123
    //	CHARACTERISTIC,  0000F531-1212-EFDE-1523-785FEABCD123, NOTIFY,
    //	CHARACTERISTIC,  0000F532-1212-EFDE-1523-785FEABCD123, WRITE_WITHOUT_RESPONSE | DYNAMIC,
    /*
    	{
    		.services_uuid16 = 0,
    		.services_uuid128 =       {0x00,0x00,0xF5,0x30 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23} ,
    		.characteristic_uuid16 = 0,
    		.characteristic_uuid128 = {0x00,0x00,0xF5,0x31 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23},
    		.opt_type = ATT_PROPERTY_NOTIFY,
    	},

    	{
    		.services_uuid16 = 0,
    		.services_uuid128 =       {0x00,0x00,0xF5,0x30 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23} ,
    		.characteristic_uuid16 = 0,
    		.characteristic_uuid128 = {0x00,0x00,0xF5,0x32 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23},
    		.opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    	},
    */

};

//配置多个扫描匹配设备
static const uint8_t ble_at_client_test_remoter_name1[] = "AW31N_BLE(BLE)";
static const client_match_cfg_t ble_at_client_match_table[] = {
    {
        .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
        .compare_data_len = sizeof(ble_at_client_test_remoter_name1) - 1, //去结束符
        .compare_data = ble_at_client_test_remoter_name1,
        .filter_pdu_bitmap = 0,
    },
};

//配置扫描匹配连接的设备，已经连上后搜索匹配的profile uuid
static gatt_search_cfg_t ble_at_client_search_config = {
    .match_devices = ble_at_client_match_table,
    .match_devices_count = (sizeof(ble_at_client_match_table) / sizeof(client_match_cfg_t)),
    .match_rssi_enable = 0,

    .search_uuid_group = ble_at_client_search_uuid_table,
    .search_uuid_count = (sizeof(ble_at_client_search_uuid_table) / sizeof(target_uuid_t)),
    .auto_enable_ccc = 1,
};

//配置扫描匹配连接绑定后的设备
static gatt_search_cfg_t ble_at_client_bond_config = {
    .match_devices = NULL,
    .match_devices_count = 0,
    .match_rssi_enable = 0,

    .search_uuid_group = ble_at_client_search_uuid_table,
    .search_uuid_count = (sizeof(ble_at_client_search_uuid_table) / sizeof(target_uuid_t)),
    .auto_enable_ccc = 1,
};

//带绑定的设备搜索
static client_match_cfg_t *ble_at_client_bond_device_table;
static uint8_t  ble_at_client_bond_device_cnt;

static int ble_at_client_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param);
static void ble_at_client_conn_config_set(struct ctl_pair_info_t *pair_info);
const gatt_client_cfg_t ble_at_client_init_cfg = {
    .event_packet_handler = ble_at_client_event_packet_handler,
};


/*************************************************************************************************/
/*!
 *  \brief   测试write数据操作
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_client_test_write(void)
{
#if AT_CLIENT_WRITE_SEND_DATA
    static uint32_t count = 0;
    int i, ret = 0;
    uint16_t tmp_handle;

    count++;
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        tmp_handle = ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT);
        if (tmp_handle && ble_at_client_write_handle) {
            ret = ble_comm_att_send_data(tmp_handle, ble_at_client_write_handle, (uint8_t *)&count, 16, ATT_OP_WRITE_WITHOUT_RESPOND);
            log_info("test_write:%04x,%d", tmp_handle, ret);
        }
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief   测试write指定数据操作
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_client_write_data(uint8_t *data, uint16_t send_len)
{
#if AT_CLIENT_WRITE_SEND_DATA
    int i, ret = 0;
    uint16_t tmp_handle;
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        tmp_handle = ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT);
        if (ble_comm_att_check_send(tmp_handle, send_len)) {
            ret = ble_comm_att_send_data(tmp_handle, ble_at_client_write_handle, data, send_len, ATT_OP_WRITE_WITHOUT_RESPOND);
        }
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief   更新连接设备的匹配配置
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_client_reflash_bond_search_config(void)
{
    int i;

    if (!ble_at_client_bond_device_table) {
        log_info("device tabl is null!");
        return;
    }

    /*建立对应关系 配对表(匹配地址) + 默认匹配表 ble_at_client_match_table*/
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        ble_at_client_bond_device_table[i].create_conn_mode = BIT(CLI_CREAT_BY_ADDRESS);
        ble_at_client_bond_device_table[i].compare_data_len = 6;
        ble_at_client_bond_device_table[i].compare_data = &ble_at_client_record_bond_info[i].peer_address_info[1];

        if (CLIENT_PAIR_BOND_TAG == ble_at_client_record_bond_info[i].head_tag) {
            log_info("set bond search: %d\n", i);
            ble_at_client_bond_device_table[i].filter_pdu_bitmap = BIT(EVENT_ADV_SCAN_IND) | BIT(EVENT_ADV_NONCONN_IND);
        } else {
            ble_at_client_bond_device_table[i].filter_pdu_bitmap = EVENT_DEFAULT_REPORT_BITMAP;
        }
    }
    memcpy(&ble_at_client_bond_device_table[SUPPORT_MAX_GATT_CLIENT], ble_at_client_match_table, sizeof(ble_at_client_match_table));

    ble_at_client_bond_config.match_devices = ble_at_client_bond_device_table;
    ble_at_client_bond_config.match_devices_count = ble_at_client_bond_device_cnt;
    ble_gatt_client_set_search_config(&ble_at_client_bond_config);
}

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
static int ble_at_client_pair_vm_do(struct ctl_pair_info_t *info, uint8_t rw_flag)
{
    int ret = 0;

#if CLIENT_PAIR_BOND_ENABLE
    int i;
    int uint_len = sizeof(struct ctl_pair_info_t);

    log_info("-conn_pair_info vm_do:%d\n", rw_flag);

    if (rw_flag == 0) {
        ret = syscfg_read(CFG_BLE_BONDING_REMOTE_INFO2, (uint8_t *)&ble_at_client_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
        if (!ret) {
            log_info("-null--\n");
            memset((uint8_t *)&ble_at_client_record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
            ret = 1;
        }

        /*检查合法性*/
        for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
            if (CLIENT_PAIR_BOND_TAG != ble_at_client_record_bond_info[i].head_tag || ble_at_client_record_bond_info[i].pair_flag != 1) {
                memset((uint8_t *)&ble_at_client_record_bond_info[i], 0xff, uint_len);
            }
        }

    } else {
        int fill_index = -1;

        if (info == NULL) {
            log_info("vm clear\n");
            memset((uint8_t *)&ble_at_client_record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
        } else {

            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (0 == memcmp(info, &ble_at_client_record_bond_info[i], uint_len)) {
                    log_info("dev in table\n");
                    return ret;
                }
            }

            log_info_hexdump((uint8_t *)&ble_at_client_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
            log_info_hexdump((uint8_t *)info, uint_len);
            log_info("write vm start\n");
            log_info("find table\n");

            /*find same*/
            if (fill_index == -1) {
                for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    if (0 == memcmp(info->peer_address_info, &ble_at_client_record_bond_info[i].peer_address_info, 7)) {
                        log_info("replace old\n");
                        fill_index = i;
                        break;
                    }
                }
            }

            /*find idle*/
            if (fill_index == -1) {
                for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    if (CLIENT_PAIR_BOND_TAG != ble_at_client_record_bond_info[i].head_tag) {
                        log_info("find idle\n");
                        fill_index = i;
                        break;
                    }
                }
            }

            /*find first*/
            if (fill_index == -1) {
                for (i = 1; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    memcpy(&ble_at_client_record_bond_info[i - 1], &ble_at_client_record_bond_info[i], uint_len);
                    ble_at_client_record_bond_info[i - 1].match_dev_id = i - 1; /*change id*/
                }
                log_info("replace first\n");
                fill_index = SUPPORT_MAX_GATT_CLIENT - 1;
            }

            /*连接顺序不同，handle是不一样,防止重复相同*/
            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (info->conn_handle == ble_at_client_record_bond_info[i].conn_handle) {
                    ble_at_client_record_bond_info[i].conn_handle = 0;
                    log_info("clear repeat handle\n");
                }
            }

            info->match_dev_id = fill_index;/*change id*/
            memcpy(&ble_at_client_record_bond_info[fill_index], info, uint_len);

        }

        syscfg_write(CFG_BLE_BONDING_REMOTE_INFO2, (uint8_t *)&ble_at_client_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    }

    log_info_hexdump((uint8_t *)&ble_at_client_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    ble_at_client_reflash_bond_search_config();/*配对表发生变化，更新scan设备匹配*/

#endif

    return ret;
}

/*************************************************************************************************/
/*!
 *  \brief   清除配对信息
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int ble_at_client_clear_pair(void)
{
#if CLIENT_PAIR_BOND_ENABLE
    ble_gatt_client_disconnect_all();
    memset(&ble_at_client_cur_conn_info, 0, sizeof(ble_at_client_cur_conn_info));
    ble_at_client_pair_vm_do(NULL, 1);
    if (BLE_ST_SCAN == ble_gatt_client_get_work_state()) {
        ble_gatt_client_scan_enable(0);
        ble_gatt_client_scan_enable(1);
    }
#endif
    return 0;
}

#if AT_CHAR_SCAN_SNED_ENABLE
static uint8_t at_send_adv_buf[128] = {0};
/*************************************************************************************************/
/*!
 *  \brief  scan到数据转发到AT UART
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_client_parse_adv_at_send(uint8_t *adv_address, uint8_t data_length, uint8_t *data, s8 rssi, uint8_t addr_type)
{
    uint8_t i, lenght, ad_type;
    uint8_t *adv_data_pt;
    uint32_t tmp32;
    uint8_t ret = 0;
    adv_data_pt = data;

    ret = hex_2_str(adv_address, 6, at_send_adv_buf); //dev_addr
    at_send_adv_buf[ret++] = ',';
    at_send_adv_buf[ret++] = addr_type + '0';     //addr_type
    at_send_adv_buf[ret++] = ',';
    sprintf(&at_send_adv_buf[ret], "%d", rssi);  //rssi
    ret = strlen(at_send_adv_buf);
    at_cmd_send(at_send_adv_buf, ret);
    //--------------------

    for (i = 0; i < data_length;) {
        if (*adv_data_pt == 0) {
            /* log_info("analyze end\n"); */
            break;
        }

        lenght = *adv_data_pt++;
        if (lenght >= data_length || (lenght + i) >= data_length) {
            /*过滤非标准包格式*/
            log_info("!!!error_adv_packet:");
            log_info_hexdump(data, data_length);
            break;
        }

        ad_type = *adv_data_pt++;
        i += (lenght + 1);

        switch (ad_type) {
        case HCI_EIR_DATATYPE_FLAGS:
            /* log_info("flags:%02x\n",adv_data_pt[0]); */
            break;

        case HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_128BIT_SERVICE_UUIDS:
            //-----------------new
            sprintf(at_send_adv_buf, "UUID:");
            ret = hex_2_str(adv_data_pt, lenght - 1, &at_send_adv_buf[5]);
            at_cmd_send(at_send_adv_buf, ret + 5);
            //--------------------
            /* log_info("service uuid:"); */
            /* log_info_hexdump(adv_data_pt, lenght - 1); */
            break;

        case HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME:
        case HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME:
            tmp32 = adv_data_pt[lenght - 1];
            adv_data_pt[lenght - 1] = 0;;
            log_info("remoter_name: %s\n", adv_data_pt);
            //---------------------new
            sprintf(at_send_adv_buf, "NAME:%s", adv_data_pt);
            at_cmd_send(at_send_adv_buf, strlen(at_send_adv_buf));
            //------------------------
            log_info_hexdump(adv_address, 6);
            break;

        case HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA:
            sprintf(at_send_adv_buf, "MANU:");
            ret = hex_2_str(adv_data_pt, lenght - 1, &at_send_adv_buf[5]);
            at_cmd_send(at_send_adv_buf, ret + 5);
            break;

        case HCI_EIR_DATATYPE_APPEARANCE_DATA:
            /* log_info("get_class_type:%04x\n",little_endian_read_16(adv_data_pt,0)); */
            break;

        default:
            /* log_info("unknow ad_type:"); */
            break;
        }
    }
}
#endif

/*************************************************************************************************/
/*!
 *  \brief   处理gatt 回调的事件，hci & gatt
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int ble_at_client_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param)
{
    /* log_info("event: %02x,size= %d\n",event,size); */
    switch (event) {
    case GATT_COMM_EVENT_GATT_DATA_REPORT: {
        att_data_report_t *report_data = (void *)packet;
        log_info("data_report:hdl=%04x,pk_type=%02x,size=%d\n", report_data->conn_handle, report_data->packet_type, report_data->blob_length);
        log_info_hexdump(report_data->blob, report_data->blob_length);

        switch (report_data->packet_type) {
        case GATT_EVENT_NOTIFICATION://notify
            break;
        case GATT_EVENT_INDICATION://indicate
            break;
        case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT://read
            break;
        case GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT://read long
            break;
        default:
            break;
        }
    }
    break;

    case GATT_COMM_EVENT_CAN_SEND_NOW:
        break;

    case GATT_COMM_EVENT_CONNECTION_COMPLETE:
        log_info("connection_handle:%04x\n", little_endian_read_16(packet, 0));
        log_info("con_interval = %d\n", little_endian_read_16(ext_param, 14 + 0));
        log_info("con_latency = %d\n", little_endian_read_16(ext_param, 14 + 2));
        log_info("cnn_timeout = %d\n", little_endian_read_16(ext_param, 14 + 4));
        log_info("peer_address_info:");
        log_info_hexdump(&ext_param[7], 7);

        memcpy(ble_at_client_cur_conn_info.peer_address_info, &ext_param[7], 7);
        ble_at_client_cur_conn_info.conn_handle =   little_endian_read_16(packet, 0);
        ble_at_client_cur_conn_info.conn_interval = little_endian_read_16(ext_param, 14 + 0);
        ble_at_client_cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 14 + 2);
        ble_at_client_cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 14 + 4);
        ble_at_client_cur_conn_info.pair_flag = 0;
        ble_at_client_cur_conn_info.head_tag = 0;
        break;

    case GATT_COMM_EVENT_DISCONNECT_COMPLETE:
        log_info("disconnect_handle:%04x,reason= %02x\n", little_endian_read_16(packet, 0), packet[2]);

        //TODO 可以推msg到app处理
#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif
        at_send_disconnect(ble_at_client_cur_dev_cid);
        break;

    case GATT_COMM_EVENT_ENCRYPTION_CHANGE:
        log_info("ENCRYPTION_CHANGE:handle=%04x,state=%d,process =%d", little_endian_read_16(packet, 0), packet[2], packet[3]);

        if (packet[3] == LINK_ENCRYPTION_RECONNECT) {
            log_info("reconnect...\n");
        } else {
            log_info("first pair...\n");
        }

#if CLIENT_PAIR_BOND_ENABLE
        ble_at_client_cur_conn_info.head_tag = CLIENT_PAIR_BOND_TAG;
        ble_at_client_cur_conn_info.pair_flag = 1;
        ble_at_client_pair_vm_do(&ble_at_client_cur_conn_info, 1);
#endif
        break;

    case GATT_COMM_EVENT_CONNECTION_UPDATE_COMPLETE:
        log_info("conn_param update_complete:%04x\n", little_endian_read_16(packet, 0));
        log_info("update_interval = %d\n", little_endian_read_16(ext_param, 6 + 0));
        log_info("update_latency = %d\n",  little_endian_read_16(ext_param, 6 + 2));
        log_info("update_timeout = %d\n",  little_endian_read_16(ext_param, 6 + 4));

        if (ble_at_client_cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) {
            ble_at_client_cur_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
            ble_at_client_cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
            ble_at_client_cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
        }

#if CLIENT_PAIR_BOND_ENABLE
        if (ble_at_client_cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) {
            if (ble_at_client_cur_conn_info.pair_flag) {
                log_info("update_cur_conn\n");
                ble_at_client_pair_vm_do(&ble_at_client_cur_conn_info, 1);
            }
        } else {
            struct ctl_pair_info_t tmp_conn_info;
            for (int i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (ble_at_client_record_bond_info[i].pair_flag && ble_at_client_record_bond_info[i].conn_handle == little_endian_read_16(packet, 0)) {
                    log_info("update_record_conn\n");
                    memcpy(&tmp_conn_info, &ble_at_client_record_bond_info[i], sizeof(struct ctl_pair_info_t));
                    tmp_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
                    tmp_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
                    tmp_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
                    ble_at_client_pair_vm_do(&tmp_conn_info, 1);
                    break;
                }
            }
        }
#endif
        break;

    case GATT_COMM_EVENT_SCAN_DEV_MATCH: {
        log_info("match_dev:addr_type= %d\n", packet[0]);
        log_info_hexdump(&packet[1], 6);
        if (packet[8] == 2) {
            log_info("is TEST_BOX\n");
            break;
        }
        client_match_cfg_t *match_cfg = (void *)ext_param;
        if (match_cfg) {
            log_info("match_mode: %d\n", match_cfg->create_conn_mode);
            if (match_cfg->compare_data_len) {
                log_info_hexdump(match_cfg->compare_data, match_cfg->compare_data_len);
            }
        }

        //update info
        ble_at_client_cur_conn_info.conn_handle = 0;
        ble_at_client_cur_conn_info.pair_flag = 0;
        ble_at_client_cur_conn_info.match_dev_id = packet[9];

#if CLIENT_PAIR_BOND_ENABLE
        if (packet[9] < SUPPORT_MAX_GATT_CLIENT) {
            /*记录表回连，使用记录的连接参数建立*/
            log_info("match bond,reconnect\n");
            ble_at_client_conn_config_set(&ble_at_client_record_bond_info[packet[9]]);
            if (!ble_at_client_search_profile) {
                ble_at_client_bond_config.search_uuid_count = 0;//set no search
            }
        } else {
            /*记录表回连，使用记录的连接参数建立*/
            log_info("match config\n");
            ble_at_client_conn_config_set(NULL);
        }
#endif

    }
    break;

    case GATT_COMM_EVENT_GATT_SEARCH_MATCH_UUID: {
        opt_handle_t *opt_hdl = (void *)packet;
        log_info("match:server_uuid= %04x,charactc_uuid= %04x,value_handle= %04x\n", \
                 opt_hdl->search_uuid->services_uuid16, opt_hdl->search_uuid->characteristic_uuid16, opt_hdl->value_handle);
#if AT_CLIENT_WRITE_SEND_DATA
        //for test
        if (opt_hdl->search_uuid->characteristic_uuid16 == AT_CLIENT_WRITE_UUID) {
            ble_at_client_write_handle = opt_hdl->value_handle;
        }
#endif
    }
    break;

    case GATT_COMM_EVENT_SCAN_ADV_REPORT:
        // 如果ble_at_client_match_table内容为空会走这个分支回调，直接向串口输出广播数据
        /* log_info(">>>> scan adv report"); */
        adv_report_t *adv_report = (adv_report_t *)packet;
#if AT_CHAR_SCAN_SNED_ENABLE
        ble_at_client_parse_adv_at_send(adv_report->address, adv_report->length, adv_report->data, adv_report->rssi, adv_report->address_type);
#endif
        break;

    case GATT_COMM_EVENT_MTU_EXCHANGE_COMPLETE:
        log_info("con_handle= %02x, ATT MTU = %u\n", little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
        break;

    case GATT_COMM_EVENT_GATT_SEARCH_PROFILE_COMPLETE:
        at_send_string("OK");
        at_send_connected(ble_at_client_cur_dev_cid);
#if CLIENT_PAIR_BOND_ENABLE
        if (!ble_at_client_search_profile) {
            ble_at_client_bond_config.search_uuid_count = (sizeof(ble_at_client_search_uuid_table) / sizeof(target_uuid_t));//recover
        }
#endif
        break;

    case GATT_COMM_EVENT_CLIENT_STATE:
        log_info("client_state: handle=%02x,%02x\n", little_endian_read_16(packet, 1), packet[0]);
        break;

    default:
        break;
    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief   scan参数设置
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ble_at_client_conn_config_set(struct ctl_pair_info_t *pair_info)
{
    ble_at_client_scan_cfg.scan_auto_do = 1;
    ble_at_client_scan_cfg.creat_auto_do = 1;
    ble_at_client_scan_cfg.scan_type = SET_SCAN_TYPE;
    ble_at_client_scan_cfg.scan_filter = 1;
    ble_at_client_scan_cfg.scan_interval = SET_SCAN_INTERVAL;
    ble_at_client_scan_cfg.scan_window = SET_SCAN_WINDOW;

    if (pair_info) {
        log_info("pair_to_creat:%d,%d,%d\n", pair_info->conn_interval, pair_info->conn_latency, pair_info->conn_timeout);
        ble_at_client_scan_cfg.creat_conn_interval = pair_info->conn_interval;
        ble_at_client_scan_cfg.creat_conn_latency = pair_info->conn_latency;
        ble_at_client_scan_cfg.creat_conn_super_timeout = pair_info->conn_timeout;
    } else {
        ble_at_client_scan_cfg.creat_conn_interval = ble_at_client_conn_param.interval_min;
        ble_at_client_scan_cfg.creat_conn_latency = ble_at_client_conn_param.latency;
        ble_at_client_scan_cfg.creat_conn_super_timeout = ble_at_client_conn_param.timeout;
    }

    ble_at_client_scan_cfg.conn_update_accept = 1;
    ble_at_client_scan_cfg.creat_state_timeout_ms = SET_CREAT_CONN_TIMEOUT;

    ble_gatt_client_set_scan_config(&ble_at_client_scan_cfg);
}

/*************************************************************************************************/
/*!
 *  \brief   at client 初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/

void ble_at_client_init(void)
{
    log_info("%s", __FUNCTION__);
    int i;

#if CLIENT_PAIR_BOND_ENABLE
    if (!ble_at_client_bond_device_table) {
        int table_size = sizeof(ble_at_client_match_table) + sizeof(client_match_cfg_t) * SUPPORT_MAX_GATT_CLIENT;
        ble_at_client_bond_device_cnt = ble_at_client_search_config.match_devices_count + SUPPORT_MAX_GATT_CLIENT;
        ble_at_client_bond_device_table = my_malloc(table_size, 0);
        ASSERT(ble_at_client_bond_device_table != NULL, "%s,malloc fail!", __func__);
        memset(ble_at_client_bond_device_table, 0, table_size);
    }

    if (0 == ble_at_client_pair_vm_do(NULL, 0)) {
        log_info("client already bond dev");
    }

    if (ble_at_client_pair_redo) {
        sm_set_master_pair_redo(ble_at_client_pair_redo);
    }

#else
    ble_gatt_client_set_search_config(&ble_at_client_search_config);
#endif

    ble_at_client_conn_config_set(NULL);
}

static bool ble_at_client_uuid_elements_equal(uint16_t services_uuid16, uint16_t characteristic_uuid16, uint8_t opt_type, target_uuid_t *element)
{
    return (element->services_uuid16 == services_uuid16 &&
            element->characteristic_uuid16 == characteristic_uuid16 &&
            element->opt_type == opt_type);
}

bool ble_at_client_set_target_uuid16(uint16_t services_uuid16, uint16_t characteristic_uuid16, uint8_t opt_type)
{
    log_info(">>>>>>add target uuid:services_uuid16:%04x, characteristic_uuid16:%04x, opt_type:%04x", services_uuid16, characteristic_uuid16, opt_type);
    int i;
    for (i = 0; i < MAX_UUID_MATCH_NUM; i++) {
        if (ble_at_client_search_uuid_table[i].services_uuid16 == 0 &&
            ble_at_client_search_uuid_table[i].characteristic_uuid16 == 0 &&
            ble_at_client_search_uuid_table[i].opt_type == 0) {
            break; // 找到空位置
        }
        if (ble_at_client_uuid_elements_equal(services_uuid16, characteristic_uuid16, opt_type, &ble_at_client_search_uuid_table[i])) {
            return false; // 元素已存在
        }
    }
    if (i < MAX_UUID_MATCH_NUM) {
        ble_at_client_search_uuid_table[i].services_uuid16 = services_uuid16;
        ble_at_client_search_uuid_table[i].characteristic_uuid16 = characteristic_uuid16;
        ble_at_client_search_uuid_table[i].opt_type = opt_type;
        ble_at_client_scan_enable(0);
        ble_gatt_client_set_search_config(&ble_at_client_search_config);
        ble_at_client_scan_enable(1);
        return true; // 成功添加
    }
    return false; // 数组已满
}


static int ble_at_client_create_connection_cannel(void)
{
    if (ble_gatt_client_get_work_state() == BLE_ST_CREATE_CONN) {
        return ble_op_create_connection_cancel();
    } else {
        return 1;
    }
}

int ble_at_client_creat_cannel(void)
{
    return ble_at_client_create_connection_cannel();
}

int ble_at_client_scan_enable(uint8_t enable)
{
    log_info(">>>> scan enable");
    ble_gatt_client_module_enable(enable);
    return 0;
}

int ble_at_client_disconnect(uint8_t id)
{
    log_info(">>> ble at client disconnect");
    return ble_comm_disconnect(ble_at_client_cur_conn_info.conn_handle);
}

int ble_at_client_get_conn_param(uint16_t *conn_param)
{
    log_info(">>> ble at client get conn param");
    conn_param[0] = ble_at_client_conn_param.interval_min; //(unit:0.625ms)
    conn_param[1] = ble_at_client_conn_param.interval_max; //(unit:0.625ms)
    conn_param[2] = ble_at_client_conn_param.latency;      //(unit: interval)
    conn_param[3] = ble_at_client_conn_param.timeout;      //(unit:10ms)
    return 0;
}

int ble_at_client_set_conn_param(uint16_t *conn_param)
{
    log_info(">>> ble at client set conn param");
    ble_at_client_conn_param.interval_min = conn_param[0];  //(unit:0.625ms)
    ble_at_client_conn_param.interval_max = conn_param[1];  //(unit:0.625ms)
    ble_at_client_conn_param.latency      = conn_param[2];      //(unit: interval)
    ble_at_client_conn_param.timeout      = conn_param[3];      //(unit:10ms)
    // 更新连接配置，下次开关广播生效
    ble_at_client_conn_config_set(NULL);
    return 0;
}

int ble_at_client_creat_connection(uint8_t *conn_addr, uint8_t addr_type)
{
    log_info(">>> ble at client creat conn");
    return ble_gatt_client_create_connection_request(conn_addr, addr_type, 0);
}

void ble_at_client_send_data(uint8_t cid, uint8_t *packet, uint16_t size)
{
    log_info(">>> ble at client send data");
    ble_at_client_write_data(packet, size);
}

void ble_test_auto_scan(uint8_t en)
{
    log_info("ble_test_auto_scan = %d", en);
    ble_at_client_scan_enable(en);
}

/*************************************************************************************************/
/*!
 *  \brief  client exit
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void ble_at_client_exit(void)
{
    if (!ble_at_client_bond_device_table) {
        my_free(ble_at_client_bond_device_table);
        ble_at_client_bond_device_table = NULL;
    }
}
#endif


