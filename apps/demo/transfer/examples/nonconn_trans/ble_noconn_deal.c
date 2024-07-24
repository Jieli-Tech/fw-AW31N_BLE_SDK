/*********************************************************************************************
    *   Filename        : ble_24g_deal.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2024

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
#include "led_control.h"
#include "app_comm_proc.h"
#include "ble_noconn_deal.h"
#include "user_cfg.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

// todo change func name

#if CONFIG_APP_NONCONN_24G
#define LOG_TAG         "[ble_noconn]"
#include "log.h"

// noconn tx
static uint8_t noconn_adv_data_len = 0;
static uint8_t noconn_adv_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t noconn_scan_rsp_data_len = 0;
static uint8_t noconn_scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31
static const char noconn_ext_name[] = "(BLE)";
static uint8_t noconn_adv_ctrl_en = 0;             //adv控制
static char    noconn_gap_device_name[BT_NAME_LEN_MAX] = "jl_ble_test";
static uint8_t noconn_gap_device_name_len = 0;     //名字长度，不包含结束符

// noconn rx
static uint8_t noconn_scan_ctrl_en = 0;            //scan控制
static uint8_t noconn_test_buffer[64];
static uint8_t noconn_rx_buffer[ADV_RSP_PACKET_MAX * 2];
static uint8_t noconn_rx_len = 0;

/*************************************************************************************************/
/*!
 *  \brief      发射机广播参数设置
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_tx_setup_init(void)
{
    uint8_t adv_type;
    uint8_t adv_channel = ADV_CHANNEL_ALL;

    if (noconn_scan_rsp_data_len) {
        adv_type = ADV_SCAN_IND;
    } else {
        adv_type = ADV_NONCONN_IND;
    }

    ble_op_set_adv_param(ADV_INTERVAL_VAL, adv_type, adv_channel);
    ble_op_set_adv_data(noconn_adv_data_len, noconn_adv_data);
    put_buf(noconn_adv_data, noconn_adv_data_len);

    ble_op_set_rsp_data(noconn_scan_rsp_data_len, noconn_scan_rsp_data);
    put_buf(noconn_scan_rsp_data, noconn_scan_rsp_data_len);
}

/*************************************************************************************************/
/*!
 *  \brief      发射机广播开关
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_tx_enable(uint8_t enable)
{
#if CONFIG_TX_MODE_ENABLE
    if (noconn_adv_ctrl_en != enable) {
        noconn_adv_ctrl_en = enable;
        log_info("tx_en:%d\n", enable);
        if (enable) {
            noconn_tx_setup_init();
        }
        ble_op_adv_enable(enable);
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      发送数据, len support max is 60
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int noconn_tx_send_data(const uint8_t *data, uint8_t len)
{
    if (0 == len || len > (ADV_RSP_PACKET_MAX - 1) * 2) {
        log_info("len is overflow:%d\n", len);
        return -1;
    }

    noconn_adv_data_len = 0;
    noconn_scan_rsp_data_len = 0;

    if (len > ADV_RSP_PACKET_MAX - 1) {
        noconn_adv_data_len = ADV_RSP_PACKET_MAX - 1;
    } else {
        noconn_adv_data_len = len;
    }

    noconn_adv_data[0] = len;//packet len
    memcpy(&noconn_adv_data[1], data, noconn_adv_data_len);
    data += noconn_adv_data_len;
    len -= noconn_adv_data_len;

    noconn_adv_data_len++; //add head

    if (len) {
        noconn_scan_rsp_data[0] = RSP_TX_HEAD;
        noconn_scan_rsp_data_len = len + 1; //add head
        memcpy(&noconn_scan_rsp_data[1], data, len);
    }

    noconn_tx_enable(1);
    //延时确定发送成功
    sys_timeout_add(0, (void *)noconn_tx_enable, TX_DATA_COUNT * TX_DATA_INTERVAL / 10 + 1);
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      发送数据测试函数
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_tx_timer_test(void)
{
    static uint8_t tag_value = 0;
    uint8_t test_len = 60;
    if (tag_value == 0) {
        for (int i = 0; i < 64; i++) {
            noconn_test_buffer[i] = i;
        }
        tag_value++;
    }

    noconn_tx_send_data(noconn_test_buffer, test_len);
    log_info("tx_data: %02x", noconn_test_buffer[0]);
    put_buf(noconn_test_buffer, test_len);
    noconn_test_buffer[0]++;
}

/*************************************************************************************************/
/*!
 *  \brief      发送指定数据
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void noconn_tx_key_test(uint8_t key_event, uint8_t key_value)
{
#if CONFIG_TX_MODE_ENABLE
    log_info(">>>>>>>>>noconn_tx_key_test\n");
    // 24g收发通信test,
    // k1单击亮led1，k1双击灭led1
    // k2单机亮led2，k2双击灭led2
    on_off_opcode_t on_off_opcode;
    on_off_opcode.led_opcode = LED_ON_OFF_OPCODE;
    if (key_value == TCFG_ADKEY_VALUE0) {
        on_off_opcode.led_num = LED1;
        on_off_opcode.led_status = (key_event == KEY_EVENT_CLICK) ? LED_ON_STATUS : LED_OFF_STATUS;
    } else if (key_value == TCFG_ADKEY_VALUE1) {
        on_off_opcode.led_num = LED2;
        on_off_opcode.led_status = (key_event == KEY_EVENT_CLICK) ? LED_ON_STATUS : LED_OFF_STATUS;
    }

    if (key_event == KEY_EVENT_DOUBLE_CLICK) {
        on_off_opcode.led_status = LED_OFF_STATUS;
    }
    noconn_tx_send_data((const uint8_t *)&on_off_opcode, sizeof(on_off_opcode_t));
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      扫描接收到的数据
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_rx_data_handle(const uint8_t *data, uint16_t len)
{
    log_info("rx_data: %d", len);
    put_buf((uint8_t *)data, len);
#if BLE_SLAVE_CLIENT_LED_OP_EN
    on_off_opcode_t *op = (on_off_opcode_t *)data;
    led_onoff_op(op);
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      接收机数据处理回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_rx_report_handle(adv_report_t *report_pt, uint16_t len)
{
    /* log_info("event_type,addr_type:%x,%x\n", report_pt->event_type, report_pt->address_type); */
    /* log_info_hexdump(report_pt->address, 6); */
    static uint32_t scan_packet_num = 0;//for test
    /* r_printf("rx data:%d",report_pt->length); */
    /* put_buf(report_pt->data,report_pt->length);	 */
    if (report_pt == NULL) {
        return;
    }

    if (0 == report_pt->length) {
        return;
    }

    uint8_t data_len = report_pt->length - 1;

    if (!data_len) {
        return;
    }

    if (report_pt->data[0] == RSP_TX_HEAD) {
        memcpy(&noconn_rx_buffer[ADV_RSP_PACKET_MAX - 1], &report_pt->data[1], data_len);
        log_info("long_packet =%d\n", noconn_rx_len);
    } else {
        memcpy(noconn_rx_buffer, &report_pt->data[1], data_len);
        noconn_rx_len = report_pt->data[0];
        if (noconn_rx_len > ADV_RSP_PACKET_MAX - 1) {
            log_info("first_packet =%d,wait next packet\n", data_len);
            return;
        } else {
            log_info("short_packet =%d\n", noconn_rx_len);
        }
    }

    //for debug
    log_info("rssi:%d,packet_num:%u\n", report_pt->rssi, ++scan_packet_num);
    noconn_rx_data_handle(noconn_rx_buffer, noconn_rx_len);
    noconn_rx_len = 0;
}

