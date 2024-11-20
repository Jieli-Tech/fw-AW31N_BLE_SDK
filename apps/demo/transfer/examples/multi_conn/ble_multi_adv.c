/*********************************************************************************************
    *   Filename        : ble_multi_adv.c

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

#if CONFIG_APP_MULTI && CONFIG_BT_NOCONN_ADV_NUM

#define LOG_TAG         "[MUL-ADV]"
#include "log.h"

//------------------------ADV配置--------------------------------//

// 广播周期 (unit:0.625ms)
#define MULTI_ADV_INTERVAL_MIN          ADV_SCAN_MS(100)//
#define MULTI_ADV_CHANNEL               ADV_CHANNEL_ALL

static const char *mult_adv_name = "aw31_multi_adv";
static uint8_t multi_adv_data_len = 0;
static uint8_t multi_adv_data[ADV_RSP_PACKET_MAX];//max is 31
static uint8_t multi_adv_rsp_data_len = 0;
static uint8_t multi_adv_rsp_data[ADV_RSP_PACKET_MAX];//max is 31


/*************************************************************************************************/
/*!
 *  \brief      组织adv包数据，放入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int multi_adv_set_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = multi_adv_data;

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, FLAGS_GENERAL_DISCOVERABLE_MODE | FLAGS_EDR_NOT_SUPPORTED, 1);
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF30, 2);

    if (offset > ADV_RSP_PACKET_MAX) {
        log_error("***multi_adv_data overflow!!!!!!\n");
        return -1;
    }
    multi_adv_data_len = offset;
    log_info("multi_adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      组织rsp包数据，放入buff
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int multi_adv_rsp_set_data(void)
{
    uint8_t offset = 0;
    uint8_t *buf = multi_adv_rsp_data;

    uint8_t name_len = strlen(mult_adv_name);
    uint8_t vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)mult_adv_name, name_len);

    if (offset > ADV_RSP_PACKET_MAX) {
        log_error("***mult_adv_rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("multi_adv_rsp_data(%d):", offset);
    multi_adv_rsp_data_len = offset;
    log_info_hexdump(buf, offset);
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      多机非连接广播参数设置
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void multi_adv_init(void)
{
    int ret = 0;
    uint8_t adv_type;

    ret |= multi_adv_set_data();
    ret |= multi_adv_rsp_set_data();

    if (ret) {
        log_info("multi adv_setup_init fail!!!\n");
        return;
    }

    if (multi_adv_rsp_data_len) {
        adv_type = ADV_SCAN_IND;
    } else {
        adv_type = ADV_NONCONN_IND;
    }

    ble_op_set_adv_param(MULTI_ADV_INTERVAL_MIN, adv_type, ADV_CHANNEL_ALL);
    ble_op_set_adv_data(multi_adv_data_len, multi_adv_data);
    log_info_hexdump(multi_adv_data, multi_adv_data_len);

    ble_op_set_rsp_data(multi_adv_rsp_data_len, multi_adv_rsp_data);
    log_info_hexdump(multi_adv_rsp_data, multi_adv_rsp_data_len);
}

void multi_adv_enable(u8 enable)
{
    if (enable) {
        log_info(">>>>>>open multi adv");
    } else {
        log_info(">>>>>>close multi adv");
    }
    ble_op_adv_enable(enable);
}
#endif
