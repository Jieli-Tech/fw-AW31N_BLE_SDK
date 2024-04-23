/*********************************************************************************************
 *   Filename        : app_mouse.c

 *   Description     :鼠标单模切换

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
#include "standard_hid.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "OMSensor_manage.h"
#include "gptimer.h"
#include "user_cfg.h"
#include "app_mouse.h"

#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif
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

static volatile uint8_t mouse_is_active;// 1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static uint8_t mouse_double_key_long_cnt; // key cnt
static uint8_t mouse_cpi_mode = MOUSE_CPI_1000; // mouse cpi setting, default 1000

static mouse_info_t mouse_info;
static mouse_send_flags_t mouse_flag;

static volatile mouse_packet_data_t mouse_first_packet = {0};
static volatile mouse_packet_data_t mouse_second_packet = {0};

static const ble_init_cfg_t mouse_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_MOUSE,
    .report_map = mouse_report_map,
    .report_map_size = sizeof(mouse_report_map),
};

static void mouse_select_btmode(uint8_t mode);
static void mouse_vm_deal(uint8_t rw_flag);
static void mouse_optical_sensor_event_handler(struct sys_event *event);
static void mouse_test_ctrl(void);

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
 *  \brief     鼠标发数函数
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_timer_handler(int priv)
{
    if (!mouse_info.mouse_is_paired) {
        return;
    }

    if (mouse_flag.button_send_flag == 0 || mouse_flag.wheel_send_flag == 0) {
        log_info_hexdump((uint8_t *)&mouse_first_packet.data, sizeof(mouse_first_packet.data));
        ble_hid_data_send(1, (uint8_t *)&mouse_first_packet.data, sizeof(mouse_first_packet.data));

        if (mouse_flag.button_send_flag == 0) {
            mouse_flag.button_send_flag = 1;
        }

        if (mouse_flag.wheel_send_flag == 0) {
            mouse_flag.wheel_send_flag = 1;
            mouse_first_packet.data[WHEEL_IDX] = 0;
        }
    }

    if (mouse_flag.sensor_send_flag == 0) {
        /* log_info_hexdump((uint8_t *)&mouse_second_packet.data, sizeof(mouse_second_packet.data)); */
        ble_hid_data_send(2, (uint8_t *)&mouse_second_packet.data, sizeof(mouse_second_packet.data));
        memset((void *)&mouse_second_packet.data, 0, sizeof(mouse_second_packet.data));
        mouse_flag.sensor_send_flag = 1;
    }
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
    p33_soft_reset();
    while (1);
}

/*************************************************************************************************/
/*!
 *  \brief     根据配置好的模式切换
 *
 *  \param      [in]void
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_mode_switch_handler(void)
{
#if SWITCH_MODE_24G_ENABLE || SWITCH_MODE_BLE_ENABLE
    log_info("mode_switch_handler");
    int switch_success = 0;

    mouse_is_active = 1;

    if (HID_MODE_BLE == mouse_info.mouse_hid_mode) {
#if SWITCH_MODE_24G_ENABLE
        switch_success = 1;
        mouse_select_btmode(HID_MODE_24G);
#else
        log_info("No other mode available to switch to\n");
#endif
    } else if (HID_MODE_24G == mouse_info.mouse_hid_mode) {
#if SWITCH_MODE_BLE_ENABLE
        switch_success = 1;
        mouse_select_btmode(HID_MODE_BLE);
#else
        log_info("No other mode available to switch to\n");
#endif
    }

    if (switch_success) {
        log_info("Switch success\n");
        sys_timeout_add(NULL, mouse_set_soft_reset, WAIT_DISCONN_TIME_MS);
    } else {
        log_info("Switch fail\n");
        mouse_is_active = 0;
    }
#endif
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
        mouse_first_packet.data[WHEEL_IDX] = -sw_val;
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

        mouse_second_packet.data[SENSOR_XLSB_IDX] = delta_x & 0xFF;
        mouse_second_packet.data[SENSOR_YLSB_XMSB_IDX] = ((delta_y << 4) & 0xF0) | ((delta_x >> 8) & 0x0F);
        mouse_second_packet.data[SENSOR_YMSB_IDX] = (delta_y >> 4) & 0xFF;

        /* log_info("x = %d.\ty = %d.\n", event->u.axis.x, event->u.axis.y); */
    }
    mouse_flag.sensor_send_flag = 0;
}

