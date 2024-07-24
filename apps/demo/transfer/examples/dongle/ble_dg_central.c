/*********************************************************************************************
    *   Filename        : ble_dg_central.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  :

    *   Copyright:(c)JIELI  2023-2035  @ , All Rights Reserved.
*********************************************************************************************/
#include "vm.h"
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "user_cfg.h"
#include "btcontroller_modules.h"
#include "le_common.h"
#include "bt_include/btstack_event.h"
#include "le_gatt_common.h"
#include "usb/usb_phy.h"
#include "usb/usb_std_class_def.h"
#include "sys_timer.h"
#include "ble_dg_central.h"
#include "app_dongle.h"
#include "app_power_mg.h"
#include "usb_suspend_resume.h"
#include "my_malloc.h"
#include "msg.h"
#if RCSP_BTMATE_EN
#include "ota_dg_central.h"
#endif

#if (CONFIG_APP_DONGLE)
#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[BLE_DG]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#if RCSP_BTMATE_EN
#define RCSP_RX_HANDLER_TAIL_TAG         0xEF
#define RCSP_RX_HANDLER_HEAD_TAG         0xFEDCBA
#define RCSP_RX_DEVICE_AUTH_TAG1         0x00
#define RCSP_RX_DEVICE_AUTH_TAG2         0x01
#define RCSP_RX_DEVICE_AUTH_TAG3         0x02
static uint8_t dg_central_buf_total[128];//RCSP透传接收buffer
#endif

//输入passkey 加密
#define PASSKEY_ENABLE                     0

static uint8_t dg_central_bt_connected = DG_NOT_CONNECT;
static uint8_t dg_central_ota_is_support = 0;//是否支持ota升级
static uint8_t dg_central_wait_usb_wakeup = 1;//0:发送空包  1:正常发送数据
static uint8_t dg_central_usb_send_packet[HID_MOUSE_EP_IN_MAX_SIZE];/*usb发送缓存*/

static uint16_t dg_central_timer_id;
static uint16_t dg_central_write_handle;
static uint16_t dg_central_read_handle;
static uint16_t dg_central_conn_handle, dg_central_conn_handle2;
uint8_t err_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* 预设知道连接hid设备 nofify发送的handle,通过handle分发数据上报到usb端 */
static const uint16_t dg_central_ota_notify_handle[2] = {0x0084, 0x0082};
static const uint16_t dg_central_mouse_notify_handle[3] = {0x0027, 0x002b, 0x002f};
static const uint16_t dg_central_standard_keyboard_notify_handle[2] = {0x0036, 0x003a};
//带绑定的设备搜索
static uint16_t           dg_central_bond_device_table_cnt;
static client_match_cfg_t *dg_central_bond_device_table;/*配对表(匹配地址) + 默认匹配表dg_central_match_device_table*/

static scan_conn_cfg_t dg_central_scan_cfg;
static struct ctl_pair_info_t dg_central_cur_conn_info;
static struct ctl_pair_info_t dg_central_record_bond_info[SUPPORT_MAX_GATT_CLIENT] = {
    [0 ...(SUPPORT_MAX_GATT_CLIENT - 1)] = {
        .write_handle = 0,
    },
};


//------------------------------------------------------
static void dg_central_conn_config_set(struct ctl_pair_info_t *pair_info);
static int  dg_central_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param);
static int (*dg_central_input_handler)(uint8_t *packet, uint16_t size);
//------------------------------------------------------

static const sm_cfg_t dg_central_sm_init_config = {
    .master_security_auto_req = 1,
    .master_set_wait_security = 1,

#if PASSKEY_ENABLE
    .io_capabilities = IO_CAPABILITY_KEYBOARD_ONLY,
#else
    .io_capabilities = IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
#endif

    .authentication_req_flags = SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION,
    .min_key_size = 7,
    .max_key_size = 16,
    .sm_cb_packet_handler = NULL,
};

static const gatt_client_cfg_t dg_central_init_cfg = {
    .event_packet_handler = dg_central_event_packet_handler,
};

static gatt_ctrl_t dg_central_gatt_control_block = {
    //public
    .mtu_size = ATT_LOCAL_MTU_SIZE,
    .cbuffer_size = ATT_SEND_CBUF_SIZE,
    .multi_dev_flag	= 0,

    //config
    .server_config = NULL,

#if CONFIG_BT_GATT_CLIENT_NUM
    .client_config = &dg_central_init_cfg,
#else
    .client_config = NULL,
#endif

#if CONFIG_BT_SM_SUPPORT_ENABLE
    .sm_config = &dg_central_sm_init_config,
#else
    .sm_config = NULL,
#endif
    //cbk,event handle
    .hci_cb_packet_handler = NULL,
};


//---------------------------------------------------------------------------
//指定搜索uuid
static const target_uuid_t  dg_central_search_uuid_table[] = {
    {
        .services_uuid16 = 0x1800,
        .characteristic_uuid16 = 0x2a00,
        .opt_type = ATT_PROPERTY_READ,
    },

    {
        .services_uuid16 = 0x180a,
        .characteristic_uuid16 = 0x2a50,
        .opt_type = ATT_PROPERTY_READ,
    },

    /* { */
    /* .services_uuid16 = 0x1812, */
    /* .characteristic_uuid16 = 0x2a4b, */
    /* .opt_type = ATT_PROPERTY_READ, */
    /* }, */

    {
        .services_uuid16 = 0x1812,
        .characteristic_uuid16 = 0x2a4b,
        .opt_type = ATT_PROPERTY_READ,
        .read_long_enable = 1,
    },

    {
        .services_uuid16 = 0x1812,
        .characteristic_uuid16 = 0x2a4d,
        .opt_type = ATT_PROPERTY_NOTIFY,
        .read_report_reference = 1,
    },

#if RCSP_BTMATE_EN //确保远端设备是否支持ota
    {
        .services_uuid16 = 0xae00,
        .characteristic_uuid16 = 0xae01,
        .opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    },

    {
        .services_uuid16 = 0xae00,
        .characteristic_uuid16 = 0xae02,
        .opt_type = ATT_PROPERTY_NOTIFY,
    },
#endif

    /* { */
    /* .services_uuid16 = 0x1812, */
    /* .characteristic_uuid16 = 0x2a33, */
    /* .opt_type = ATT_PROPERTY_NOTIFY, */
    /* }, */

    /* { */
    /* .services_uuid16 = 0x1801, */
    /* .characteristic_uuid16 = 0x2a05, */
    /* .opt_type = ATT_PROPERTY_INDICATE, */
    /* }, */

};

