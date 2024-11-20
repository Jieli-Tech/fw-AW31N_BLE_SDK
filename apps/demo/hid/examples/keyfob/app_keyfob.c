/*********************************************************************************************
    *   Filename        : app_keyfob.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

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
#include "led_control.h"
#include "standard_hid.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "app_comm_proc.h"
#include "ble_hogp.h"
#include "user_cfg.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if(CONFIG_APP_KEYFOB)

#define LOG_TAG_CONST      KEYFOB
#define LOG_TAG             "[KEYFOB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//按键ID
#define KEY_BIG_ID    0
#define KEY_SMALL_ID  1
// report id
#define KEYFOB_REPORT_ID   3

enum {
    KEYFOB_CLICK = 0,
    KEYFOB_CLICK_HOLD,
    KEYFOB_CLICK_UP
};

static uint16_t keyfob_auto_shutdown_timer = 0;
//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static volatile uint8_t keyfob_is_active = 0;
//----------------------------------
static const uint8_t keyfob_report_map[] = {
    //通用按键
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0B,        //   Report Count (11)
    0x0A, 0x23, 0x02,  //   Usage (AC Home)
    0x0A, 0x21, 0x02,  //   Usage (AC Search)
    0x0A, 0xB1, 0x01,  //   Usage (AL Screen Saver)
    0x09, 0xB8,        //   Usage (Eject)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0x30,        //   Usage (Power)
    0x0A, 0xAE, 0x01,  //   Usage (AL Keyboard Layout)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x0D,        //   Report Size (13)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // 119 bytes
};

//----------------------------------
static const ble_init_cfg_t keyfob_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_KEYBOARD,
    .report_map = keyfob_report_map,
    .report_map_size = sizeof(keyfob_report_map),
};

//----------------------------------------
static const uint8_t keyfob_big_press[3] = {0x00, 0x02, 0x00};
static const uint8_t keyfob_big_null[3] =  {0x00, 0x00, 0x00};

static const uint8_t keyfob_small_press[3] = {0x00, 0x01, 0x00};
static const uint8_t keyfob_small_null[3] =  {0x00, 0x00, 0x00};

static void keyfob_recv_callback(uint8_t *buffer, uint16_t size);
static void keyfob_set_soft_poweroff(void);

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
static void keyfob_auto_shutdown_disable(void)
{
    log_info("----%s", __FUNCTION__);
    if (keyfob_auto_shutdown_timer) {
        sys_timeout_del(keyfob_auto_shutdown_timer);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     按键发送数据
 *
 *  \param      [in] key_value:按键值 mode: 按键类型
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keyfob_send_value(uint8_t key_value, u8 mode)
{
    if (key_value == KEY_BIG_ID) {
        if (mode == KEYFOB_CLICK || mode == KEYFOB_CLICK_HOLD) {
            ble_hid_data_send(KEYFOB_REPORT_ID, (uint8_t *)keyfob_big_press, sizeof(keyfob_big_press));
        }
        if (mode == KEYFOB_CLICK || mode == KEYFOB_CLICK_UP) {
            ble_hid_data_send(KEYFOB_REPORT_ID, (uint8_t *)keyfob_big_null, sizeof(keyfob_big_null));
        }
    } else if (key_value == KEY_SMALL_ID) {
        if (mode == KEYFOB_CLICK || mode == KEYFOB_CLICK_HOLD) {
            ble_hid_data_send(KEYFOB_REPORT_ID, (uint8_t *)keyfob_small_press, sizeof(keyfob_small_press));
        }
        if (mode == KEYFOB_CLICK || mode == KEYFOB_CLICK_UP) {
            ble_hid_data_send(KEYFOB_REPORT_ID, (uint8_t *)keyfob_small_null, sizeof(keyfob_small_null));
        }
    }

}
/*************************************************************************************************/
/*!
 *  \brief      按键处理
 *
 *  \param      [in] key_type:按键类型 key_value:按键值
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keyfob_key_deal_test(uint8_t key_type, uint8_t key_value)
{
    uint16_t key_msg = 0;

    switch (key_type) {
    case KEY_EVENT_CLICK:
        keyfob_send_value(key_value, KEYFOB_CLICK);
#if TCFG_LED_ENABLE
        led_operate(LED_KEY_UP);
#endif
        break;

    case KEY_EVENT_LONG:
        keyfob_send_value(key_value, KEYFOB_CLICK);
        break;

    case KEY_EVENT_HOLD:
        if (key_value == KEY_BIG_ID) {
            //启动连拍
            keyfob_send_value(key_value, KEYFOB_CLICK_HOLD);
        }

#if TCFG_LED_ENABLE
        led_operate(LED_KEY_HOLD);
#endif
        break;

    case KEY_EVENT_UP:
        if (key_value == KEY_BIG_ID) {
            //停止连拍
            keyfob_send_value(key_value, KEYFOB_CLICK_UP);
        }

#if TCFG_LED_ENABLE
        led_operate(LED_KEY_UP);
#endif
        break;

    default:
        break;
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
static void keyfob_set_soft_poweroff(void)
{
    log_info("keyfob_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    keyfob_is_active = 1;
#endif

    btstack_ble_exit(0);

    if (ble_hid_is_connected()) {
        //延时确保BT退出链路断开
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
 *  \brief      bt初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keyfob_app_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);
    btstack_ble_start_before_init(&keyfob_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
    le_hogp_set_output_callback(keyfob_recv_callback);
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
static void keyfob_app_start()
{
    log_info("=======================================");
    log_info("-------------keyfob DEMO-----------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    keyfob_app_bt_start();

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
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
static void keyfob_recv_callback(uint8_t *buffer, uint16_t size)
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
static int keyfob_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_KEYFOB:
            keyfob_app_start();
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
 *  \brief      蓝牙ble hogp状态消息回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keyfob_hogp_ble_status_callback(ble_state_e status, uint8_t value)
{
    log_info("keyfob_hogp_ble_status_callback==================== %d   value:0x%x\n", status, value);
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
 *  \brief      蓝牙HCI事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int keyfob_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("------------------------keyfob_bt_hci_event_handler reason %x %x", bt->event, bt->value);
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
static int keyfob_bt_connction_status_event_handler(struct bt_event *bt)
{

    log_info("--------keyfob_bt_connction_status_event_handler %d", bt->event);

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
 *  \brief      蓝牙公共消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int keyfob_bt_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE \n");
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        keyfob_auto_shutdown_disable();
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
static void app_keyfob_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;

        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_event: %d,%d\n", event_type, key_value);
        if (event_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE1) {
            if (ble_hid_is_connected()) {
                le_hogp_disconnect();
            }
            le_hogp_set_pair_allow();
            return;
        }

        if (event_type == KEY_EVENT_TRIPLE_CLICK && key_value == TCFG_ADKEY_VALUE0) {
            keyfob_set_soft_poweroff();
            return;
        }

        keyfob_key_deal_test(event_type, key_value);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      app 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int keyfob_event_handler(struct application *app, struct sys_event *event)
{

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //重置无操作定时计数
    if (event->type != SYS_DEVICE_EVENT || DEVICE_EVENT_FROM_POWER != event->arg) {
        sys_timer_modify(keyfob_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
#endif

    switch (event->type) {
    case SYS_KEY_EVENT:
        app_keyfob_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            keyfob_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            keyfob_bt_hci_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            keyfob_hogp_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return keyfob_bt_common_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, keyfob_set_soft_poweroff);
        }
        return 0;

    default:
        return FALSE;
    }

    return FALSE;
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
//system check go sleep is ok
static uint8_t app_keyfob_idle_query(void)
{
    return !keyfob_is_active;
}

REGISTER_LP_TARGET(app_keyfob_lp_target) = {
    .name = "app_keyfob_deal",
    .is_idle = app_keyfob_idle_query,
};

static const struct application_operation app_keyfob_ops = {
    .state_machine  = keyfob_state_machine,
    .event_handler 	= keyfob_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_keyfob) = {
    .name 	= "keyfob",
    .action	= ACTION_KEYFOB,
    .ops 	= &app_keyfob_ops,
    .state  = APP_STA_DESTROY,
};

#endif



