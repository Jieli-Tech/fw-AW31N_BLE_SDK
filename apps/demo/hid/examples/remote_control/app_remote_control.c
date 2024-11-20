/*********************************************************************************************
    *   Filename        : app_remote_control.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
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
#include "app_comm_proc.h"
#include "led_control.h"
#include "ble_hogp.h"
#include "ir_encoder.h"
#include "user_cfg.h"
#include "standard_hid.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif
#include "app_modules.h"
#if (CONFIG_APP_REMOTE_CONTROL)

#define LOG_TAG_CONST       HID_RC
#define LOG_TAG             "[HID_RC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020
#define CONSUMER_SCAN_FRAME_FORWARD     0x0040
#define CONSUMER_SCAN_FRAME_BACK        0x0080
#define IR_POWER_CMD_VALUE              0x0d

//---------------------------------------------------------------------
//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static volatile uint8_t hidrc_is_active;
static uint16_t hidrc_auto_shutdown_timer;
//----------------------------------
static const uint8_t hidrc_report_map[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB3,        //   Usage (Fast Forward)
    0x09, 0xB4,        //   Usage (Rewind)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    // 35 bytes
};

//----------------------------------
static const uint16_t hidrc_click_table[10] = {
    0,
    CONSUMER_SCAN_PREV_TRACK,
    CONSUMER_MUTE,
    CONSUMER_VOLUME_DEC,
    CONSUMER_PLAY_PAUSE,
    CONSUMER_VOLUME_INC,
    0,
    CONSUMER_SCAN_NEXT_TRACK,
    0,
    0,
};

static const uint16_t hidrc_hold_table[10] = {
    0,
    0,
    0,
    CONSUMER_VOLUME_DEC,
    0,
    CONSUMER_VOLUME_INC,
    0,
    0,
    0,
    0,
};

//----------------------------------
const uint8_t hidrc_ir_cmd_tab[] = { // 红外cmd码表,匹配海信电视
    0x14, // menu
    0x16, // up
    0,
    /* 0x19, // left */
    /* 0x18, // right */
    0x43, // vol down
    0x15, // confirm
    0x44, // vol up
    0x48, // return
    0x17, // down
    0x94, // main page
    0,
    0x0d, // power
};
//----------------------------------

//----------------------------------

static const ble_init_cfg_t hidrc_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_GENERIC_REMOTE_CONTROL,
    .report_map = hidrc_report_map,
    .report_map_size = sizeof(hidrc_report_map),
};

static void hidrc_set_soft_poweroff(void);
static void hidrc_recv_callback(uint8_t *buffer, uint16_t size);