//------!!!!注意:如果是OTA升级的话,这里名字修改还需要去ota_dg_central.c修改dg_ana_remoter_name1/2
static const uint8_t dg_central_test_remoter_name1[] = "Lenovo Go Multi-Device Mouse";//键盘
/* static const uint8_t dg_central_test_remoter_name2[] = "AC897N_MX(BLE)";//鼠标 */
static const uint8_t dg_central_test_remoter_name2[] = "AW31N_MOUSE_DUAL(BLE)";//鼠标
static const client_match_cfg_t dg_central_match_device_table[] = {
    {
        .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
        .compare_data_len = sizeof(dg_central_test_remoter_name1) - 1, //去结束符
        .compare_data = dg_central_test_remoter_name1,
        .filter_pdu_bitmap = 0,
    },
    {
        .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
        .compare_data_len = sizeof(dg_central_test_remoter_name2) - 1, //去结束符
        .compare_data = dg_central_test_remoter_name2,
        .filter_pdu_bitmap = 0,
    }
};


static const gatt_search_cfg_t dg_central_search_config = {
    .match_devices = dg_central_match_device_table,
    .match_devices_count = (sizeof(dg_central_match_device_table) / sizeof(client_match_cfg_t)),
    .match_rssi_enable = 0,
    .match_rssi_value = MATCH_DEVICE_RSSI_LEVEL,

    .search_uuid_count = (sizeof(dg_central_search_uuid_table) / sizeof(target_uuid_t)),
    .search_uuid_group = dg_central_search_uuid_table,
    .auto_enable_ccc = 1,

};

//配置扫描匹配连接绑定后的设备
static gatt_search_cfg_t dg_central_bond_config = {
    .match_devices = NULL,
    .match_devices_count = 0,
    .match_rssi_enable = 0,

    .search_uuid_group = dg_central_search_uuid_table,
    .search_uuid_count = (sizeof(dg_central_search_uuid_table) / sizeof(target_uuid_t)),
    .auto_enable_ccc = 1,
};

//---------------------------------------------------------------------------------------------------------
/*更新连接设备的匹配配置*/
static void dg_central_reflash_bond_search_config(void)
{
    int i;

    if (!dg_central_bond_device_table) {
        log_info("device tabl is null!");
        return;
    }

    /*建立对应关系 配对表(匹配地址) + 默认匹配表 dg_central_match_device_table*/
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        dg_central_bond_device_table[i].create_conn_mode = BIT(CLI_CREAT_BY_ADDRESS);
        dg_central_bond_device_table[i].compare_data_len = 6;
        dg_central_bond_device_table[i].compare_data = &dg_central_record_bond_info[i].peer_address_info[1];

        if (PAIR_BOND_TAG == dg_central_record_bond_info[i].head_tag) {
            r_printf("set bond search: %d\n", i);
            dg_central_bond_device_table[i].filter_pdu_bitmap = BIT(EVENT_ADV_SCAN_IND) | BIT(EVENT_ADV_NONCONN_IND);
        } else {
#if RCSP_BTMATE_EN
            dg_central_bond_device_table[i].filter_pdu_bitmap = 0;
#else
            dg_central_bond_device_table[i].filter_pdu_bitmap = EVENT_DEFAULT_REPORT_BITMAP;
#endif
        }
    }
    memcpy(&dg_central_bond_device_table[SUPPORT_MAX_GATT_CLIENT], dg_central_match_device_table, sizeof(dg_central_match_device_table));

    dg_central_bond_config.match_devices = dg_central_bond_device_table;
    dg_central_bond_config.match_devices_count = dg_central_bond_device_table_cnt;
    ble_gatt_client_set_search_config(&dg_central_bond_config);
}

static void dg_central_timer_handle(void *priv)
{
    putchar('%');
    static uint8_t connected_tag = 0;//上电连接配对过标识

    if (dg_central_bt_connected != DG_NOT_CONNECT) {
        if (dg_central_bt_connected == DG_SEARCH_PROFILE_COMPLETE) {
            connected_tag = 1;
        }
        return;
    }

    if ((int)priv == 0) {
        ble_gatt_client_scan_enable(0);
        ///init
        if (dg_central_record_bond_info[0].head_tag == PAIR_BOND_TAG && \
            0 == ble_gatt_client_create_connection_request(&dg_central_record_bond_info[0].peer_address_info[1], dg_central_record_bond_info[0].peer_address_info[0], 0)) {
            log_info("pair is exist");
            log_info("reconnect start0");
        } else {
            log_info("pair new start0");
            ble_gatt_client_scan_enable(1);
        }
    } else {

        if (connected_tag) {
            //上电连接配对过，就不执行搜索配对;默认创建连接
            putchar('^');
#if !RCSP_BTMATE_EN
            if (dg_central_bt_connected == DG_NOT_CONNECT && ble_gatt_client_get_work_state() != BLE_ST_CREATE_CONN) {
                dg_central_conn_config_set(&dg_central_record_bond_info[0]);/*record's config*/
                if (ble_gatt_client_create_connection_request(&dg_central_record_bond_info[0].peer_address_info[1], dg_central_record_bond_info[0].peer_address_info[0], 0)) {
                    log_info("recreate_conn fail!!!");
                }
            }
#endif
            return;
        }
        switch (ble_gatt_client_get_work_state()) {
        case BLE_ST_CREATE_CONN:
            ble_gatt_client_create_connection_cannel();
            dg_central_conn_config_set(NULL);/*default config*/
            ble_gatt_client_scan_enable(1);
            log_info("pair new start1");
            break;

        case BLE_ST_SCAN:
            if (dg_central_record_bond_info[0].head_tag == PAIR_BOND_TAG) {
                ble_gatt_client_scan_enable(0);
                dg_central_conn_config_set(&dg_central_record_bond_info[0]);/*record's config*/
                if (0 == ble_gatt_client_create_connection_request(&dg_central_record_bond_info[0].peer_address_info[1], dg_central_record_bond_info[0].peer_address_info[0], 0)) {
                    log_info("reconnect start1");
                } else {
                    log_info("keep pair new start1");
                    dg_central_conn_config_set(NULL);/*default config*/
                    ble_gatt_client_scan_enable(1);
                }
            }
            break;

        case BLE_ST_INIT_OK:
        case BLE_ST_IDLE:
        case BLE_ST_DISCONN:
            ble_gatt_client_scan_enable(1);
            log_info("pair new start000");
            break;

        default:
            break;
        }
    }
}

