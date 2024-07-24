/*********************************************************************************************
    *   Filename        : app_at_char_com.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :2024

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
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "app_modules.h"
#include "led_control.h"
#include "common_uart_control.h"
#include "app_comm_proc.h"
#include "user_cfg.h"
#include "ble_at_char_client.h"
#include "ble_at_char_server.h"
#include "ble_at_char.h"
#include "at_char_cmds.h"
#include "at_char_uart.h"
#include "le_gatt_common.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#define LOG_TAG_CONST       AT_COM
#define LOG_TAG             "[AT_COM]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if  CONFIG_APP_AT_CHAR_COM

#define TEST_ATCHAR_AUTO_BT_OPEN        1//for test

static uint8_t atchar_app_is_active = 1;
static uint8_t atchar_force_wakeup;

//---------------------------------------------------------------------
void atchar_set_soft_poweroff()
{
    log_info("set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    atchar_app_is_active = 1;
#endif
    //必须先主动断开蓝牙链路,否则要等链路超时断开
    btstack_ble_exit(0);
    //延时，确保BT退出链路断开

    if (ble_comm_dev_is_connected(GATT_ROLE_SERVER) || ble_comm_dev_is_connected(GATT_ROLE_CLIENT)) {
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
 *  \brief      设置当前低功耗状态
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void at_set_low_power_mode(uint8_t enable)
{
    atchar_app_is_active = !enable;
    if (atchar_app_is_active) {
        atchar_force_wakeup = 1;
    }
}

/*************************************************************************************************/
/*!
 *  \brief      获取当前低功耗状态
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
uint8_t at_get_low_power_mode(void)
{
    return !atchar_app_is_active;
}

/*************************************************************************************************/
/*!
 *  \brief      设置进入低功耗
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void at_go_low_power(void *priv)
{
    // at_set_low_power_mode设置出低功耗时不能再进入低功耗
    if (atchar_force_wakeup) {
        atchar_force_wakeup = 0;
        return;
    }
    atchar_app_is_active = 0;
}

/*************************************************************************************************/
/*!
 *  \brief      唤醒函数,UART发数唤醒一秒
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void at_char_wake_up_set(P33_IO_WKUP_EDGE edge)
{
    if (!atchar_app_is_active) {
        atchar_app_is_active = 1;
        sys_timeout_add(NULL, at_go_low_power, 1000);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      app bt start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void atchar_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(NULL, 0);

    cfg_file_parse(0);
    btstack_init();
}

/*************************************************************************************************/
/*!
 *  \brief      app start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void atchar_app_start()
{
    log_info("=======================================");
    log_info("-------------atchar_com demo---------------");
    log_info("=======================================");

    log_info("app_file: %s", __FILE__);

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    atchar_bt_start();

    int msg[4]   = {0};
    while (1) {
        get_msg(sizeof(msg) / sizeof(int), msg);
        app_comm_process_handler(msg);
    }
}
/*************************************************************************************************/
/*!
 *  \brief      app 状态机处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int atchar_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_AT_COM:
            atchar_app_start();
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
 *  \brief      hci 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int atchar_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    bt_comm_ble_hci_event_handler(bt);
    return 0;
}
/*************************************************************************************************/
/*!
 *  \brief      bt 连接状态处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int atchar_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_info("BT_STATUS_INIT_OK\n");

        at_cmd_init();

        bt_ble_init();

#if TEST_ATCHAR_AUTO_BT_OPEN
#if CONFIG_BT_GATT_SERVER_NUM
        ble_test_auto_adv(1);
#endif
#if CONFIG_BT_GATT_CLIENT_NUM
        ble_test_auto_scan(1);
#endif
#endif
        break;

    default:
        bt_comm_ble_status_event_handler(bt);
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
static void atchar_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = event->u.key.event;
        uint8_t key_value = event->u.key.value;
        log_info("app_key_event: %d,%d\n", event_type, key_value);


        if (event_type == KEY_EVENT_TRIPLE_CLICK
            && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
            atchar_set_soft_poweroff();
            return;
        }

        if (event_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE0) {
#if CONFIG_BT_GATT_CLIENT_NUM
            // 发数测试
            u8 data[3] = {0x11, 0x22, 0x33};
            ble_at_client_send_data(1, data, 3);
#endif
            return;
        }
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
static int atchar_event_handler(struct application *app, struct sys_event *event)
{
    uint32_t cbuf_len = 0;
    switch (event->type) {
    case SYS_KEY_EVENT:
        atchar_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            atchar_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            atchar_bt_hci_event_handler(&event->u.bt);
        }
        return 0;


    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, atchar_set_soft_poweroff);
        }
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation atchar_com_ops = {
    .state_machine  = atchar_state_machine,
    .event_handler 	= atchar_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_atchar_com) = {
    .name 	= "at_com",
    .action	= ACTION_AT_COM,
    .ops 	= &atchar_com_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static uint8_t app_atchar_state_idle_query(void)
{
    return !atchar_app_is_active;
}

REGISTER_LP_TARGET(app_atchar_lp_target) = {
    .name = "app_at_com_deal",
    .is_idle = app_atchar_state_idle_query,
};

#endif