/*************************************************************************************************/
/*!
 *  \brief      删除auto关机
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_auto_shutdown_disable(void)
{
    log_info("----%s", __FUNCTION__);
    if (hidrc_auto_shutdown_timer) {
        sys_timeout_del(hidrc_auto_shutdown_timer);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     发送红外数据
 *
 *  \param      [in]cmd：操作码 is_long:是否持续发送
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
#if TCFG_IR_ENABLE
static void hidrc_send_ir_key(uint8_t cmd, uint8_t is_long)
{
    ir_encoder_tx_hx(0, cmd, is_long);
}

static void hidrc_ir_key_deal(uint8_t key_value, uint8_t key_type)
{
    // ir test
    uint8_t cmd = hidrc_ir_cmd_tab[key_value];
    if (!cmd) {
        return;
    }

    // use ble control if connected,except power cmd
    if (cmd != IR_POWER_CMD_VALUE && ble_hid_is_connected()) {
        return;
    }

    if (key_type == KEY_EVENT_CLICK) {
        hidrc_send_ir_key(cmd, 0);
    } else if (key_type == KEY_EVENT_HOLD) {
        hidrc_send_ir_key(cmd, 1);
    } else if (key_type == KEY_EVENT_UP) {
        ir_encoder_repeat_stop();
    } else {
        ;
    }
}
#endif
/*************************************************************************************************/
/*!
 *  \brief      按键处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_app_key_deal_test(uint8_t key_type, uint8_t key_value)
{
    uint16_t key_msg = 0;
    uint16_t key_msg_up = 0;

#if TCFG_IR_ENABLE
    hidrc_ir_key_deal(key_value, key_type);
#endif

#if TCFG_LED_ENABLE
    led_operate(LED_KEY_UP);
#endif

    if (key_type == KEY_EVENT_CLICK) {
        key_msg = hidrc_click_table[key_value];
    } else if (key_type == KEY_EVENT_HOLD) {
        key_msg = hidrc_hold_table[key_value];
    } else if (key_type == KEY_EVENT_UP) {
        log_info("key_up_val = %02x\n", key_value);
        ble_hid_data_send(1, (uint8_t *)&key_msg_up, 2);
        return;
    }

    if (key_msg) {
        log_info("key_msg = %02x\n", key_msg);
        ble_hid_data_send(1, (uint8_t *)&key_msg, 2);
        if (KEY_EVENT_HOLD != key_type) {
            ble_hid_data_send(1, (uint8_t *)&key_msg_up, 2);
        }
        return;
    }

    if (key_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE1) {
        if (ble_hid_is_connected()) {
            le_hogp_disconnect();
        }
        le_hogp_set_pair_allow();
        return;
    }

    if (key_type == KEY_EVENT_TRIPLE_CLICK
        && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
        hidrc_set_soft_poweroff();
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \brief      进入软关机
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_set_soft_poweroff(void)
{
    log_info("hidrc_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    hidrc_is_active = 1;
#endif

    btstack_ble_exit(0);

    if (ble_hid_is_connected()) {
        //延时，确保BT退出链路断开
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
        //soft 方式非必须等链路断开
        sys_timeout_add(NULL, app_power_set_soft_poweroff, WAIT_DISCONN_TIME_MS);
#elif (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        //must wait disconn
        app_power_soft.wait_disconn = 1;
#endif
    } else {
        app_power_set_soft_poweroff(NULL);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     app bt初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_app_bt_start()
{
    u32 sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(&hidrc_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
    le_hogp_set_output_callback(hidrc_recv_callback);

    btstack_init();
}

/*************************************************************************************************/
/*!
 *  \brief      app 入口
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_app_start()
{
    log_info("=======================================");
    log_info("-------------HID_RC DEMO-----------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);
    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    hidrc_app_bt_start();

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
#endif

#if TCFG_IR_ENABLE
    ir_encoder_init(IR_KEY_IO, IR_WORK_FRQ, IR_WORK_DUTY);
#endif

    int msg[4] = {0};
    while (1) {
        get_msg(sizeof(msg) / sizeof(int), msg);
        app_comm_process_handler(msg);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      hid 数据接收
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_recv_callback(uint8_t *buffer, uint16_t size)
{
    log_info("ble_hid_data_receive:size=%d", size);
    put_buf(buffer, size);

    return;
}

/*************************************************************************************************/
/*!
 *  \brief      app  状态处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidrc_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_REMOTE_CONTROL:
            hidrc_app_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY\n");
        break;
    }

    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      蓝牙HCI事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidrc_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    bt_comm_ble_hci_event_handler(bt);

    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      蓝牙连接状态事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidrc_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);


    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        // 蓝牙初始化完成
        log_info("BT_STATUS_INIT_OK\n");
        btstack_ble_start_after_init(0);
        break;

    default:
        bt_comm_ble_status_event_handler(bt);
        break;
    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      蓝牙ble hogp状态消息回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_hogp_ble_status_callback(ble_state_e status, uint8_t value)
{
    log_info("hidrc_hogp_ble_status_callback============== %02x   value:0x%x\n", status, value);
    switch (status) {
    case BLE_ST_CONNECT:
#if TCFG_LED_ENABLE
        led_set_connect_flag(1);
        led_operate(LED_CLOSE);
        // 连接过程中灯常亮
        led_operate(LED_ON);
#endif
        log_info("BLE_ST_CONNECT\n");
        break;

    case BLE_ST_DISCONN:
#if TCFG_LED_ENABLE
        led_set_connect_flag(0);
        if (value == ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
            led_operate(LED_CLOSE);
        } else {
            led_operate(LED_WAIT_CONNECT);
        }
#endif
        log_info("BLE_ST_DISCONN BY LOCAL\n");

#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif
        break;

    case BLE_ST_NOTIFY_IDICATE:
        log_info("BLE_ST_NOTIFY_IDICATE\n");
        break;

    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:
        log_info("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        break;

    case BLE_ST_CONNECT_UPDATE_REQUEST:
        log_info("BLE_ST_CONNECT_UPDATE_REQUEST\n");
        break;

    case BLE_ST_CONNECTION_UPDATE_OK:
#if TCFG_LED_ENABLE
        // 连接完成灯灭
        led_operate(LED_CLOSE);
#endif
        // update timer period from connect inteval
        log_info("BLE_ST_CONNECTION_UPDATE_OK, INTERVAL:%d\n", value);
        break;

    default:
        break;
    }
}
/*************************************************************************************************/
/*!
 *  \brief      蓝牙公共消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidrc_bt_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE,%d \n", bt->value);
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        hidrc_auto_shutdown_disable();
        break;

    default:
        break;

    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      按键事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidrc_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_event: %d,%d\n", event_type, key_value);
        hidrc_app_key_deal_test(event_type, key_value);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      app 线程事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidrc_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        hidrc_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            hidrc_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            hidrc_bt_hci_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            hidrc_hogp_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((u32)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return hidrc_bt_common_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, hidrc_set_soft_poweroff);
        }
        return 0;

    default:
        return 0;
    }

    return 0;
}


/*************************************************************************************************/
/*!
 *  \brief      注册控制是否进入sleep
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
//-----------------------
//system check go sleep is ok
static uint8_t hidrc_app_idle_query(void)
{
    return !hidrc_is_active;
}

REGISTER_LP_TARGET(app_hidrc_lp_target) = {
    .name = "app_hidrc_deal",
    .is_idle = hidrc_app_idle_query,
};


static const struct application_operation app_hidrc_ops = {
    .state_machine  = hidrc_state_machine,
    .event_handler 	= hidrc_event_handler,
};

/*
 * 注册模式
 */
REGISTER_APPLICATION(app_hidrc) = {
    .name 	= "hid_rc",
    .action	= ACTION_REMOTE_CONTROL,
    .ops 	= &app_hidrc_ops,
    .state  = APP_STA_DESTROY,
};


#endif