static void dg_central_enable_notify_ccc(uint16_t conn_handle)
{
    log_info("%s\n", __FUNCTION__);
    /* ble_comm_att_send_data(conn_handle, dg_central_mouse_notify_handle[0] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE); */
    /* ble_comm_att_send_data(conn_handle, dg_central_mouse_notify_handle[1] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE); */
    /* ble_comm_att_send_data(conn_handle, dg_central_mouse_notify_handle[2] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE); */
    /* ble_comm_att_send_data(conn_handle, dg_central_standard_keyboard_notify_handle[0] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE); */
    /* ble_comm_att_send_data(conn_handle, dg_central_standard_keyboard_notify_handle[1] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE); */
#if RCSP_BTMATE_EN
    uint16_t mouse_ccc_value = 0x0001;
    ble_comm_att_send_data(conn_handle, dg_central_ota_notify_handle[0] + 1, &mouse_ccc_value, 2, ATT_OP_WRITE);
#endif
}


static struct ctl_pair_info_t *dg_central_get_pair_info(uint16_t conn_handle)
{
    if (!conn_handle) {
        return NULL;
    }

    for (int i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        if (dg_central_record_bond_info[i].conn_handle == conn_handle) {
            return &dg_central_record_bond_info[i];
        }
    }
    return NULL;
}

static void dg_central_disconnect_all(void)
{
    log_info("disconnect all connection!");
    for (int i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        if (dg_central_record_bond_info[i].conn_handle) {
            ble_comm_disconnect(dg_central_record_bond_info[i].conn_handle);

        }
    }
}


static int dg_central_pair_vm_do(struct ctl_pair_info_t *info, uint8_t rw_flag)
{
    int ret = 0;

#if CLIENT_PAIR_BOND_ENABLE
    int i;
    int uint_len = sizeof(struct ctl_pair_info_t);

    log_info("-conn_pair_info vm_do:%d\n", rw_flag);

    if (rw_flag == 0) {
        ret = syscfg_read(CFG_BLE_BONDING_REMOTE_INFO2, (uint8_t *)&dg_central_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
        if (!ret) {
            log_info("-null--\n");
            memset((uint8_t *)&dg_central_record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
            ret = 1;
        }

        /*检查合法性*/
        for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
            if (PAIR_BOND_TAG != dg_central_record_bond_info[i].head_tag || dg_central_record_bond_info[i].pair_flag != 1) {
                memset((uint8_t *)&dg_central_record_bond_info[i], 0xff, uint_len);
            }
        }

    } else {
        int fill_index = -1;

        if (info == NULL) {
            log_info("vm clear\n");
            memset((uint8_t *)&dg_central_record_bond_info, 0xff, uint_len * SUPPORT_MAX_GATT_CLIENT);
        } else {

            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (0 == memcmp(info, &dg_central_record_bond_info[i], uint_len)) {
                    log_info("dev in table\n");
                    return ret;
                }
            }

            log_info_hexdump((uint8_t *)&dg_central_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
            log_info_hexdump((uint8_t *)info, uint_len);
            log_info("write vm start\n");
            log_info("find table\n");

            /*find same*/
            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (0 == memcmp(info->peer_address_info, &dg_central_record_bond_info[i].peer_address_info, 7)) {
                    log_info("replace old,match_dev_id: %d,%d\n", info->match_dev_id, dg_central_record_bond_info[i].match_dev_id);
                    fill_index = i;
                    if (info->match_dev_id < SUPPORT_MAX_GATT_CLIENT) {
                        /*地址回连方式,获取原来的search id*/
                        info->match_dev_id = dg_central_record_bond_info[i].match_dev_id;
                        /*地址回连方式,获取原来的write_handle*/
                        info->write_handle = dg_central_record_bond_info[i].write_handle;
                    } else if ((info->match_dev_id >= SUPPORT_MAX_GATT_CLIENT) && (info->match_dev_id != dg_central_record_bond_info[i].match_dev_id)) {
                        /*遇到连接多设备match_dev_id为别的通道的值时,不更改*/
                        info->match_dev_id = dg_central_record_bond_info[i].match_dev_id;
                    }
                    break;
                }
            }

            /*find idle*/
            if (fill_index == -1) {
                for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    if (PAIR_BOND_TAG != dg_central_record_bond_info[i].head_tag) {
                        log_info("find idle\n");
                        fill_index = i;
                        break;
                    }
                }
            }

            /*find first*/
            if (fill_index == -1) {
                for (i = 1; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                    memcpy(&dg_central_record_bond_info[i - 1], &dg_central_record_bond_info[i], uint_len);
                }
                log_info("replace first\n");
                fill_index = SUPPORT_MAX_GATT_CLIENT - 1;
            }

            /*连接顺序不同，handle是不一样,防止重复相同*/
            for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (info->conn_handle == dg_central_record_bond_info[i].conn_handle) {
                    dg_central_record_bond_info[i].conn_handle = 0;
                    log_info("clear repeat handle %d\n", info->conn_handle);
                }
            }

            memcpy(&dg_central_record_bond_info[fill_index], info, uint_len);
            log_info("new record,match_dev_id= %d\n", info->match_dev_id);

        }

        syscfg_write(CFG_BLE_BONDING_REMOTE_INFO2, (uint8_t *)&dg_central_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    }

    log_info_hexdump((uint8_t *)&dg_central_record_bond_info, uint_len * SUPPORT_MAX_GATT_CLIENT);
    dg_central_reflash_bond_search_config();/*配对表发生变化，更新scan设备匹配*/
