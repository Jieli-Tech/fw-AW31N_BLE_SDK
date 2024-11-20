/*********************************************************************************************
    *   Filename        : ble_multi_scan.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "app_modules.h"
#include "app_comm_proc.h"
#include "user_cfg.h"

#if CONFIG_APP_MULTI && CONFIG_BT_NOCONN_SCAN_NUM

#define LOG_TAG         "[MUL-SCAN]"
#include "log.h"

//------------------------SCAN配置--------------------------------//
//搜索类型
#define MULTI_SCAN_TYPE       SCAN_ACTIVE
//搜索 周期大小
#define MULTI_SCAN_INTERVAL   ADV_SCAN_MS(200)//unit: 0.625ms
//搜索 窗口大小
#define MULTI_SCAN_WINDOW     ADV_SCAN_MS(200)//unit: 0.625ms

#define RSP_TX_HEAD               0xff

#define MULTI_SCAN_LOG_ENABLE     0

static uint8_t multi_scan_buffer[ADV_RSP_PACKET_MAX * 2];
static uint8_t multi_scan_len = 0;
/*************************************************************************************************/
/*!
 *  \brief      初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void multi_scan_init(void)
{
    ble_op_set_scan_param(MULTI_SCAN_TYPE, MULTI_SCAN_INTERVAL, MULTI_SCAN_WINDOW);
}

void multi_scan_enable(u8 enable)
{
    if (enable) {
        log_info(">>>>>>open multi scan");
    } else {
        log_info(">>>>>>close multi scan");
    }

    ble_op_scan_enable(enable);
}

/*************************************************************************************************/
/*!
 *  \brief      扫描接收到的数据
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void multi_scan_data_handle(const uint8_t *data, uint16_t len)
{
    log_info("rx_data: %d", len);
    log_info_hexdump((uint8_t *)data, len);
}

/*************************************************************************************************/
/*!
 *  \brief      接收机数据处理回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void multi_scan_report_handle(adv_report_t *report_pt, uint16_t len)
{
    /* log_info("event_type,addr_type:%x,%x\n", report_pt->event_type, report_pt->address_type); */
    /* log_info_hexdump(report_pt->address, 6); */
    static uint32_t scan_packet_num = 0;//for test
    /* r_printf("rx data:%d",report_pt->length); */
    /* put_buf(report_pt->data,report_pt->length);	 */
    if (report_pt == NULL) {
        return;
    }

    if (0 == report_pt->length) {
        return;
    }

    uint8_t data_len = report_pt->length - 1;

    if (!data_len) {
        return;
    }

    if (report_pt->data[0] == RSP_TX_HEAD) {
        memcpy(&multi_scan_buffer[ADV_RSP_PACKET_MAX - 1], &report_pt->data[1], data_len);
        log_info("long_packet =%d\n", multi_scan_len);
    } else {
        memcpy(multi_scan_buffer, &report_pt->data[1], data_len);
        multi_scan_len = report_pt->data[0];
        if (multi_scan_len > ADV_RSP_PACKET_MAX - 1) {
            log_info("first_packet =%d,wait next packet\n", data_len);
            return;
        } else {
            /* log_info("short_packet =%d\n", multi_scan_len); */
        }
    }

    //for debug
#if MULTI_SCAN_LOG_ENABLE
    log_info("rssi:%d,packet_num:%u\n", report_pt->rssi, ++scan_packet_num);
    multi_scan_data_handle(multi_scan_buffer, multi_scan_len);
#endif
    multi_scan_len = 0;
}
#endif
