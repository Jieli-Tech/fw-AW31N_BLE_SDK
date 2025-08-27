/*********************************************************************************************
 *   Filename        : mouse_single.c

 *   Description     :鼠标单模 BLE(133回报率)

    *   Author          :

    *   Email           :

   *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include <stdlib.h>
#include "cpu.h"
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
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "OMSensor_manage.h"
#include "gptimer.h"
#include "user_cfg.h"
#include "app_mouse_single.h"
#include "app_modules.h"

#if(CONFIG_APP_MOUSE_SINGLE)

#define LOG_TAG_CONST       MOUSE
#define LOG_TAG             "[MOUSE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

static const char mouse_ble_name[] = "AW31N_MOUSE_SINGLE";
static volatile uint8_t mouse_is_active;// 1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static uint32_t mouse_reset_cnt;
static uint8_t  mouse_double_key_long_cnt; // key cnt
static uint8_t  mouse_switch_key_long_cnt;
static uint8_t  mouse_cpi_mode = MOUSE_CPI_1000; // mouse cpi setting, default 1000

static mouse_info_t mouse_info;
static mouse_send_flags_t mouse_flag;

static mouse_packet_data_t mouse_send_packet;

static const ble_init_cfg_t mouse_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_MOUSE,
    .report_map = mouse_report_map,
    .report_map_size = sizeof(mouse_report_map),
};

static void mouse_optical_sensor_event_handler(struct sys_event *event);
static void mouse_send_data_test(void);
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
void mouse_auto_shutdown_disable(void)
{
    if (mouse_info.mouse_auto_shutdown_timer) {
        sys_timeout_del(mouse_info.mouse_auto_shutdown_timer);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     鼠标发数函数，定时器调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_data_send(void *priv_hw, uint8_t hw_state, bool is_24g)
{
#if TEST_MOUSE_SIMULATION_ENABLE
    mouse_send_data_test();
#else
#ifdef TCFG_OMSENSOR_ENABLE
    optical_mouse_read_sensor_handler_high(&mouse_send_packet, &mouse_flag);
#endif
#endif
    // 取数
    if (!(mouse_flag.sensor_send_flag && mouse_flag.wheel_send_flag && mouse_flag.button_send_flag)) {
        mouse_reset_cnt++;
        uint32_t mouse_reset_cnt_max;
        mouse_reset_cnt_max = MOUSE_7MS_CLEAR_WDT_CNT_MAX;

        if (mouse_reset_cnt > mouse_reset_cnt_max) {
#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
            sys_timer_modify(mouse_info.mouse_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
#endif
            // 避免看门狗超时
            wdt_clear();
            mouse_reset_cnt = 0;
        }
        ble_hid_data_send(MOUSE_SEND_DATA_REPORT_ID, (uint8_t *)&mouse_send_packet, sizeof(mouse_send_packet));

        // 清除鼠标信息
        if (mouse_flag.button_send_flag == 0) {
            mouse_flag.button_send_flag = 1;
        }

        if (mouse_flag.wheel_send_flag == 0) {
            mouse_flag.wheel_send_flag = 1;
            mouse_send_packet.wheel = 0;
        }

        if (mouse_flag.sensor_send_flag == 0) {
            mouse_flag.sensor_send_flag = 1;
            memset((void *)&mouse_send_packet.xymovement, 0, sizeof(mouse_send_packet.xymovement));
        }
    } else {
        // 未取到按键/传感器数据，过滤
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \brief     BLE 鼠标发数函数，定时器调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_timer_handler(u32 tid, void *private_data)
{
    if (!mouse_info.mouse_is_paired) {
        return;
    }

    mouse_data_send(NULL, 0, false);
}

/*************************************************************************************************/
/*!
 *  \brief     重启
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_set_soft_reset(void *priv)
{
    cpu_reset();
    while (1);
}

/*************************************************************************************************/
/*!
 *  \brief      更新鼠标滚轮器值
 *
 *  \param      [in] event:code_sw事件
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_code_sw_event_handler(struct sys_event *event)
{
    static s8 sw_val = 0;

    if (mouse_flag.wheel_send_flag) {
        sw_val = 0;
    }

    if (event->u.codesw.event == 0) {
        sw_val += event->u.codesw.value;
        mouse_send_packet.wheel = -sw_val;
    }

    mouse_flag.wheel_send_flag = 0;
}

/*************************************************************************************************/
/*!
 *  \brief      更新gsensor位置
 *
 *  \param      [in] event:事件 x：x轴值 y：y轴值
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_optical_sensor_event(uint8_t event, s16 x, s16 y)
{
    static s16 delta_x = 0, delta_y = 0;
    if (mouse_flag.sensor_send_flag) {
        delta_x = 0;
        delta_y = 0;
    }

    if (event == 0) {
        if (((delta_x + x) >= -2047) && ((delta_x + x) <= 2047)) {
        } else {
            x = 0;
        }

        if (((delta_y + y) >= -2047) && ((delta_y + y) <= 2047)) {
        } else {
            y = 0;
        }

        //坐标调整
        delta_x += (-y);
        delta_y += (x);

        mouse_send_packet.xymovement[SENSOR_XLSB_IDX] = delta_x & 0xFF;
        mouse_send_packet.xymovement[SENSOR_YLSB_XMSB_IDX] = ((delta_y << 4) & 0xF0) | ((delta_x >> 8) & 0x0F);
        mouse_send_packet.xymovement[SENSOR_YMSB_IDX] = (delta_y >> 4) & 0xFF;

        /* log_info("x = %d.\ty = %d.\n", event->u.axis.x, event->u.axis.y); */
    }
    mouse_flag.sensor_send_flag = 0;
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
static void mouse_set_soft_poweroff()
{
    log_info("mouse_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    mouse_is_active = 1;
#endif
    //必须先主动断开蓝牙链路,否则要等链路超时断开
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
static void mouse_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(&mouse_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_IND, 5000);

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
static void mouse_app_start()
{
    log_info("=======================================");
    log_info("-------------Single Mouse--------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);
    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    mouse_bt_start();

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //无操作定时软关机
    mouse_info.mouse_auto_shutdown_timer = sys_timeout_add(NULL, mouse_set_soft_poweroff, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
#endif

    int msg[4] = {0};
    while (1) {
        get_msg(sizeof(msg) / sizeof(int), msg);
        app_comm_process_handler(msg);
    }

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
static int mouse_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_MOUSE_MAIN:
            mouse_app_start();
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
static int mouse_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("------------------------mouse_bt_hci_event_handler reason %x %x", bt->event, bt->value);
    bt_comm_ble_hci_event_handler(bt);
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      BLE模式 mouse发送定时器，使用gtimer
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_send_data_timer_init(uint32_t resolu_us)
{
    const struct gptimer_config mouse_timer_config = {
        .timer.period_us = resolu_us,  //us
        .irq_cb = mouse_timer_handler, //设置中断回调函数
        .irq_priority = 4,             //中断优先级
        .mode = GPTIMER_MODE_TIMER,    //设置工作模式
        .private_data = NULL,
    };

    mouse_info.mouse_gtimer_tid = gptimer_init(MOUSE_GTIMER_ID, &mouse_timer_config);
    gptimer_start(mouse_info.mouse_gtimer_tid);
    log_info("mouse gtimer_config tid = %d\n", mouse_info.mouse_gtimer_tid);
}

/*************************************************************************************************/
/*
 *  \brief      mouse 关闭gtimer
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_send_data_timer_deinit(void)
{
    if (mouse_info.mouse_gtimer_tid) {
        gptimer_deinit(mouse_info.mouse_gtimer_tid);
    }
    log_info("mouse gtimer_config deinit");
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
static void mouse_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("-----------------------mouse_bt_connction_status_event_handler %d", bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        //  蓝牙初始化完成
        log_info("BT_STATUS_INIT_OK\n");
        mouse_board_devices_init();
        bt_set_local_name((char *)mouse_ble_name, strlen(mouse_ble_name));
        btstack_ble_start_after_init(0);
        mouse_send_data_timer_init(MOUSE_BLE_GTIMER_INIT_TIME);
        // 开启more data
        set_config_vendor_le_bb(0);

#if (TEST_MOUSE_SIMULATION_ENABLE == 0)
        if (!le_hogp_get_is_paired()) {
            ble_module_enable(0);
        }
#endif

        break;

    default: {
        bt_comm_ble_status_event_handler(bt);
    }
    break;
    }
}

/*************************************************************************************************/
/*!
 *  \brief      设置MOUSE连接状态
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_set_is_paired(uint8_t is_paired)
{
    mouse_info.mouse_is_paired = is_paired;
}

void mouse_set_is_paired_cb(void *priv)
{
    uint8_t is_paired = *(uint8_t *)priv;
    mouse_set_is_paired(is_paired);
}
/*************************************************************************************************/
/*!
 *  \brief      获取MOUSE状态信息
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
mouse_info_t *mouse_get_status_info(void)
{
    return &mouse_info;
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
static void mouse_hogp_ble_status_callback(ble_state_e status, uint32_t value)
{
    log_info("mouse_hogp_ble_status_callback============== %02x   value:0x%x\n", status, value);
    switch (status) {
    case BLE_ST_CONNECT:
        log_info("BLE_ST_CONNECT\n");
        break;

    case BLE_PRIV_MSG_PAIR_CONFIRM:
        log_info("BLE_PRIV_MSG_PAIR_CONFIRM\n");
        break;

    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:
        // 存在配对信息时回连才发数
        if (le_hogp_get_is_paired()) {
            mouse_set_is_paired(1);
        }
        log_info("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        break;

    case BLE_ST_DISCONN:
        mouse_set_is_paired(0);
        log_info("BLE_ST_DISCONNL\n");

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
#if TEST_MOUSE_SIMULATION_ENABLE
        // 测试模式不能立即发数，否则会报超时断开
        sys_timeout_add((void *)1, mouse_set_is_paired_cb, 500);
#else
        if (!le_hogp_get_is_paired()) {
            mouse_set_is_paired(1);
        }
#endif
        // update gptimer period from connect inteval
        log_info("BLE_ST_CONNECTION_UPDATE_OK, INTERVAL:%d\n", value);
        gptimer_set_timer_period(mouse_info.mouse_gtimer_tid, value * 1000);
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
static int mouse_bt_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE \n");
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        mouse_auto_shutdown_disable();
        break;

    default:
        break;

    }
    return 0;
}

static void mouse_handle_custom_key_event(uint8_t event_value, uint8_t event_type)
{
#if CUSTOM_KEY_ENABLE
    uint16_t hid_packet;

    // 检查是否为特定的按键值，并在按键抬起事件时处理
    if (event_type == KEY_EVENT_UP && (event_value == TCFG_ADKEY_VALUE6 || event_value == TCFG_ADKEY_VALUE7)) {
        switch (event_value) {
        case TCFG_ADKEY_VALUE7:
            hid_packet = CONSUMER_PLAY_PAUSE;
            log_info(">>>>> send custom: play/pause");
            break;

        case TCFG_ADKEY_VALUE6:
            hid_packet = CONSUMER_MUTE;
            log_info(">>>>> send custom: mute");
            break;

        default:
            return;
        }

        ble_hid_data_send(CUSTOM_SEND_DATA_REPORT_ID, (uint8_t *)&hid_packet, 2);
    }
#endif
}

static void mouse_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        log_info("key_value = %d.\tevent_type = %d.\n", event->u.key.value, event->u.key.event);

        uint8_t event_type = event->u.key.event;

        // cpi选择
        if (event->u.key.value == KEY_CPI_VAL && event_type == KEY_EVENT_LONG) {
            mouse_cpi_mode++;
            if (mouse_cpi_mode >= (sizeof(mouse_cpi_value_table) / sizeof(uint16_t))) {
                mouse_cpi_mode = 0;
            }
#ifdef TCFG_OMSENSOR_ENABLE
            optical_mouse_sensor_set_cpi(mouse_cpi_value_table[mouse_cpi_mode]);
#endif
            return;
        }

        // left+right键, 长按数秒开广播
#if (DOUBLE_KEY_HOLD_PAIR)
        if (KEY_LK_RK_VAL == event->u.key.value && event_type == KEY_EVENT_LONG) {
            log_info("adv start:key3 hold:%d", mouse_double_key_long_cnt);
            if (++mouse_double_key_long_cnt >= DOUBLE_KEY_HOLD_CNT) {
                if (!ble_hid_is_connected()) {
                    le_hogp_set_pair_allow();
                    ble_module_enable(1);
                }

                mouse_double_key_long_cnt = 0;
            }
            return;
        } else {
            mouse_double_key_long_cnt = 0;
        }
#endif

        // 自定义hid控制按键
        mouse_handle_custom_key_event(event->u.key.value, event_type);

        mouse_send_packet.buttonMask = 0;
        if (event_type == KEY_EVENT_CLICK || \
            event_type == KEY_EVENT_LONG || \
            event_type == KEY_EVENT_HOLD) {
            mouse_send_packet.buttonMask |= event->u.key.value;
        }
    }

    if (mouse_flag.button_send_flag) {
        mouse_flag.button_send_flag = 0;
    }
}


static int mouse_event_handler(struct application *app, struct sys_event *event)
{
#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //重置无操作定时计数
    if (event->type != SYS_DEVICE_EVENT || DEVICE_EVENT_FROM_POWER != (int)event->arg) { //过滤电源消息
        sys_timer_modify(mouse_info.mouse_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
#endif

    switch (event->type) {
    case SYS_KEY_EVENT:
        /* log_info("Sys Key : %s", event->arg); */
        mouse_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            mouse_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            mouse_bt_hci_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            mouse_hogp_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return mouse_bt_common_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if (!(strcmp(event->arg, "code_switch"))) {
            mouse_code_sw_event_handler(event);
        } else if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, mouse_set_soft_poweroff);
        }
        return 0;

    default:
        return FALSE;
    }

    return FALSE;
}

//system check go sleep is ok
static uint8_t mouse_hid_idle_query(void)
{
    return !mouse_is_active;
}

REGISTER_LP_TARGET(app_hid_lp_target) = {
    .name = "mouse_hid_deal",
    .is_idle = mouse_hid_idle_query,
};

static const struct application_operation app_mouse_ops = {
    .state_machine  = mouse_state_machine,
    .event_handler 	= mouse_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_mouse) = {
    .name 	= "mouse_single",
    .action	= ACTION_MOUSE_MAIN,
    .ops 	= &app_mouse_ops,
    .state  = APP_STA_DESTROY,
};

//=====================================MOUSE TEST==================================================//
#if TEST_MOUSE_SIMULATION_ENABLE
/*************************************************************************************************/
/*!
 *  \brief  mouse测试函数
 *
 *  \param
 *
 *  \return
 *
 *  \note   持续控制鼠标画一个矩形
 */
/*************************************************************************************************/
static void mouse_send_data_test(void)
{
    static int16_t deltaX, deltaY;
    static uint8_t dcount = 100;

    dcount++;
    if (dcount >= 100) {
        dcount = 0;
        if ((deltaX == 0) && (deltaY == 4)) {
            deltaX = 4;
            deltaY = 0;
        } else if ((deltaX == 4) && (deltaY == 0)) {
            deltaX = 0;
            deltaY = -4;
        } else if ((deltaX == 0) && (deltaY == -4)) {
            deltaX = -4;
            deltaY = 0;
        } else if ((deltaX == -4) && (deltaY == 0)) {
            deltaX = 0;
            deltaY = 4;
        } else {
            deltaX = 4;
            deltaY = 0;
        }
    }
    mouse_optical_sensor_event(0, deltaX, deltaY);
}
//=====================================MOUSE TEST==================================================//
#endif


#endif