#endif
    return ret;
}

//清配对信息
int dg_central_clear_pair(void)
{
#if CLIENT_PAIR_BOND_ENABLE
    ble_gatt_client_disconnect_all();
    memset(&dg_central_cur_conn_info, 0, sizeof(dg_central_cur_conn_info));
    dg_central_pair_vm_do(NULL, 1);
    if (BLE_ST_SCAN == ble_gatt_client_get_work_state()) {
        ble_gatt_client_scan_enable(0);
        ble_gatt_client_scan_enable(1);
    }
#endif
    return 0;
}

//dongle——hid 数据发送接口
static int dg_central_hid_send_data(uint8_t *data, uint16_t len)
{
    //对应ble_hogp_profile.h中的input最后一位att_handle
    dg_central_read_handle = 0x003e;

    if (dg_central_conn_handle && dg_central_read_handle) {
        return ble_comm_att_send_data(dg_central_conn_handle, dg_central_read_handle, data, len, ATT_OP_WRITE);
    }
    return APP_BLE_OPERATION_ERROR;
}



uint8_t dg_central_get_match_id(uint16_t conn_handle)
{
    struct ctl_pair_info_t *dg_handle_info =  dg_central_get_pair_info(conn_handle);
    if (dg_handle_info) {
        return dg_handle_info->match_dev_id;
    }
    return 0;
}

uint8_t dg_central_get_ota_is_support(uint16_t conn_handle)
{
    log_info("dg_central_ota_is_support :%d", dg_central_ota_is_support);
    return dg_central_ota_is_support;
}

uint8_t dg_central_is_succ_connection(void)
{
    log_info("is_support :%d", dg_central_bt_connected);
    return dg_central_bt_connected;
}

uint8_t *dg_central_get_conn_address(uint16_t conn_handle)
{
    struct ctl_pair_info_t *dg_handle_info =  dg_central_get_pair_info(conn_handle);
    /* if (conn_handle == dg_central_cur_conn_info.conn_handle) { //避免连接上过早的拿地址导致地址出错 */
    /*     log_info("peer address is now:"); */
    /*     return dg_central_cur_conn_info.peer_address_info; */
    /* } else { */
    if (dg_handle_info) {
        log_info("peer address:");
        /* log_info_hexdump(dg_handle_info->peer_address_info, 7); */
        return dg_handle_info->peer_address_info;
    } else {
        log_info("err address:");
        return err_address;
    }
}

/*断开后,清PC按键;根据设备描述符的长度,清0*/
static void dg_central_disable_clear_key(uint16_t conn_handle)
{
    uint8_t match_id = dg_central_get_match_id(conn_handle);
    uint8_t tmp_buf[16];
    memset(tmp_buf, 0, 16);
    if (match_id == NAME1_DEV_ID) {
        log_info("clear dev_id1 key\n");
        tmp_buf[0] = 1;
        dongle_ble_hid_input_handler(tmp_buf, 10);
        tmp_buf[0] = 2;
        dongle_ble_hid_input_handler(tmp_buf, 10);

    } else {
        log_info("clear dev_id2 key\n");
        tmp_buf[0] = 4;
        dongle_second_ble_hid_input_handler(tmp_buf, 9);
        tmp_buf[0] = 5;
        dongle_second_ble_hid_input_handler(tmp_buf, 9);
    }
}

static void dg_central_state_to_user(uint8_t state, uint8_t reason)
{
    static uint8_t ble_state = 0xff;
    uint8_t en = 0;
    if (state != ble_state) {
        log_info("ble_state:%02x===>%02x\n", ble_state, state);
        ble_state = state;
        en = 1;
    } else {
        log_info("Same as the previous state!!\n");
    }
    if (en) {
        struct sys_event *e = event_pool_alloc();
        if (e == NULL) {
            log_info("Memory allocation failed for sys_event");
            return;
        }
        e->type = SYS_BT_EVENT;
        e->arg  = (void *)SYS_BT_EVENT_BLE_STATUS;
        e->u.bt.event = state;
        e->u.bt.value = reason;
        main_application_operation_event(NULL, e);
    }
}

#if CONFIG_BLE_CONNECT_SLOT
/*************************************************************************************************/
/*!
 *  \brief      1. 基带接口接收数据接口,和app_mouse绑定使用
                2. 底层有事件触发后回调直接来上层拿数据发送,函数名不可修改，无需调用;
                3. 不可做耗时操作（需要在us级）;
                4. 数据接收不会再走dg_central_event_packet_handler-GATT_COMM_EVENT_GATT_DATA_REPORT
 *
 *  \param      [in] NUll
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
bool ll_conn_rx_acl_hook_get(const uint8_t *const data, size_t len)
{
    uint8_t hid_send_packet[MOUSE_USB_PACKET_LEN];
    //demo code
    uint16_t conn_handle = little_endian_read_16(data, 0) & 0x0fff;
    uint16_t l2cap_chl_id = little_endian_read_16(data, 6);
    //check pdu_start,conn_handle,channel_id
    if ((data[1] & 0x20) && (conn_handle >= 0x50 && conn_handle < (0x50 + CONFIG_BT_GATT_CLIENT_NUM)) && l2cap_chl_id == 4) {
        uint16_t att_handle = little_endian_read_16(data, 9);
        if (att_handle != 0x27) {
            putchar('e');
            return false;
        }
        // mouse report id
        hid_send_packet[0] = MOUSE_REPORT_ID;
        // mouse report start data
        const uint8_t *priv = &data[11];

        uint8_t usb_status_ret = usb_slave_status_get();
        if (usb_status_ret == USB_SLAVE_RESUME) {
            wdt_clear();
            if (dg_central_wait_usb_wakeup) {
                memcpy(&hid_send_packet[1], priv, MOUSE_USB_PACKET_LEN - 1);
                /* log_info_hexdump(hid_send_packet, MOUSE_USB_PACKET_LEN); */
            } else {
                log_info("clear length: %d", MOUSE_USB_PACKET_LEN - 1);
                memset(&hid_send_packet[1], 0, MOUSE_USB_PACKET_LEN - 1); //发空包
            }
        } else if (usb_status_ret == USB_SLAVE_SUSPEND) {
            dg_central_wait_usb_wakeup = 0;
            log_info("send remote_wakeup\n");
            usb_remote_wakeup(0);

            return true;
        } else {
            ;
        }

        int ret = dongle_ble_hid_input_handler(hid_send_packet, MOUSE_USB_PACKET_LEN);

        if (ret && dg_central_wait_usb_wakeup == 0) {
            dg_central_wait_usb_wakeup = 1;
            log_info("send 0packet success!\n");
        }

        return true;
    }
    putchar('E');
    return false;
}

