/*********************************************************************************************
    *   Filename        : app_trans.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2023-10-05 10:09

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
#include "ble_trans.h"
#include "cpu_debug.h"
#include "my_malloc.h"
#if TCFG_NORMAL_SET_DUT_MODE_API
#include "ble_test_api.h"
#endif
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#define  LE_DEBUG_TIMER_INFO    0


#if (CONFIG_APP_LE_TRANS)
#define LOG_TAG_CONST       LE_TRANS
#define LOG_TAG             "[LE_TRANS]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

static u8 le_trans_is_active = 0;
static const ble_init_cfg_t le_trans_data_config = {
    .same_address = 0,
    .appearance = 0,
};

static void le_timer_test_handle(void *priv)
{
#if LE_DEBUG_TIMER_INFO
    log_info("timer_1s");
    static u8 cnt = 0;
    if (++cnt % 5 == 0) {
    }
#endif
}

//---------------------------------------------------------------------
static void le_trans_set_soft_poweroff(void)
{
    log_info("le_trans_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    le_trans_is_active = 1;
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
 *  \brief      app bt  start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void le_trans_bt_start()
{
    u32 sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(&le_trans_data_config, 0);

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
static void le_trans_app_start()
{
    log_info("=======================================");
    log_info("-----------le trans demo-------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    le_trans_bt_start();

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
#endif

    // 配置一个通用串口做ble2uart or uart2ble
#if TCFG_COMMON_UART_ENABLE
    common_uart_init(TCFG_COMMON_UART_BAUDRATE, COMMON_UART_TX_PIN, COMMON_UART_RX_PIN);
#endif

#if LE_DEBUG_TIMER_INFO
    sys_timer_add(0, le_timer_test_handle, 1000);
#endif

    int msg[4] = {0};
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
static int le_trans_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_LE_TRANS_MAIN:
            le_trans_app_start();
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
static int le_trans_bt_hci_event_handler(struct bt_event *bt)
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
static int le_trans_bt_connction_status_event_handler(struct bt_event *bt)
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
static void le_trans_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        u8 event_type = 0;
        u8 key_value = 0;

        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);

// dut api test
#if TCFG_NORMAL_SET_DUT_MODE_API
        ble_dut_mode_key_handle(event_type, key_value);
        return;
#endif

        if (event_type == KEY_EVENT_TRIPLE_CLICK && key_value == TCFG_ADKEY_VALUE0) {
            le_trans_set_soft_poweroff();
            return;
        }

#if TCFG_SYS_LVD_EN
        if (event_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE3) {
            static u8 test_low_power_flag = 0;
            test_low_power_flag = ~test_low_power_flag;
            if (test_low_power_flag) {
                app_power_set_lvd(6000);
            } else {
                app_power_set_lvd(3300);
            }
            return;
        }
#endif

        trans_key_deal_test(event_type, key_value);
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
static int le_trans_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        le_trans_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            le_trans_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            le_trans_bt_hci_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, le_trans_set_soft_poweroff);
        }
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_le_trans_ops = {
    .state_machine  = le_trans_state_machine,
    .event_handler 	= le_trans_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_le_trans) = {
    .name 	= "le_trans",
    .action	= ACTION_LE_TRANS_MAIN,
    .ops 	= &app_le_trans_ops,
    .state  = APP_STA_DESTROY,
};

/*************************************************************************************************/
/*!
 *  \brief      注册控制是否允许系统进入sleep状态
 *
 *  \param      [in]
 *
 *  \return     1--可以进入sleep  0--不允许
 *
 *  \note
 */
/*************************************************************************************************/
static u8 le_trans_state_idle_query(void)
{
    return !le_trans_is_active;
}

REGISTER_LP_TARGET(le_trans_state_lp_target) = {
    .name = "le_trans_state_deal",
    .is_idle = le_trans_state_idle_query,
};

#endif