/*************************************************************************************************/
/*!
 *  \brief      接收机事件回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_rx_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case GAP_EVENT_ADVERTISING_REPORT:
            putchar('V');
            noconn_rx_report_handle((void *)&packet[2], packet[1]);
            break;
        default:
            break;
        }
        break;
    }
}

/*************************************************************************************************/
/*!
 *  \brief      接收机初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_rx_setup_init(void)
{
    ble_op_set_scan_param(SET_SCAN_TYPE, SET_SCAN_INTERVAL, SET_SCAN_WINDOW);
}

/*************************************************************************************************/
/*!
 *  \brief      scan使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_rx_enable(uint8_t enable)
{
#if CONFIG_RX_MODE_ENABLE
    if (noconn_scan_ctrl_en != enable) {
        noconn_scan_ctrl_en = enable;
        log_info("rx_en:%d\n", enable);
        if (enable) {
            noconn_rx_setup_init();
        }
        ble_op_scan_enable2(enable, 0);
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief     协议栈内部调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void ble_profile_init(void)
{
    log_info("ble profile init\n");
    // register for HCI events
    hci_event_callback_set(&noconn_rx_event_handler);
}

/*************************************************************************************************/
/*!
 *  \brief     ble 模块使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void ble_module_enable(uint8_t en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
    } else {
        noconn_rx_enable(0);
        noconn_tx_enable(0);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     bt init
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
    log_info("***** ble_init******\n");
    log_info("ble_file: %s", __FILE__);

    const char *name_p;
    uint8_t ext_name_len = sizeof(noconn_ext_name) - 1;

#if CFG_RF_USE_24G_CDOE
    rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
#endif

#if CFG_RF_ADV_SCAN_CHL
    //通信信道，发送接收都要一致
    ble_rf_vendor_fixed_channel(36, 3);
#endif

    name_p = bt_get_local_name();
    noconn_gap_device_name_len = strlen(name_p);
    if (noconn_gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len) {
        noconn_gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len;
    }

    //增加后缀，区分名字
    memcpy(noconn_gap_device_name, name_p, noconn_gap_device_name_len);
    memcpy(&noconn_gap_device_name[noconn_gap_device_name_len], "(BLE)", ext_name_len);
    noconn_gap_device_name_len += ext_name_len;

    log_info("ble name(%d): %s \n", noconn_gap_device_name_len, noconn_gap_device_name);

#if CONFIG_TX_MODE_ENABLE
#if TX_TEST_SEND_MODE
    //for test
    sys_timer_add(NULL, (void *)noconn_tx_timer_test, 1000);
#endif
#endif

#if CONFIG_RX_MODE_ENABLE
    noconn_rx_enable(1);
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      修改2.4G CODED码
 *  \param      [in] coded         设置coded码为32bits.
 *  \param      [in] channel       设置广播(GATT_ROLE_SERVER) or 扫描(GATT_ROLE_CLIENT)
 *
 *  \note       在初始化完成后任意非连接时刻修改CODED码
 */
/*************************************************************************************************/
void rf_set_conn_24g_coded(uint32_t coded, uint8_t channel)
{
    if (channel == GATT_ROLE_CLIENT) {
        noconn_rx_enable(0);
        rf_set_scan_24g_hackable_coded(coded);
        noconn_rx_enable(1);
    } else {
        noconn_tx_enable(0);
        rf_set_adv_24g_hackable_coded(coded);
        noconn_tx_enable(1);
    }
}
/*************************************************************************************************/
/*!
 *  \brief     bt exit
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
    log_info("***** ble_exit******\n");
    ble_module_enable(0);
}
#endif