#endif

/*************************************************************************************************/
/*!
 *  \brief      USB状态变更回调
 *  \param      [in] NUll
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void dg_central_usb_status_handler(const usb_dev usb_id, usb_slave_status status)
{
    /* PC进入睡眠模式，usb suspend，断开所有连接，只保留scan */
    if (status == USB_SLAVE_SUSPEND && dg_central_wait_usb_wakeup) {
        dg_central_disconnect_all();
    }
}

static int dg_central_event_packet_handler(int event, uint8_t *packet, uint16_t size, uint8_t *ext_param)
{
    att_data_report_t *report_data = (void *)packet;
    static uint16_t mtu_size = 0;

    /* log_info("event: %02x,size= %d\n", event, size); */
    switch (event) {
    case GATT_COMM_EVENT_GATT_DATA_REPORT: {
        /* log_info("dg_central_event_packet_handler: %x", report_data->value_handle); */
        /* log_info("data_report:hdl=%04x,pk_type=%02x,size=%d,val_hdl=%04x\n", \ */
        /* report_data->conn_handle, report_data->packet_type, report_data->blob_length, report_data->value_handle); */
        /* log_info_hexdump(report_data->blob, report_data->blob_length); */

        /*uint8_t match_id = dg_central_get_match_id(report_data->conn_handle);*/
        /* log_info("data_match_id= %02x\n",match_id); */
        /* log_info("{%d}", match_id); */

        switch (report_data->packet_type) {
        case GATT_EVENT_NOTIFICATION: { //notify
            if (report_data->value_handle != dg_central_ota_notify_handle[0]) {
                /*预设知道连接hid设备 nofify发送的handle,通过handle分发数据上报到usb端*/
                //uint8_t packet[16];
                if (report_data->blob_length >= sizeof(dg_central_usb_send_packet)) {
                    log_info("err:usb ep over");
                    return 0;
                }
                uint8_t second_devices = 0;
                if (report_data->value_handle == dg_central_mouse_notify_handle[0]) {
                    dg_central_usb_send_packet[0] = 1; //report_id
                } else if (report_data->value_handle == dg_central_mouse_notify_handle[1]) {
                    dg_central_usb_send_packet[0] = 2;//report_id
                } else if (report_data->value_handle == dg_central_standard_keyboard_notify_handle[0]) {
                    dg_central_usb_send_packet[0] = 4;//report_id
                    second_devices = 1;
                } else if (report_data->value_handle == dg_central_standard_keyboard_notify_handle[1]) {
                    dg_central_usb_send_packet[0] = 5;//report_id
                    second_devices = 1;
                } else {
                    log_info("err notify");
                    break;
                    /* dg_central_usb_send_packet[0] = 1;//report_id */
                }

                uint8_t usb_status_ret = usb_slave_status_get();
                if (usb_status_ret == USB_SLAVE_RESUME) {
                    wdt_clear();
                    if (dg_central_wait_usb_wakeup) {
                        memcpy(&dg_central_usb_send_packet[1], report_data->blob, report_data->blob_length);
                        log_info_hexdump(dg_central_usb_send_packet, report_data->blob_length + 1);
                    } else {
                        log_info("clear length: %d", report_data->blob_length);
                        memset(&dg_central_usb_send_packet[1], 0, report_data->blob_length); //发空包
                    }
                } else if (usb_status_ret == USB_SLAVE_SUSPEND) {
                    dg_central_wait_usb_wakeup = 0;
                    log_info("send remote_wakeup\n");
                    usb_remote_wakeup(0);
                    break;
                } else {
                    ;
                }
                /* } else if (usb_status_ret == USB_RESUME_WAIT) { */
                /*     putchar('W'); */
                /*     break; */
                /*  */
                /* } else if (usb_status_ret == USB_RESUME_OK) { */
                /*     putchar('M'); */
                /* } */

                if (report_data->value_handle == dg_central_mouse_notify_handle[0]) {
                    dg_central_usb_send_packet[0] = 1; //report_id
                } else if (report_data->value_handle == dg_central_mouse_notify_handle[1]) {
                    dg_central_usb_send_packet[0] = 2;//report_id
                } else {
                    dg_central_usb_send_packet[0] = 1;//report_id
                }

                /* int (*dg_central_input_handler)(uint8_t *packet, uint16_t size); */
                if (second_devices == 0) {
                    /* dongle_ble_hid_input_handler(dg_central_usb_send_packet, report_data->blob_length + 1); */
                    /* putchar('&'); */
                    dg_central_input_handler = dongle_ble_hid_input_handler;
                } else {
                    /* dongle_second_ble_hid_input_handler(dg_central_usb_send_packet, report_data->blob_length + 1); */
                    /* putchar('&'); */
                    dg_central_input_handler = dongle_second_ble_hid_input_handler;
                }

                int ret = dg_central_input_handler(dg_central_usb_send_packet, report_data->blob_length + 1);
                if (ret && dg_central_wait_usb_wakeup == 0) {
                    dg_central_wait_usb_wakeup = 1;
                    log_info("send 0packet success!\n");
                }
            } else {
#if RCSP_BTMATE_EN
                if (!dg_central_ota_is_support) {
                    log_info("This device support OTA updata!!!");
                    dg_central_ota_is_support = 1;
                }

                /* gatt_client_get_mtu(report_data->conn_handle, &mtu_size); */
                /* mtu_size -= 3; */
                /* log_info("mut_size is %d", mtu_size); */

                if (0xff == report_data->blob[0]) {
                    putchar('E');//dorp
                    return 0;
                }

                /* server_receive = 0; */
                log_info_hexdump(report_data->blob, report_data->blob_length);
                dongle_send_data_to_pc_2(report_data->conn_handle, report_data->blob, report_data->blob_length);
#endif
            }
        }
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
        log_info("peer_address_info:");
        log_info_hexdump(&ext_param[7], 7);
        memcpy(dg_central_cur_conn_info.peer_address_info, &ext_param[7], 7);
        dg_central_cur_conn_info.conn_handle =   little_endian_read_16(packet, 0);
        dg_central_cur_conn_info.conn_interval = little_endian_read_16(ext_param, 14 + 0);
        dg_central_cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 14 + 2);
        dg_central_cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 14 + 4);
        dg_central_cur_conn_info.pair_flag = 0;
        dg_central_cur_conn_info.head_tag = 0;
        dg_central_bt_connected = DG_CONNECT_COMPLETE;

        if (!dg_central_conn_handle) {
            dg_central_conn_handle = little_endian_read_16(packet, 0);
        } else {
            dg_central_conn_handle2 = little_endian_read_16(packet, 0);
        }

#if RCSP_BTMATE_EN
        check_is_reconn_succ(1, dg_central_cur_conn_info.conn_handle);//返回重连接成功,未放到encry处,避免不走加密情况
#endif
        break;

    case GATT_COMM_EVENT_DISCONNECT_COMPLETE: {
        log_info("disconnect_handle:%04x,reason= %02x\n", little_endian_read_16(packet, 0), packet[2]);

        //TODO 可以推msg到app处理
#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif

        if (little_endian_read_16(packet, 0) == dg_central_conn_handle) {
            dg_central_conn_handle = 0;
            dg_central_bt_connected = DG_NOT_CONNECT;
        } else if (little_endian_read_16(packet, 0) == dg_central_conn_handle2) {
            dg_central_conn_handle2 = 0;
        }

        if (packet[2] == 0x08) {
            /*超时断开,清按键行为操作*/
            /* dg_central_disable_clear_key(little_endian_read_16(packet, 0)); */
        }
#if RCSP_BTMATE_EN
        if (!get_reonn_param()) {
            dongle_return_online_list();
        }
        if (reonn_channel_is_change()) {
            dongle_ota_init();
        }
        clear_auth(little_endian_read_16(packet, 0));
        ble_gatt_client_scan_enable(0);//ota升级过程中,从机主动断开连接需要等上位机下发回连接命令
        //当回连接出现偶现0x3e同步报错问题,为避免直接卡死.重新打开scan
        if (packet[2] == 0x3e) {
            ble_gatt_client_scan_enable(1);
            break;
        }

        sys_timeout_add(little_endian_read_16(packet, 0), judge_is_reonn, 1000);//判断是否是ota回连接
#endif
    }
    break;

    case GATT_COMM_EVENT_ENCRYPTION_CHANGE:
        log_info("ENCRYPTION_CHANGE:handle=%04x,state=%d,process =%d", little_endian_read_16(packet, 0), packet[2], packet[3]);
        if (packet[3] == LINK_ENCRYPTION_RECONNECT) {
            log_info("reconnect...\n");
            dg_central_enable_notify_ccc(little_endian_read_16(packet, 0));
#if (DG_PAIR_RECONNECT_SEARCH_PROFILE == 0)
            dg_central_bond_config.search_uuid_count = 0;//set no search
#endif
        } else {
            log_info("first pair...\n");
            dg_central_bond_config.search_uuid_count = (sizeof(dg_central_search_uuid_table) / sizeof(target_uuid_t));//set search profile
        }
        dg_central_state_to_user(BLE_PRIV_PAIR_ENCRYPTION_CHANGE, little_endian_read_16(packet, 0));

        if (packet[2]) {
            log_info("error:encryption fail=%02x,disconnect,%04x\n", packet[2], little_endian_read_16(packet, 0));
#if RCSP_BTMATE_EN
            //ota升级失败之后,由于是不加密连接,会触发26的加密失败,做一个不断开的操作
            if (packet[2] == 26) {
                log_info("OTA no encryption conn!!");
            } else {
                ble_comm_disconnect(little_endian_read_16(packet, 0));
                break;
            }
#else
            ble_comm_disconnect(little_endian_read_16(packet, 0));
            break;
#endif
        }

#if CLIENT_PAIR_BOND_ENABLE
#if RCSP_BTMATE_EN
        dg_central_ota_is_support = 0;
        for (uint8_t i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
            if (dg_central_record_bond_info[i].write_handle) {
                log_info("open ota :%d, ota handle:%d", i, dg_central_record_bond_info[i].write_handle);
                dg_central_cur_conn_info.write_handle = dg_central_record_bond_info[i].write_handle;
                dg_central_ota_is_support = 1;
            }
        }
#endif
        dg_central_cur_conn_info.head_tag = PAIR_BOND_TAG;
        dg_central_cur_conn_info.pair_flag = 1;
        dg_central_pair_vm_do(&dg_central_cur_conn_info, 1);
#endif
        break;

    case GATT_COMM_EVENT_CONNECTION_UPDATE_COMPLETE:
        log_info("conn_param update_complete:%04x\n", little_endian_read_16(packet, 0));
        log_info("update_interval = %d\n", little_endian_read_16(ext_param, 6 + 0));
        log_info("update_latency = %d\n",  little_endian_read_16(ext_param, 6 + 2));
        log_info("update_timeout = %d\n",  little_endian_read_16(ext_param, 6 + 4));

        if (dg_central_cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) {
            dg_central_cur_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
            dg_central_cur_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
            dg_central_cur_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
        }

#if CLIENT_PAIR_BOND_ENABLE
        if (dg_central_cur_conn_info.conn_handle == little_endian_read_16(packet, 0)) {
            if (dg_central_cur_conn_info.pair_flag) {
                log_info("update_cur_conn\n");
                dg_central_pair_vm_do(&dg_central_cur_conn_info, 1);
            }
        } else {
            struct ctl_pair_info_t tmp_conn_info;
            for (int i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
                if (dg_central_record_bond_info[i].pair_flag && dg_central_record_bond_info[i].conn_handle == little_endian_read_16(packet, 0)) {
                    log_info("update_record_conn\n");
                    memcpy(&tmp_conn_info, &dg_central_record_bond_info[i], sizeof(struct ctl_pair_info_t));
                    tmp_conn_info.conn_interval = little_endian_read_16(ext_param, 6 + 0);
                    tmp_conn_info.conn_latency =  little_endian_read_16(ext_param, 6 + 2);
                    tmp_conn_info.conn_timeout =  little_endian_read_16(ext_param, 6 + 4);
                    dg_central_pair_vm_do(&tmp_conn_info, 1);
                    break;
                }
            }
        }
#endif

        break;


    case GATT_COMM_EVENT_SCAN_DEV_MATCH: {
        log_info("match_dev:addr_type= %d,rssi= %d\n", packet[0], packet[7]);
        log_info_hexdump(&packet[1], 6);
        if (packet[8] == 2) {
            log_info("is TEST_BOX\n");
        }
        /* client_match_cfg_t *match_cfg = ext_param; */
        client_match_cfg_t match_cfg_buffer;
        memcpy(&match_cfg_buffer, ext_param, sizeof(client_match_cfg_t));
        client_match_cfg_t *match_cfg = &match_cfg_buffer;

        if (match_cfg) {
            log_info("match_mode: %d\n", match_cfg->create_conn_mode);
            if (match_cfg->compare_data_len) {
                log_info_hexdump((uint8_t *)match_cfg->compare_data, match_cfg->compare_data_len);
            }
        }
        //update info
        dg_central_cur_conn_info.conn_handle = 0;
        dg_central_cur_conn_info.pair_flag = 0;
        dg_central_cur_conn_info.match_dev_id = packet[9];
        dg_central_bond_config.search_uuid_count = (sizeof(dg_central_search_uuid_table) / sizeof(target_uuid_t));//set search profile

#if CLIENT_PAIR_BOND_ENABLE
        if (packet[9] < SUPPORT_MAX_GATT_CLIENT) {
            /*记录表地址回连，使用记录的连接参数建立*/
            r_printf("match bond,reconnect\n");
            dg_central_conn_config_set(&dg_central_record_bond_info[packet[9]]);
        } else {
            /*搜索匹配方式连接*/
            r_printf("match search_config\n");
            dg_central_conn_config_set(NULL);
        }
#endif
        log_info("search match_dev_id: %d\n", dg_central_cur_conn_info.match_dev_id);

    }
    break;

    case GATT_COMM_EVENT_CREAT_CONN_TIMEOUT:
        break;

    case GATT_COMM_EVENT_GATT_SEARCH_MATCH_UUID: {

#if RCSP_BTMATE_EN
        opt_handle_t *opt_hdl = packet;
        log_info("match:server_uuid= %04x,charactc_uuid= %04x,value_handle= %04x\n", \
                 opt_hdl->search_uuid->services_uuid16, opt_hdl->search_uuid->characteristic_uuid16, opt_hdl->value_handle);


        if (opt_hdl->value_handle == dg_central_ota_notify_handle[1]) {
            log_info("This device support OTA updata!!");
            dg_central_cur_conn_info.write_handle = opt_hdl->value_handle;

            //回连过程不走加密,做保存
            if (get_reonn_param() != 1) {
                dg_central_cur_conn_info.head_tag = PAIR_BOND_TAG;
                dg_central_cur_conn_info.pair_flag = 1;
                dg_central_pair_vm_do(&dg_central_cur_conn_info, 1);
            }
            dg_central_ota_is_support = 1;
        } else {
            /* log_info("This device No support OTA updata!!!"); */
            /* dg_central_ota_is_support = 0; */
        }
#endif
    }
    break;


    case GATT_COMM_EVENT_MTU_EXCHANGE_COMPLETE:
        log_info("con_handle= %02x, ATT MTU = %u\n", little_endian_read_16(packet, 0), little_endian_read_16(packet, 2));
        break;

    case GATT_COMM_EVENT_GATT_SEARCH_PROFILE_COMPLETE:
        log_info("GATT_COMM_EVENT_GATT_SEARCH_PROFILE_COMPLETE\n");
        dg_central_bt_connected = DG_SEARCH_PROFILE_COMPLETE;
#if RCSP_BTMATE_EN
        dongle_return_online_list();
        ble_gatt_client_scan_enable(1);//ota升级流程连接完成后继续开启scan
        if (get_reonn_param()) {
            //如果开了提速,ota回连接也申请del
            if (config_btctler_le_features & LE_DATA_PACKET_LENGTH_EXTENSION) {
                log_info(">>>>>>>>s2--request DLE, %04x\n", dg_central_cur_conn_info.conn_handle);
                ble_comm_set_connection_data_length(dg_central_cur_conn_info.conn_handle, config_btctler_le_acl_packet_length, 2120);
            }

        }
#endif

    case GATT_COMM_EVENT_CLIENT_STATE:
        log_info("client_state: %02x,%02x\n", little_endian_read_16(packet, 1), packet[0]);
        dg_central_state_to_user(packet[0], little_endian_read_16(packet, 0));
        break;

        break;

    default:
        break;
    }
    return 0;
}

