/*********************************************************************************************
    *   Filename        : ble_at_char.c

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
#include "app_comm_proc.h"
#include "le_gatt_common.h"
#include "user_cfg.h"
#include "ble_at_char.h"
#include "ble_at_char_server.h"
#include "ble_at_char_client.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if CONFIG_APP_AT_CHAR_COM

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[AT_CHAR]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf

#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

//------------------------------------------------------
//输入passkey 加密
#define PASSKEY_ENABLE                     0

static const sm_cfg_t multi_sm_init_config = {
    .master_security_auto_req = 1,
    .master_set_wait_security = 1,
    .slave_security_auto_req = 0,
    .slave_set_wait_security = 1,

#if PASSKEY_ENABLE
    .io_capabilities = IO_CAPABILITY_DISPLAY_ONLY,
#else
    .io_capabilities = IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
#endif

    .authentication_req_flags = SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION,
    .min_key_size = 7,
    .max_key_size = 16,
    .sm_cb_packet_handler = NULL,
};

extern const gatt_server_cfg_t ble_at_server_init_cfg;
extern const gatt_client_cfg_t ble_at_client_init_cfg;

//gatt 控制块初始化
static gatt_ctrl_t at_char_gatt_control_block = {
    //public
    .mtu_size = ATT_LOCAL_MTU_SIZE,
    .cbuffer_size = ATT_SEND_CBUF_SIZE,
    .multi_dev_flag	= 1,

    //config
#if CONFIG_BT_GATT_SERVER_NUM
    .server_config = &ble_at_server_init_cfg,
#else
    .server_config = NULL,
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    .client_config = &ble_at_client_init_cfg,
#else
    .client_config = NULL,
#endif

#if CONFIG_BT_SM_SUPPORT_ENABLE
    .sm_config = &multi_sm_init_config,
#else
    .sm_config = NULL,
#endif
    //cbk,event handle
    .hci_cb_packet_handler = NULL,
};

/*************************************************************************************************/
/*!
 *  \brief      ble 模块使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void ble_module_enable(uint8_t en)
{
    ble_comm_module_enable(en);
}

/*************************************************************************************************/
/*!
 *  \brief      协议栈开始前初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_before_start_init(void)
{
    log_info("%s", __FUNCTION__);
    ble_comm_init(&at_char_gatt_control_block);
}

/*************************************************************************************************/
/*!
 *  \brief      模块初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_init(void)
{
    log_info("%s\n", __FUNCTION__);
    log_info("ble_file: %s", __FILE__);

#if CONFIG_BT_GATT_SERVER_NUM
    ble_at_server_init();
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    ble_at_client_init();
#endif

}


/*************************************************************************************************/
/*!
 *  \brief      模块退出
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_exit(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_module_enable(0);

    ble_comm_exit();

#if CONFIG_BT_GATT_SERVER_NUM
    ble_at_server_exit();
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    ble_at_client_exit();
#endif

}
#endif



