/*********************************************************************************************
    *   Filename        : app_keyboard.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include <stdlib.h>
#include "cpu_debug.h"
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
#include "standard_hid.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "user_cfg.h"
#include "my_malloc.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if (CONFIG_APP_KEYBOARD)
#define LOG_TAG_CONST       HID_KEY
#define LOG_TAG             "[HID_KEY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

#define  HID_DEBUG_TIMER_INFO             0

//测试发数据
#define  HID_TEST_KEEP_SEND_EN            1

// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020
#define CONSUMER_SCAN_FRAME_FORWARD     0x0040
#define CONSUMER_SCAN_FRAME_BACK        0x0080


//---------------------------------------------------------------------
//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static volatile uint8_t hidkey_is_active;
static uint8_t hidkey_keep_send_flag;
static uint16_t hidkey_auto_shutdown_timer;
static uint16_t hidkey_timer_handle;
static uint32_t hidkey_timer_handle_interval = 10;
//----------------------------------
static const uint8_t hidkey_report_map[] = {
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
static const uint16_t hidkey_click_table[8] = {
    CONSUMER_PLAY_PAUSE,
    CONSUMER_SCAN_PREV_TRACK,
    CONSUMER_VOLUME_DEC,
    CONSUMER_SCAN_NEXT_TRACK,
    CONSUMER_VOLUME_INC,
    CONSUMER_MUTE,
    0,
    0,
};

static const uint16_t hidkey_hold_table[8] = {
    0,
    CONSUMER_SCAN_FRAME_BACK,
    CONSUMER_VOLUME_DEC,
    CONSUMER_SCAN_FRAME_FORWARD,
    CONSUMER_VOLUME_INC,
    0,
    0,
    0,
};

//----------------------------------
static const ble_init_cfg_t hidkey_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_KEYBOARD,
    .report_map = hidkey_report_map,
    .report_map_size = sizeof(hidkey_report_map),
};

static void hidkey_set_soft_poweroff(void);
static void hidkey_recv_callback(uint8_t *buffer, uint16_t size);

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
static void hidkey_auto_shutdown_disable(void)
{
    log_info("----%s", __FUNCTION__);
    if (hidkey_auto_shutdown_timer) {
        sys_timeout_del(hidkey_auto_shutdown_timer);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      测试一直发空键
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
#if HID_TEST_KEEP_SEND_EN
void hidkey_test_keep_send_data(void *priv)
{
    static const uint8_t test_data_000[8] = {0, 0, 0, 0};
    if (hidkey_keep_send_flag && ble_hid_is_connected()) {
        ble_hid_data_send(1, (uint8_t *)test_data_000, sizeof(test_data_000));
    }
}

/*************************************************************************************************/
/*!
 *  \brief      初始化测试发送
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void hidkey_test_keep_send_init(void)
{
    log_info("###keep test ble\n");
    hidkey_timer_handle = sys_timer_add((void *)0, hidkey_test_keep_send_data, hidkey_timer_handle_interval);
}

/*************************************************************************************************/
/*!
 *  \brief      关闭测试发送
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void hidkey_test_keep_send_deinit(void)
{
    log_info("###close test ble\n");
    if (hidkey_timer_handle) {
        sys_timer_del(hidkey_timer_handle);
        hidkey_timer_handle = 0;
    }
}
#endif

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
static void hidkey_app_key_deal_test(uint8_t key_type, uint8_t key_value)
{
    uint16_t key_msg = 0;
    uint16_t key_msg_up = 0;

#if HID_TEST_KEEP_SEND_EN
    if (key_type == KEY_EVENT_LONG && key_value == TCFG_ADKEY_VALUE0) {
        hidkey_keep_send_flag = !hidkey_keep_send_flag;
        if (hidkey_keep_send_flag) {
            hidkey_test_keep_send_init();
        } else {
            hidkey_test_keep_send_deinit();
        }
        log_info("hidkey_keep_send_flag=%d\n", hidkey_keep_send_flag);
        sleep_run_check_enalbe(!hidkey_keep_send_flag);
        return;
    }
#endif

#if TCFG_LED_ENABLE
    led_operate(LED_KEY_UP);
#endif

    if (key_type == KEY_EVENT_CLICK) {
        key_msg = hidkey_click_table[key_value];
    } else if (key_type == KEY_EVENT_HOLD) {
        key_msg = hidkey_hold_table[key_value];
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

#if TCFG_SYS_LVD_EN
    if (key_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE3) {
        static uint8_t test_low_power_flag = 0;
        test_low_power_flag = ~test_low_power_flag;
        if (test_low_power_flag) {
            app_power_set_lvd(6000);
        } else {
            app_power_set_lvd(2800);
        }
        return;
    }
#endif

    if (key_type == KEY_EVENT_TRIPLE_CLICK
        && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
        hidkey_set_soft_poweroff();
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
static void hidkey_set_soft_poweroff(void)
{
    log_info("hidkey_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    hidkey_is_active = 1;
#endif

    btstack_ble_exit(0);

    if (ble_hid_is_connected()) {
        /* if(ble_comm_dev_is_connected(GATT_ROLE_SERVER) || ble_comm_dev_is_connected(GATT_ROLE_CLIENT)) { */
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
 *  \brief     app bt初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidkey_app_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    //bt normal mode
    btstack_ble_start_before_init(&hidkey_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
    le_hogp_set_output_callback(hidkey_recv_callback);


    cfg_file_parse(0);
    btstack_init();
}