//scan参数设置
static void dg_central_conn_config_set(struct ctl_pair_info_t *pair_info)
{
#if RCSP_BTMATE_EN
    dg_central_scan_cfg.scan_auto_do = 0;
#else
    dg_central_scan_cfg.scan_auto_do = CONFIG_BT_GATT_CLIENT_NUM > 0 ? 1 : 0;
#endif
    dg_central_scan_cfg.creat_auto_do = 1;
    dg_central_scan_cfg.scan_type = SET_SCAN_TYPE;
    dg_central_scan_cfg.scan_filter = 1;
    dg_central_scan_cfg.scan_interval = SET_SCAN_INTERVAL;
    dg_central_scan_cfg.scan_window = SET_SCAN_WINDOW;

    if (pair_info) {
        log_info("pair_to_creat:%d,%d,%d\n", pair_info->conn_interval, pair_info->conn_latency, pair_info->conn_timeout);
        dg_central_scan_cfg.creat_conn_interval = pair_info->conn_interval;
        dg_central_scan_cfg.creat_conn_latency = pair_info->conn_latency;
        dg_central_scan_cfg.creat_conn_super_timeout = pair_info->conn_timeout;
    } else {
        dg_central_scan_cfg.creat_conn_interval = SET_CONN_INTERVAL;
        dg_central_scan_cfg.creat_conn_latency = SET_CONN_LATENCY;
        dg_central_scan_cfg.creat_conn_super_timeout = SET_CONN_TIMEOUT;
    }

    dg_central_scan_cfg.conn_update_accept = 1;
    dg_central_scan_cfg.creat_state_timeout_ms = SET_CREAT_CONN_TIMEOUT;

    ble_gatt_client_set_scan_config(&dg_central_scan_cfg);
}

