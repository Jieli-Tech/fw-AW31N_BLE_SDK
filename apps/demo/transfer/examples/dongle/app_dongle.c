#include "msg.h"
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
#include "usb/device/hid.h"
#include "usb/device/cdc.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "app_comm_proc.h"
#include "app_power_mg.h"
#include "app_dongle.h"
#include "user_cfg.h"
#include "ota_dg_central.h"
#include "usb_suspend_resume.h"
#include "usb_stack.h"
#include "ble_dg_central.h"

#define LOG_TAG_CONST       DONGLE
#define LOG_TAG             "[DONGLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

#if (CONFIG_APP_DONGLE)

#if USER_SUPPORT_PROFILE_HID && USER_SUPPORT_PROFILE_SPP
//不支持同时打开
#error " not support double profile!!!!!!"
#endif

static uint8_t app_dongle_is_active = 0;
//---------------------------------------------------------------------
static void dongle_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    app_dongle_is_active = 1;
#endif
    //必须先主动断开蓝牙链路,否则要等链路超时断开

#if (USER_SUPPORT_PROFILE_HID==1)
    user_hid_exit();
#endif
    btstack_ble_exit(0);

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

static void dongle_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(NULL, 0);
#if CFG_RF_24G_CODE_ID
    rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
#endif
    cfg_file_parse(0);
    btstack_init();
}

static void dongle_usb_start()
{
#if (TCFG_PC_ENABLE)
    //配置选择上报PC的描述符
    //first device
    log_info("register channel 1");
#if CONFIG_HIDKEY_REPORT_TEST & BIT(0)
    usb_hid_mouse_set_report_map(sHIDReportDesc_hidkey, sizeof(sHIDReportDesc_hidkey));
#else
    usb_hid_mouse_set_report_map(sHIDReportDesc_mouse, sizeof(sHIDReportDesc_mouse));
#endif

#if (CONFIG_BT_GATT_CLIENT_NUM == 2)
    //second device
    log_info("register channel 2");
#if CONFIG_HIDKEY_REPORT_TEST & BIT(1)
    usb_hid_set_second_repport_map(sHIDReportDesc_hidkey, sizeof(sHIDReportDesc_hidkey));
#else
    usb_hid_set_second_repport_map(sHIDReportDesc_stand_keyboard2, sizeof(sHIDReportDesc_stand_keyboard2));
#endif
#endif

#if TCFG_OTG_USB_DEV_EN
    const struct otg_dev_data otg_data = {
        .usb_dev_en = TCFG_OTG_USB_DEV_EN,
        .slave_online_cnt = TCFG_OTG_SLAVE_ONLINE_CNT,
        .slave_offline_cnt = TCFG_OTG_SLAVE_OFFLINE_CNT,
        .host_online_cnt = TCFG_OTG_HOST_ONLINE_CNT,
        .host_offline_cnt = TCFG_OTG_HOST_OFFLINE_CNT,
        .detect_mode = TCFG_OTG_MODE,
        .detect_time_interval = TCFG_OTG_DET_INTERVAL,
        .usb_otg_sof_check_init = usb_otg_sof_check_init,
    };
    usb_otg_init(NULL, (void *)&otg_data);
#else
    usb_start();
#endif
    usb_slave_set_status_hander(dg_central_usb_status_handler);
#if RCSP_BTMATE_EN
    dongle_ota_init();
    //usb TODO
    custom_hid_set_rx_hook(NULL, dongle_custom_hid_rx_handler);//重注册接收回调到dongle端
    /* download_buf = malloc(1024); */
    /* dongle_return_online_list(); */
    sys_timeout_add(NULL, dongle_return_online_list, 4000);
#endif
#endif

}

static void dongle_app_start()
{
    log_info("=======================================\n");
    log_info("---------usb + dongle start demo-------\n");
    log_info("=======================================\n");
    log_info("app_file: %s", __FILE__);

    clock_bt_init();
    dongle_bt_start();
    dongle_usb_start();

    int msg[4] = {0};
    while (1) {
        get_msg(sizeof(msg) / sizeof(int), msg);
        app_comm_process_handler(msg);
    }
}

static int dongle_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_DONGLE_MAIN:
            dongle_app_start();
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

//ble 接收设备数据
int dongle_ble_hid_input_handler(uint8_t *packet, uint16_t size)
{
    /* log_info("ble_hid_data_input:size=%d", size); */
    /* log_info_hexdump(packet, size); */

    /* rf_unpack_data(packet, size); */
#if (TCFG_PC_ENABLE)
    // 1ms高回报率时不开putchar,否则会影响收数
    /* putchar('&'); */
    return usb_hid_mouse_send_data(packet, size);
#else
    log_info("chl1 disable!!!\n");
    return 0;
#endif
}