/*************************************************************************************************/
/*!
 *  \brief      绑定信息 VM读写操作
 *
 *  \param      [in]rw_flag: 0-read vm,1--write
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_vm_deal(uint8_t rw_flag)
{
    hid_vm_cfg_t info;
    int ret;
    int vm_len = sizeof(hid_vm_cfg_t);

    log_info("-hid_info vm_do:%d\n", rw_flag);
    memset(&info, 0, vm_len);

    if (rw_flag == 0) {
        // 默认设置
        mouse_info.mouse_hid_mode = HID_MODE_NULL;
        ret = syscfg_read(CFG_AAP_MODE_INFO, (uint8_t *)&info, vm_len);

        if (!ret) {
            log_info("-null--\n");
        } else {
            if (HID_VM_HEAD_TAG == info.head_tag) {
                log_info("-exist--\n");
                log_info_hexdump((uint8_t *)&info, vm_len);
                mouse_info.mouse_hid_mode = info.mode;
            }
        }

        if (HID_MODE_NULL == mouse_info.mouse_hid_mode) {
            /* 第一次上电，默认模式优先顺序 */
#if SWITCH_MODE_BLE_ENABLE
            mouse_info.mouse_hid_mode = HID_MODE_BLE;
#else
            mouse_info.mouse_hid_mode = HID_MODE_24G;