static void dg_central_init(void)
{
    log_info("%s\n", __FUNCTION__);

    if (!dg_central_bond_device_table) {
        int table_size = sizeof(dg_central_match_device_table) + sizeof(client_match_cfg_t) * SUPPORT_MAX_GATT_CLIENT;//设置名字匹配
        dg_central_bond_device_table_cnt = dg_central_search_config.match_devices_count + SUPPORT_MAX_GATT_CLIENT;
        dg_central_bond_device_table = my_malloc(table_size, 0);
        ASSERT(dg_central_bond_device_table != NULL, "%s,malloc fail!", __func__);
        memset(dg_central_bond_device_table, 0, table_size);
    }

    if (0 == dg_central_pair_vm_do(NULL, 0)) {
        log_info("client already bond dev");
    }

    dg_central_conn_config_set(NULL);
}

//-------------------------------------------------------------------------------------
void dg_central_uuid_count_set(void)
{
    dg_central_bond_config.search_uuid_count = (sizeof(dg_central_search_uuid_table) / sizeof(target_uuid_t));//set search profile
}

void bt_ble_before_start_init(void)
{
    ble_comm_init(&dg_central_gatt_control_block);
}

void bt_ble_init(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_comm_set_config_name(bt_get_local_name(), 1);
    dg_central_conn_handle = 0;
    dg_central_bt_connected = DG_NOT_CONNECT;

#if CONFIG_BLE_CONNECT_SLOT
    // 设置高回报率模式
    ble_op_conn_us_unit(1);
#endif

    dg_central_init();
    ble_module_enable(1);

    if (CONFIG_BT_GATT_CLIENT_NUM == 1) {
        log_info("connect + scan");
#if	POWER_ON_RECONNECT_START
        dg_central_timer_handle(0);
#endif
        dg_central_timer_id = sys_timeout_add((void *)1, dg_central_timer_handle, POWER_ON_SWITCH_TIME);
    } else {
        ble_gatt_client_scan_enable(1);
        log_info("just scan");
    }
}

