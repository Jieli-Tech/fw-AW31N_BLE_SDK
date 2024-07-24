/*********************************************************************************************
    *   Filename        : app_nonconn_24g.c

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
#include "user_cfg.h"
#include "ble_noconn_deal.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#define LOG_TAG_CONST       NCON_24G
#define LOG_TAG             "[NCON_24G]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if CONFIG_APP_NONCONN_24G

static volatile uint8_t noconn_is_active = 0;

/*************************************************************************************************/
/*!
 *  \brief      设置低功耗状态
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void noconn_state_idle_set_active(uint8_t active)
{
    noconn_is_active = active;
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
static void noconn_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
    noconn_is_active = 1;
    btstack_ble_exit(0);
    app_power_set_soft_poweroff(NULL);
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
static void noconn_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(NULL, 0);
    cfg_file_parse(0);
    btstack_init();
}

/*************************************************************************************************/
/*!
 *  \brief      app  start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void noconn_app_start()
{
    log_info("=======================================");
    log_info("-----------nonconn_24g demo------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clock_bt_init();

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);
    noconn_bt_start();

    // 默认scan不开低功耗
#if CONFIG_RX_MODE_ENABLE
    noconn_is_active = 1;
#endif

#if TCFG_LED_ENABLE
    led_operate(LED_INIT);
#endif

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
static int noconn_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_NOCONN_24G_MAIN:
            noconn_app_start();
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
static int noconn_bt_hci_event_handler(struct bt_event *bt)
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
static int noconn_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);
    bt_comm_ble_status_event_handler(bt);
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
static void noconn_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);

        if (event_type == KEY_EVENT_TRIPLE_CLICK && (key_value == TCFG_ADKEY_VALUE0)) {
            noconn_set_soft_poweroff();
            return;
        }
        noconn_tx_key_test(event_type, key_value);
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
static int noconn_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        noconn_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            noconn_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            noconn_bt_hci_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, noconn_set_soft_poweroff);
        }

        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_noconn_ops = {
    .state_machine  = noconn_state_machine,
    .event_handler 	= noconn_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_noconn) = {
    .name 	= "nonconn_24g",
    .action	= ACTION_NOCONN_24G_MAIN,
    .ops 	= &app_noconn_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static uint8_t noconn_state_idle_query(void)
{
    return !noconn_is_active;
}

REGISTER_LP_TARGET(noconn_state_lp_target) = {
    .name = "noconn_state_deal",
    .is_idle = noconn_state_idle_query,
};

#endif