#endif
        } else {
            /* 修改模式后，判断VM记录的模式是否还存在 */
            if (0 == SWITCH_MODE_24G_ENABLE && mouse_info.mouse_hid_mode == HID_MODE_24G) {
                mouse_info.mouse_hid_mode = HID_MODE_BLE;
            }

            if (0 == SWITCH_MODE_BLE_ENABLE && mouse_info.mouse_hid_mode == HID_MODE_BLE) {
                mouse_info.mouse_hid_mode = HID_MODE_24G;
            }
        }

        if (mouse_info.mouse_hid_mode != info.mode) {
            log_info("-write00,mdy mode--\n");
            info.mode = mouse_info.mouse_hid_mode;
            syscfg_write(CFG_AAP_MODE_INFO, (uint8_t *)&info, vm_len);
        }
    } else {
        // 写入模式
        info.mode = mouse_info.mouse_hid_mode;
        info.head_tag = HID_VM_HEAD_TAG;
        info.res = 0;

        hid_vm_cfg_t tmp_info;
        syscfg_read(CFG_AAP_MODE_INFO, (uint8_t *)&tmp_info, vm_len);

        if (memcmp(&tmp_info, &info, vm_len)) {
            syscfg_write(CFG_AAP_MODE_INFO, (uint8_t *)&info, vm_len);
            log_info("-write11,new mode--\n");
            log_info_hexdump((uint8_t *)&info, vm_len);
        }
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
static void mouse_set_soft_poweroff()
{
    log_info("mouse_set_soft_poweroff\n");
    mouse_is_active = 1;
    //必须先主动断开蓝牙链路,否则要等链路超时断开
    btstack_ble_exit(0);

    if (ble_hid_is_connected()) {
        //延时，确保BT退出链路断开
        sys_timeout_add(NULL, app_power_set_soft_poweroff, WAIT_DISCONN_TIME_MS);
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

#if TCFG_NORMAL_SET_DUT_MODE
    user_sele_dut_mode(1);
#else
    btstack_ble_start_before_init(&mouse_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_IND, 5000);
#endif

    cfg_file_parse(0);
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

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
#endif

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //无操作定时软关机
    mouse_info.mouse_auto_shutdown_timer = sys_timeout_add(NULL, mouse_set_soft_poweroff, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
#endif

    int msg[2] = {0};
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
 *  \brief      mouse发送定时器，使用gtimer
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_send_data_timer_init()
{
    struct gptimer_config mouse_timer_config;
    mouse_timer_config.resolution_us = MOUSE_GTIMER_INIT_TIME;
    mouse_timer_config.irq_cb = mouse_timer_handler;
    mouse_timer_config.tid = MOUSE_GTIMER_ID;
    mouse_timer_config.irq_priority = 6; // 设置最高中断优先级
    mouse_info.mouse_gtimer_tid = gptimer_init(&mouse_timer_config);
    gptimer_start(mouse_info.mouse_gtimer_tid);
    log_info("mouse gtimer_config tid = %d\n", mouse_info.mouse_gtimer_tid);
}

/*************************************************************************************************/
/*!
 *  \brief      mouse bt 模式初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_btmode_init()
{
    // read vm first
    mouse_vm_deal(0);

#if DOUBLE_KEY_HOLD_PAIR
    // 2.4g&ble 配对管理，默认绑定配对设备
    le_hogp_set_pair_config(10, 1);
#endif
    log_info("##config_24g_code: %04x", CFG_RF_24G_CODE_ID);
    ble_set_fix_pwr(9);
    if (mouse_info.mouse_hid_mode == HID_MODE_BLE || mouse_info.mouse_hid_mode == HID_MODE_24G) {
        if (mouse_info.mouse_hid_mode == HID_MODE_BLE) {
            rf_set_24g_hackable_coded(0);
        } else {
            log_info("##init_24g_code: %04x", CFG_RF_24G_CODE_ID);
            rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
        }
        btstack_ble_start_after_init(0);
    }

    mouse_select_btmode(HID_MODE_INIT);

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
        mouse_btmode_init();
        mouse_send_data_timer_init();
        break;

    default: {
        bt_comm_ble_status_event_handler(bt);
    }
    break;
    }
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
        log_info("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        break;

    case BLE_ST_DISCONN:
        mouse_info.mouse_is_paired = 0;
        log_info("BLE_ST_DISCONNL\n");
        break;

    case BLE_ST_NOTIFY_IDICATE:
        log_info("BLE_ST_NOTIFY_IDICATE\n");
        break;

    case BLE_ST_CONNECT_UPDATE_REQUEST:
        mouse_info.mouse_is_paired = 1;
        log_info("BLE_ST_CONNECT_UPDATE_REQUEST\n");
        break;

    case BLE_ST_CONNECTION_UPDATE_OK:
        // update gptimer period from connect inteval
        log_info("BLE_ST_CONNECTION_UPDATE_OK, INTERVAL:%d\n", value);
        gptimer_set_resolution(mouse_info.mouse_gtimer_tid, value * 1000);
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

/*************************************************************************************************/
/*!
 *  \brief      清配对表，重新可以连接可发现
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_bt_pair_start(void)
{
    if (mouse_info.mouse_hid_mode == HID_MODE_BLE || mouse_info.mouse_hid_mode == HID_MODE_24G) {
        if (ble_hid_is_connected()) {
            log_info("disconnect ble\n");
            ble_module_enable(0);
        } else {
            ble_module_enable(0);
        }

        if (mouse_info.mouse_hid_mode == HID_MODE_24G) {
            log_info("#2.4g enter wait pair....");
        } else {
            log_info("#ble enter wait pair....");
        }
        /* le_hogp_set_pair_allow(); */
        ble_module_enable(1);
    } else {
        log_info("Unsupported HID mode");
    }
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

        // left+right键, 长按数秒进入
#if (DOUBLE_KEY_HOLD_PAIR)
        if (3 == event->u.key.value && event_type == KEY_EVENT_LONG) {
            log_info("pair_start:double key6 hold:%d", mouse_double_key_long_cnt);
            if (++mouse_double_key_long_cnt >= DOUBLE_KEY_HOLD_CNT) {
                /* mouse_bt_pair_start(); */
                if (!ble_hid_is_connected()) {
                    ble_module_enable(1);
                }

                mouse_double_key_long_cnt = 0;
#if TEST_MOUSE_SIMULATION_ENABLE
                mouse_test_ctrl();
#endif
            }
            return;
        } else {
            mouse_double_key_long_cnt = 0;
        }
#endif

        mouse_first_packet.data[BUTTONS_IDX] = 0;
        if (event_type == KEY_EVENT_CLICK || \
            event_type == KEY_EVENT_LONG || \
            event_type == KEY_EVENT_HOLD) {
            mouse_first_packet.data[BUTTONS_IDX] |= event->u.key.value;
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
    if (event->type != SYS_DEVICE_EVENT || DEVICE_EVENT_FROM_POWER != event->arg) { //过滤电源消息
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

static void mouse_select_btmode(uint8_t mode)
{
    if (mode != HID_MODE_INIT) {
        if (mouse_info.mouse_hid_mode == mode) {
            return;
        }
        mouse_info.mouse_hid_mode = mode;
    }

    log_info("###### %s: %d,%d\n", __FUNCTION__, mode, mouse_info.mouse_hid_mode);

    if (mouse_info.mouse_hid_mode == HID_MODE_BLE || mouse_info.mouse_hid_mode == HID_MODE_24G) {

        if (mouse_info.mouse_hid_mode == HID_MODE_BLE) {
            log_info("---------app select ble--------\n");
            rf_set_24g_hackable_coded(0);
        } else {
            log_info("---------app select 24g--------\n");
            log_info("set_24g_code: %04x", CFG_RF_24G_CODE_ID);
            rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
        }

        if (!STACK_MODULES_IS_SUPPORT(BT_BTSTACK_LE) || !BT_MODULES_IS_SUPPORT(BT_MODULE_LE)) {
            log_info("not surpport ble,make sure config !!!\n");
            ASSERT(0);
        }

        ble_module_enable(0);

    }

    mouse_vm_deal(1);
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
#if TCFG_OMSENSOR_ENABLE || TCFG_CODE_SWITCH_ENABLE
#error "please disable this enable!!!!"
#endif

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

/*************************************************************************************************/
/*!
 *  \brief  mouse测试控制
 *
 *  \param
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void mouse_test_ctrl(void)
{
    static uint16_t loop = 0;
    if (loop) {
        sys_s_hi_timer_del(loop);
        loop = 0;
        mouse_is_active = 0;
    } else {
        mouse_is_active = 1;
        loop = sys_s_hi_timer_add(NULL, mouse_send_data_test, TEST_SET_TIMER_VALUE);
    }
}
#endif
//======================================================================================================//

#endif

