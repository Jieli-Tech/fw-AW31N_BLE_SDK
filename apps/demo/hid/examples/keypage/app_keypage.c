/*********************************************************************************************
    *   Filename        : app_keypage.c

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
#include "app_modules.h"
#include "app_comm_proc.h"
#include "ble_hogp.h"
#include "user_cfg.h"
#include "app_keypage.h"

#if(CONFIG_APP_KEYPAGE)

#define LOG_TAG_CONST       KEYPAGE
#define LOG_TAG             "[KEYPAGE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "log.h"

#define  REMOTE_IS_IOS()           (keypage_remote_type == REMOTE_DEV_IOS)
#define  PACKET_DELAY_TIME()        mdelay(30)

static uint8_t keypage_remote_type = REMOTE_DEV_UNKNOWN;
//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗
static volatile uint8_t keypage_is_active;
static uint16_t keypage_auto_shutdown_timer;
static LabCoordinates keypage_lab_value;

static const ble_init_cfg_t keypage_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_KEYBOARD,
    .report_map = keypage_report_map,
    .report_map_size = sizeof(keypage_report_map),
};

static void keypage_coordinate_vm_deal(uint8_t flag);
static void keypage_set_soft_poweroff(void);
static int  keypage_send_packect(const uint8_t *packet_table, uint16_t table_size, uint8_t packet_len);

/*************************************************************************************************/
/*!
 *  \brief      音量控制
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void keypage_vol_ctrl(uint8_t add_dec)
{
    log_info("%s[%d]", __func__, add_dec);
    if (add_dec) {
        keypage_send_packect((uint8_t *)key_vol_add, sizeof(key_vol_add), REPROT_CONSUMER_LEN);
    } else {
        keypage_send_packect((uint8_t *)key_vol_dec, sizeof(key_vol_dec), REPROT_CONSUMER_LEN);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      恢复原点
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void keypage_connect_reset_xy(void *priv)
{
    log_info("%s", __func__);
    if (!ble_hid_is_connected()) {
        return;
    }

    if (REMOTE_IS_IOS()) {
        keypage_send_packect((uint8_t *)key_connect_before, sizeof(key_connect_before), REPROT_INFO_LEN0);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      发送包数据
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int keypage_send_packect(const uint8_t *packet_table, uint16_t table_size, uint8_t packet_len)
{
    int i;
    const uint8_t *pt = packet_table;

    log_info("remote_type:%d", keypage_remote_type);
    log_info("send_packet:%08x,size=%d,len=%d", packet_table, table_size, packet_len);

    for (i = 0; i < table_size; i += packet_len) {
        put_buf((uint8_t *)pt, packet_len);
        ble_hid_data_send(pt[0], (uint8_t *)&pt[1], packet_len - 1);
        pt += packet_len;
        PACKET_DELAY_TIME();
    }
    return 0;
}

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
static void keypage_auto_shutdown_disable(void)
{
    if (keypage_auto_shutdown_timer) {
        sys_timeout_del(keypage_auto_shutdown_timer);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      调整IOS设备坐标
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keypage_coordinate_equal_ios(uint8_t x, uint8_t y)
{
    if ((x == 0) && (y == 0)) { /// 复位原点
        keypage_connect_reset_xy(NULL);
        return;
    }
    uint16_t ios_x = 0, ios_y = 0;
    uint8_t *pos = key_focus_ios[1];
    if (x == 1) {
        ios_x = MOUSE_STEP;
    }
    if (x == 2) {
        ios_x = 0x0FFF - MOUSE_STEP;
    }
    if (y == 1) {
        ios_y = MOUSE_STEP;
    }
    if (y == 2) {
        ios_y = 0x0FFF - MOUSE_STEP;
    }
    pos[1] = (ios_x & 0xFF);
    pos[2] = ((ios_y & 0x0F) << 4) | (ios_x >> 8 & 0x0F);
    pos[3] = (ios_y >> 4 & 0xFF);
    put_buf(pos, REPROT_INFO_LEN0);
    if (REMOTE_IS_IOS()) {
        keypage_send_packect((uint8_t *)key_focus_ios, sizeof(key_focus_ios), REPROT_INFO_LEN0);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      调整设备坐标
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keypage_coordinate_equal(void)
{
    if (REMOTE_IS_IOS()) {
        return;
    }
    keypage_lab_value.x_lab_low = keypage_lab_value.x_lab & 0xff;
    keypage_lab_value.x_lab_hig = keypage_lab_value.x_lab >> 8;
    keypage_lab_value.y_lab_low = keypage_lab_value.y_lab & 0xff;
    keypage_lab_value.y_lab_hig = keypage_lab_value.y_lab >> 8;
    key_pp_press[0][3] = keypage_lab_value.x_lab_low;
    key_pp_press[0][4] = keypage_lab_value.x_lab_hig;
    key_pp_press[0][5] = keypage_lab_value.y_lab_low;
    key_pp_press[0][6] = keypage_lab_value.y_lab_hig;
    key_pp_release[0][3] = keypage_lab_value.x_lab_low;
    key_pp_release[0][4] = keypage_lab_value.x_lab_hig;
    key_pp_release[0][5] = keypage_lab_value.y_lab_low;
    key_pp_release[0][6] = keypage_lab_value.y_lab_hig;
    put_buf((uint8_t *)key_pp_press, sizeof(key_pp_press));
    keypage_send_packect((uint8_t *)key_pp_press, sizeof(key_pp_press), REPROT_INFO_LEN1);
    keypage_send_packect((uint8_t *)key_pp_release, sizeof(key_pp_release), REPROT_INFO_LEN1);
}

/*************************************************************************************************/
/*!
 *  \brief      按键数据处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keypage_key_operate(key_op op)
{
    switch (op) {
    case KEY_UP:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_up_before, sizeof(key_up_before), REPROT_INFO_LEN0);
        } else {
            keypage_send_packect((uint8_t *)key_up_click, sizeof(key_up_click), REPROT_INFO_LEN1);
        }
        break;

    case KEY_DOWN:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_down_before, sizeof(key_down_before), REPROT_INFO_LEN0);
        } else {
            keypage_send_packect((uint8_t *)key_down_click, sizeof(key_down_click), REPROT_INFO_LEN1);
        }
        break;

    case KEY_LEFT:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_left_before, sizeof(key_left_before), REPROT_INFO_LEN0);
        } else {
            keypage_send_packect((uint8_t *)key_left_click, sizeof(key_left_click), REPROT_INFO_LEN1);
        }
        break;

    case KEY_RIGHT:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_right_before, sizeof(key_right_before), REPROT_INFO_LEN0);
        } else {
            keypage_send_packect((uint8_t *)key_right_click, sizeof(key_right_click), REPROT_INFO_LEN1);
        }
        break;

    case KEY_PP_PRESS:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_pp_release_before, sizeof(key_pp_release_before), REPROT_INFO_LEN0);
            keypage_connect_reset_xy(NULL);
        } else {
            keypage_send_packect((uint8_t *)key_pp_press, sizeof(key_pp_press), REPROT_INFO_LEN1);
            keypage_send_packect((uint8_t *)key_pp_release, sizeof(key_pp_release), REPROT_INFO_LEN1);
        }
        break;

    case KEY_ONE_PRESS:
        if (REMOTE_IS_IOS()) {
            keypage_send_packect((uint8_t *)key_one_press_before, sizeof(key_one_press_before), REPROT_INFO_LEN0);
            keypage_send_packect((uint8_t *)key_one_release_before, sizeof(key_one_release_before), REPROT_INFO_LEN0);
        } else {
            keypage_send_packect((uint8_t *)key_one_press, sizeof(key_one_press), REPROT_INFO_LEN1);
            keypage_send_packect((uint8_t *)key_one_release, sizeof(key_one_release), REPROT_INFO_LEN1);
        }
        break;

    default:
        log_info("Error key op!");
        break;
    }
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
static void keypage_app_key_deal_test(uint8_t key_type, uint8_t key_value)
{
    uint16_t key_msg = 0;
    log_info("app_key_event: %d,%d\n", key_type, key_value);

#if TCFG_LED_ENABLE
    led_operate(LED_KEY_UP);
#endif

    if (key_type == KEY_EVENT_CLICK) {
        switch (key_value) {
        case 0:
            keypage_key_operate(KEY_UP);
            break;

        case 1:
            keypage_key_operate(KEY_DOWN);
            break;

        case 2:
            keypage_key_operate(KEY_LEFT);
            break;

        case 3:
            keypage_key_operate(KEY_RIGHT);
            break;

        case 4:
            keypage_key_operate(KEY_PP_PRESS);
            break;

        case 5:
            keypage_key_operate(KEY_ONE_PRESS);
            break;

        default:
            break;
        }
    }

    if (key_type == KEY_EVENT_DOUBLE_CLICK) {
        switch (key_value) {
        case 0:
            keypage_vol_ctrl(1);
            break;

        case 1:
            keypage_vol_ctrl(0);
            break;

        case 2:
            // 清除配对
            if (ble_hid_is_connected()) {
                le_hogp_disconnect();
            }
            le_hogp_set_pair_allow();
            break;

        case 4:
            // 双击点赞
            keypage_key_operate(KEY_PP_PRESS);
            sys_timeout_add((void *)KEY_PP_PRESS, (void *)keypage_key_operate, DOUBLE_KEY_DELAY_TIME);
            break;

        case 5:
            keypage_key_operate(KEY_ONE_PRESS);
            sys_timeout_add((void *)KEY_ONE_PRESS, (void *)keypage_key_operate, DOUBLE_KEY_DELAY_TIME);
            break;

        default:
            break;
        }
    }

    if (key_type == KEY_EVENT_LONG) {
        switch (key_value) {
        case 0:
            keypage_vol_ctrl(1);
            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            break;

        default:
            break;
        }
    }
    /* 通过按键三击来调整X Y的坐标达到拍照按键位置的调整，存储在VM，掉电后依旧保留之前的坐标   */
    if (key_type == KEY_EVENT_TRIPLE_CLICK) {
        switch (key_value) {
        case 0:
            keypage_lab_value.y_lab += 128;
            if (keypage_lab_value.y_lab >= 4100) {
                keypage_lab_value.y_lab = 0;
            }
            keypage_coordinate_equal_ios(0, 2);
            keypage_coordinate_equal();
            break;
        case 1:
            keypage_lab_value.y_lab -= 128;
            if (keypage_lab_value.y_lab == 0) {
                keypage_lab_value.y_lab =  4100 ;
            }
            keypage_coordinate_equal_ios(0, 1);
            keypage_coordinate_equal();
            break;

        case 2:
            keypage_lab_value.x_lab -= 128;
            if (keypage_lab_value.x_lab == 0) {
                keypage_lab_value.x_lab = 4095;
            }
            keypage_coordinate_equal_ios(2, 0);
            keypage_coordinate_equal();
            break;

        case 3:
            keypage_lab_value.x_lab += 128;
            if (keypage_lab_value.x_lab > 4095) {
                keypage_lab_value.x_lab = 0;
            }
            keypage_coordinate_equal_ios(1, 0);
            keypage_coordinate_equal();
            break;

        case 4:
            break;

        case 5:
            //send press
            if (REMOTE_IS_IOS()) {
                keypage_send_packect((uint8_t *)key_one_press_before, sizeof(key_one_press_before), REPROT_INFO_LEN0);
            } else {
                keypage_send_packect((uint8_t *)key_one_press, sizeof(key_one_press), REPROT_INFO_LEN1);
            }
            break;

        default:
            break;
        }
    }

    if (key_type == KEY_EVENT_HOLD) {
        switch (key_value) {
        case 0:
            keypage_send_packect((uint8_t *)key_up_hold_press, sizeof(key_up_hold_press), REPROT_INFO_LEN0);
            keypage_send_packect((uint8_t *)key_up_hold_release, sizeof(key_up_hold_release), REPROT_INFO_LEN0);
            break;

        case 1:
            keypage_send_packect((uint8_t *)key_down_hold_press, sizeof(key_down_hold_press), REPROT_INFO_LEN0);
            keypage_send_packect((uint8_t *)key_down_hold_release, sizeof(key_down_hold_release), REPROT_INFO_LEN0);
            break;

        case 2:
            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            break;

        default:
            break;
        }
    }

    if (key_type == KEY_EVENT_UP) {
        switch (key_value) {
        case 0:
            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            break;

        case 4:
            //send release
            if (REMOTE_IS_IOS()) {
                keypage_send_packect((uint8_t *)key_pp_release_before, sizeof(key_pp_release_before), REPROT_INFO_LEN0);
            } else {
                keypage_send_packect((uint8_t *)key_pp_release, sizeof(key_pp_release), REPROT_INFO_LEN1);
            }
            break;

        case 5:
            //send release
            if (REMOTE_IS_IOS()) {
                keypage_send_packect((uint8_t *)key_one_release_before, sizeof(key_one_release_before), REPROT_INFO_LEN0);
            } else {
                keypage_send_packect((uint8_t *)key_one_release, sizeof(key_one_release), REPROT_INFO_LEN1);
            }
            break;

        default:
            break;
        }
    }
    keypage_coordinate_vm_deal(VM_WRITE);
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
void keypage_coordinate_vm_deal(uint8_t flag)
{
    hid_vm_lab lab;
    int ret;
    memset(&lab, 0, sizeof(hid_vm_lab));
    if (flag == 0) {
        ret = syscfg_read(CFG_COORDINATE_ADDR, (uint8_t *)&lab, sizeof(lab));
        if (ret <= 0) {
            log_info("init null \n");
            keypage_lab_value.x_lab =  0x0770;
            keypage_lab_value.y_lab =  0x0770;
        } else {

            keypage_lab_value.x_lab = lab.x_vm_lab;
            keypage_lab_value.y_lab = lab.y_vm_lab;
        }
    } else {
        lab.x_vm_lab = keypage_lab_value.x_lab;
        lab.y_vm_lab = keypage_lab_value.y_lab;
        syscfg_write(CFG_COORDINATE_ADDR, (uint8_t *)&lab, sizeof(lab));
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
static void keypage_set_soft_poweroff(void)
{
    log_info("keypage_set_soft_poweroff\n");
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    keypage_is_active = 1;
#endif

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
static void keypage_app_bt_start()
{
    uint32_t sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    btstack_ble_start_before_init(&keypage_ble_config, 0);
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
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
static void keypage_app_start()
{
    log_info("=======================================");
    log_info("-------------keypage demo--------------");
    log_info("=======================================");

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    clock_bt_init();
    // close more data
    set_config_vendor_le_bb(VENDOR_BB_MD_CLOSE);
    keypage_app_bt_start();

#if TCFG_LED_ENABLE
    led_operate(LED_INIT_FLASH);
#endif

    keypage_coordinate_vm_deal(VM_RAED);

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //无操作定时软关机
    keypage_auto_shutdown_timer = sys_timeout_add(NULL, keypage_set_soft_poweroff, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
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
static int keypage_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_KEYPAGE:
            keypage_app_start();
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
static int keypage_bt_hci_event_handler(struct bt_event *bt)
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
static int keypage_bt_connction_status_event_handler(struct bt_event *bt)
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
 *  \brief      按键事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void keypage_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;

        event_type = event->u.key.event;
        key_value = event->u.key.value;
        keypage_app_key_deal_test(event_type, key_value);
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
static int keypage_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE,%d \n", bt->value);
        keypage_remote_type = bt->value;
        if (keypage_remote_type == REMOTE_DEV_IOS) {
            le_hogp_set_ReportMap((uint8_t *)keypage_report_map_ios, sizeof(keypage_report_map_ios));
        } else {
            le_hogp_set_ReportMap((uint8_t *)keypage_report_map, sizeof(keypage_report_map));
        }
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        keypage_auto_shutdown_disable();
        break;

    default:
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
static void keypage_ble_status_callback(ble_state_e status, uint8_t value)
{
    log_info("%s[status:0x%x value:0x%x]", __func__, status, value);
    switch (status) {
    case BLE_ST_IDLE:
        break;

    case BLE_ST_ADV:
        break;

    case BLE_ST_CONNECT:
#if TCFG_LED_ENABLE
        led_set_connect_flag(1);
        led_operate(LED_CLOSE);
        // 连接过程中灯常亮
        led_operate(LED_ON);
#endif
        break;

    case BLE_ST_SEND_DISCONN:
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

#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
        if (app_power_soft.wait_disconn) {
            app_power_soft.wait_disconn = 0;
            app_power_set_soft_poweroff(NULL);
        }
#endif
        break;

    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:
        sys_timeout_add(NULL, keypage_connect_reset_xy, 2000);//复位触点
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
 *  \brief      app 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int keypage_event_handler(struct application *app, struct sys_event *event)
{

#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    //重置无操作定时计数
    if (event->type != SYS_DEVICE_EVENT || DEVICE_EVENT_FROM_POWER != event->arg) { //过滤电源消息
        sys_timer_modify(keypage_auto_shutdown_timer, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
#endif

    switch (event->type) {
    case SYS_KEY_EVENT:
        keypage_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            keypage_bt_connction_status_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            keypage_bt_hci_event_handler(&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            keypage_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return keypage_common_event_handler(&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, keypage_set_soft_poweroff);
        }
        return 0;

    default:
        return 0;
    }

    return 0;
}

//system check go sleep is ok
static uint8_t keypage_idle_query(void)
{
    return !keypage_is_active;
}

REGISTER_LP_TARGET(app_keypage_lp_target) = {
    .name = "app_keypage",
    .is_idle = keypage_idle_query,
};

static const struct application_operation app_keypage_ops = {
    .state_machine  = keypage_state_machine,
    .event_handler  = keypage_event_handler,
};

// 注册模式
REGISTER_APPLICATION(app_keypage) = {
    .name   = "keypage",
    .action	= ACTION_KEYPAGE,
    .ops    = &app_keypage_ops,
    .state  = APP_STA_DESTROY,
};

#endif