//ble 接收第二个设备数据
int dongle_second_ble_hid_input_handler(uint8_t *packet, uint16_t size)
{
    /* log_info("ble_hid_data_input:size=%d", size); */
    /* put_buf(packet, size); */

    /* putchar('#'); */
#if (TCFG_PC_ENABLE) && (CONFIG_BT_GATT_CLIENT_NUM == 2)
    return hid_send_second_data(packet, size);
#else
    log_info("chl2 disable!!!\n");
    return 0;
#endif
}

static int dongle_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);
    bt_comm_ble_hci_event_handler(bt);
    return 0;
}

static int dongle_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);
    bt_comm_ble_status_event_handler(bt);
    return 0;
}

static void dongle_key_event_handler(struct sys_event *event)
{
    uint16_t event_type;
    uint32_t key_value;

    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);

        if (event_type == KEY_EVENT_TRIPLE_CLICK
            && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
            /* dongle_power_event_to_user(POWER_EVENT_POWER_SOFTOFF); */
            /* dongle_set_soft_poweroff(); */
            return;
        }

        if (event_type == KEY_EVENT_DOUBLE_CLICK && key_value == TCFG_ADKEY_VALUE0) {
            dg_central_clear_pair();
            return;
        }

#if (USER_SUPPORT_PROFILE_HID ==1)
        if (event_type == KEY_EVENT_LONG && key_value == TCFG_ADKEY_VALUE0) {
            log_info("hid disconnec test");
            user_hid_disconnect();
        }
#endif

#if (TCFG_PC_ENABLE)  && CONFIG_HIDKEY_REPORT_TEST
        if (event_type == KEY_EVENT_CLICK && key_value == TCFG_ADKEY_VALUE0) {
            uint8_t packet_buf[3] = {HIDKEY_REPORT_ID, CONSUMER_PLAY_PAUSE & 0x0ff, CONSUMER_PLAY_PAUSE >> 8}; //pp key
            log_info("key_00");
#if CONFIG_HIDKEY_REPORT_TEST & BIT(0)
            log_info(">>>>>>>usb test play/pause");
            usb_hid_mouse_send_data(packet_buf, sizeof(packet_buf));
            os_time_dly(1);
            packet_buf[1] = 0;
            usb_hid_mouse_send_data(packet_buf, sizeof(packet_buf));
#endif
        }

        if (event_type == KEY_EVENT_CLICK && key_value == TCFG_ADKEY_VALUE1) {
            uint8_t packet_buf[3] = {HIDKEY_REPORT_ID, CONSUMER_MUTE & 0x0ff, CONSUMER_MUTE >> 8}; //pp key
            log_info("key_01");
#if (CONFIG_HIDKEY_REPORT_TEST & BIT(0))
            log_info(">>>>>>>>>usb test mute");
            usb_hid_mouse_send_data(packet_buf, sizeof(packet_buf));
            os_time_dly(1);
            packet_buf[1] = 0;
            usb_hid_mouse_send_data(packet_buf, sizeof(packet_buf));
#endif
        }
#endif

    }
}

static int dongle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        dongle_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            dongle_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            dongle_bt_hci_event_handler(&event->u.bt);
#if RCSP_BTMATE_EN
        } else if ((uint32_t)event->arg == DEVICE_EVENT_FROM_PC) {
            dongle_pc_event_handler(&event->u.bt);//dongle pc命令回调处理
        } else if ((uint32_t)event->arg == DEVICE_EVENT_FROM_OTG) {
            dongle_otg_event_handler(&event->u.bt);//dongle ota升级数据透传
#endif
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            /* return app_power_event_handler(&event->u.dev, dongle_set_soft_poweroff); */
        }

#if TCFG_CHARGE_ENABLE
        else if ((uint32_t)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
#endif
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_dongle_ops = {
    .state_machine  = dongle_state_machine,
    .event_handler 	= dongle_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_dongle) = {
    .name 	= "dongle",
    .action	= ACTION_DONGLE_MAIN,
    .ops 	= &app_dongle_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static uint8_t dongle_state_idle_query(void)
{
    return !app_dongle_is_active;
}

REGISTER_LP_TARGET(dongle_state_lp_target) = {
    .name = "dongle_state_deal",
    .is_idle = dongle_state_idle_query,
};

#endif


