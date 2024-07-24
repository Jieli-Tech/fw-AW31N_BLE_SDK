/*********************************************************************************************
 *   Filename        : mouse_usb.c

 *   Description     :USB(1K回报率)

    *   Author          :

    *   Email           :

   *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include <stdlib.h>
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "sys_timer.h"
#include "user_cfg.h"
#include "app_mouse_dual.h"
#include "mouse_usb.h"
#include "gptimer.h"
#include "ble_hogp.h"
#include "usb_stack.h"
#include "usb_suspend_resume.h"
#include "usb/device/hid.h"

#if(CONFIG_APP_MOUSE_DUAL && SWITCH_MODE_USB_ENABLE)

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[mouse_usb]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

static uint8_t mouse_usb_wait_usb_wakeup = 1;


/*************************************************************************************************/
/*!
 *  \brief     usb in 回调函数
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_usb_in_hook()
{
    // 关闭蓝牙模块
    ble_module_enable(0);
    mouse_info_t *mouse_info = mouse_get_status_info();
    mouse_info->mouse_hid_mode = HID_MODE_USB;
    // 开启usb定时器发数
    if (mouse_info->mouse_gtimer_tid) {
        gptimer_set_timer_period(mouse_info->mouse_gtimer_tid, MOUSE_USB_GTIMER_INIT_TIME);
    } else {
        mouse_send_data_timer_init(MOUSE_USB_GTIMER_INIT_TIME);
    }
}

/*************************************************************************************************/
/*!
 *  \brief     usb初始化,OTG检测
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_usb_start()
{
    usb_hid_mouse_set_report_map(mouse_report_map, sizeof(mouse_report_map));
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
    usb_pc_in_handler_register(mouse_usb_in_hook);
}

/*************************************************************************************************/
/*!
 *  \brief     usb 发数处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_usb_data_send(uint8_t report_id, uint8_t *packet, uint16_t size)
{
    uint8_t hid_send_packet[MOUSE_USB_PACKET_LEN];
    // mouse report
    hid_send_packet[0] = report_id;
    uint8_t usb_status_ret = usb_slave_status_get();
    if (usb_status_ret == USB_SLAVE_RESUME) {
        if (mouse_usb_wait_usb_wakeup) {
            memcpy(&hid_send_packet[1], packet, MOUSE_USB_PACKET_LEN - 1);
        } else {
            log_info("clear length: %d", MOUSE_USB_PACKET_LEN - 1);
            memset(&hid_send_packet[1], 0, MOUSE_USB_PACKET_LEN - 1); //发空包
        }
    } else if (usb_status_ret == USB_SLAVE_SUSPEND) {
        mouse_usb_wait_usb_wakeup = 0;
        log_info("send remote_wakeup\n");
        usb_remote_wakeup(0);
        return;
    } else {
        ;
    }

    int ret = usb_hid_mouse_send_data(hid_send_packet, MOUSE_USB_PACKET_LEN);
    wdt_clear();
    if (ret && mouse_usb_wait_usb_wakeup == 0) {
        mouse_usb_wait_usb_wakeup = 1;
        log_info("send 0packet success!\n");
    }
    return;

}

/*************************************************************************************************/
/*!
 *  \brief     mouse usb升级
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void mouse_usb_set_go_updata(void)
{
    log_info(">>>>>>>>>>go to usb update");
    go_mask_usb_updata();
}

#endif