static void hidkey_timer_test_handle(void *priv)
{
#if HID_DEBUG_TIMER_INFO
    log_info("timer_1s");
    static u8 cnt = 0;
    if (++cnt % 5 == 0) {
    }
#endif
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
static void hidkey_app_start()
{

    log_info("=======================================");
    log_info("-------------HID DEMO-----------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    hidkey_app_bt_start();

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
#endif

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //无操作定时软关机
    hidkey_auto_shutdown_timer = sys_timeout_add(NULL, (void *)hidkey_set_soft_poweroff, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
#endif

#if HID_DEBUG_TIMER_INFO
    sys_timer_add(0, hidkey_timer_test_handle, 1000);
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
static void hidkey_recv_callback(uint8_t *buffer, uint16_t size)
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
static int hidkey_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_HID_MAIN:
            hidkey_app_start();
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
static int hidkey_bt_hci_event_handler(struct bt_event *bt)
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
static int hidkey_bt_connction_status_event_handler(struct bt_event *bt)
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
static void hidkey_hogp_ble_status_callback(ble_state_e status, uint8_t value)
{
    log_info("hidkey_hogp_ble_status_callback============== %02x   value:0x%x\n", status, value);
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
#if HID_TEST_KEEP_SEND_EN
        hidkey_timer_handle_interval = value;
        if (hidkey_timer_handle) {
            sys_timer_modify(hidkey_timer_handle, value);
        }
#endif
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
static int hidkey_bt_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE,%d \n", bt->value);
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        hidkey_auto_shutdown_disable();
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
static void hidkey_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_event: %d,%d\n", event_type, key_value);
        hidkey_app_key_deal_test(event_type, key_value);
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
static int hidkey_event_handler(struct application *app, struct sys_event *event)
{
#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //重置无操作定时计数
    if (event->type != SYS_DEVICE_EVENT || DEVICE_EVENT_FROM_POWER != (int)event->arg) { //过滤电源消息
        sys_timer_modify(hidkey_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
#endif

    switch (event->type) {
    case SYS_KEY_EVENT:
        hidkey_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            hidkey_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            hidkey_bt_hci_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            hidkey_hogp_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return hidkey_bt_common_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, hidkey_set_soft_poweroff);
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
static uint8_t hidkey_app_idle_query(void)
{
    return !hidkey_is_active;
}

REGISTER_LP_TARGET(app_hidkey_lp_target) = {
    .name = "app_hidkey_deal",
    .is_idle = hidkey_app_idle_query,
};


static const struct application_operation app_hidkey_ops = {
    .state_machine  = hidkey_state_machine,
    .event_handler 	= hidkey_event_handler,
};

/*
 * 注册模式
 */
REGISTER_APPLICATION(app_hidkey) = {
    .name 	= "hid_key",
    .action	= ACTION_HID_MAIN,
    .ops 	= &app_hidkey_ops,
    .state  = APP_STA_DESTROY,
};


#endif