void bt_ble_exit(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_module_enable(0);
    ble_comm_exit();

    if (dg_central_bond_device_table) {
        my_free(dg_central_bond_device_table);
        dg_central_bond_device_table = NULL;
    }
}

void ble_module_enable(uint8_t en)
{
    ble_comm_module_enable(en);
}

int app_ble_send_data_api(uint8_t *data, uint32_t len)
{
    int ret = 0;
    ret = dg_central_hid_send_data(data, len);
    return ret;
}

//usb 数据发送接口
static int ble_hid_data_send_ext(uint8_t report_type, uint8_t report_id, uint8_t *data, uint16_t len)
{
    //对应ble_hogp_profile.h中的output——att_handle
    dg_central_write_handle = 0x002f;

    if (dg_central_conn_handle && dg_central_write_handle) {
        return ble_comm_att_send_data(dg_central_conn_handle, dg_central_write_handle, data, len, ATT_OP_WRITE);
    }
    return APP_BLE_OPERATION_ERROR;
}

void ble_hid_output_handler(uint8_t report_type, uint8_t report_id, uint8_t *packet, uint16_t size)
{
    log_info("hid_data_output:type= %02x,report_id= %d,size=%d", report_type, report_id, size);
    log_info_hexdump(packet, size);

    ble_hid_data_send_ext(report_type, report_id, packet, size);
}


//ota 数据发送接口
int ble_dongle_send_data(uint16_t con_handle, uint8_t *data, uint16_t len)
{
    uint8_t i, j;
    /* log_info_hexdump(data, len); */
    /* log_info("ble_comm_dev_get_handle: %d", ble_comm_dev_get_handle(0, GATT_ROLE_CLIENT)); */
    for (i = 0; i < SUPPORT_MAX_GATT_CLIENT; i++) {
        if (ble_comm_dev_get_handle(i, GATT_ROLE_CLIENT) == con_handle) {
            break;
        }

        if (i == SUPPORT_MAX_GATT_CLIENT - 1) {
            log_info("No this handle to send!!");
            return 0;
        }
    }

    if (con_handle) {
        if (dg_central_record_bond_info[i].conn_handle == con_handle) {
            log_info("Send usb data to trans: %x, %x", con_handle, dg_central_record_bond_info[i].write_handle);
        } else {
            for (j = 0; j < SUPPORT_MAX_GATT_CLIENT; j++) {
                if (dg_central_record_bond_info[j].conn_handle == con_handle) {
                    i = j;
                }
            }
            log_info("Send usb data to trans: %x, %x", con_handle, dg_central_record_bond_info[i].write_handle);
        }

        if (!dg_central_record_bond_info[i].write_handle) {
            log_info("write_handle is err!!");
            return APP_BLE_OPERATION_ERROR;
        }

        return ble_comm_att_send_data(con_handle, dg_central_record_bond_info[i].write_handle, data, len, ATT_OP_WRITE_WITHOUT_RESPOND);
    }
    return APP_BLE_OPERATION_ERROR;
}
#endif


