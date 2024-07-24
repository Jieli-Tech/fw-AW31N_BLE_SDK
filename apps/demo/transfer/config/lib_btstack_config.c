/*********************************************************************************************
    *   Filename        : btstack_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-16 11:49

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "includes.h"
#include "btcontroller_config.h"
/* #include "bt_common.h" */

/**
 * @brief Bluetooth Stack Module
 */

#if TCFG_USER_BLE_ENABLE
#if CONFIG_APP_NONCONN_24G
const int config_stack_modules = BT_BTSTACK_LE_NOCONN;
#else
const int config_stack_modules = BT_BTSTACK_LE;
#endif

#else
const int config_stack_modules = 0;
#endif

const u16 config_bt_api_msg_buffer_size    =  CONFIG_BT_API_MSG_BUFSIZE;
const u16 config_hci_host_msg_buffer_size  =  CONFIG_HOST_MSG_BUFSIZE;
const u16 config_hci_ctrl_msg_buffer_size  =  CONFIG_CTRL_MSG_BUFSIZE;

const int CONFIG_BTSTACK_BIG_FLASH_ENABLE     = 0;
const int CONFIG_BTSTACK_SUPPORT_AAC    = 0;
const int config_rcsp_stack_enable = 0;

#if TCFG_USER_BLE_ENABLE
#if CONFIG_APP_NONCONN_24G
//无链接,不需要gatt功能
const int config_le_hci_connection_num = 0;//支持同时连接个数
const int config_le_sm_support_enable = 0; //是否支持加密配对
const int config_le_gatt_server_num = 0;   //支持server角色个数
const int config_le_gatt_client_num = 0;   //支持client角色个数

#else
const int config_le_hci_connection_num = CONFIG_BT_GATT_CONNECTION_NUM;//支持同时连接个数
const int config_le_sm_support_enable = CONFIG_BT_SM_SUPPORT_ENABLE; //是否支持加密配对
const int config_le_gatt_server_num = CONFIG_BT_GATT_SERVER_NUM;   //支持server角色个数
const int config_le_gatt_client_num = CONFIG_BT_GATT_CLIENT_NUM;   //支持client角色个数
#endif

#else
const int config_le_hci_connection_num = 0;//支持同时连接个数
const int config_le_sm_support_enable = 0; //是否支持加密配对
const int config_le_gatt_server_num = 0;   //支持server角色个数
const int config_le_gatt_client_num = 0;   //支持client角色个数
#endif

const int config_le_sm_sub_sc_enable = 0;
/*
   u8 l2cap_debug_enable = 0xf0;
   u8 rfcomm_debug_enable = 0xf;
   u8 profile_debug_enable = 0xff;
   u8 ble_debug_enable    = 0xff;
 */



