/*********************************************************************************************
    *   Filename        : log_config.c

    *   Description     : Optimized Code & RAM (编译优化Log配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:45

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"

/**
 * @brief Bluetooth Controller Log
 */
/*-----------------------------------------------------------*/
/* const char log_tag_const_i_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1); */
/* const char log_tag_const_d_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1); */
/* const char log_tag_const_e_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1); */
/* const char log_tag_const_c_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1); */

const char log_tag_const_i_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_HEAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);

const char log_tag_const_i_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_e_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_NORM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);

const char log_tag_const_i_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_e_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_MAIN AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);

const char log_tag_const_i_APP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_d_APP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_e_APP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_APP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);

const char log_tag_const_i_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);

const char log_tag_const_i_GPTIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_GPTIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_GPTIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);
const char log_tag_const_c_GPTIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(1);

const char log_tag_const_i_OFF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_OFF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_OFF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_OFF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);

const char log_tag_const_v_SETUP AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_SETUP AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_SETUP AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_SETUP AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_SETUP AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_BOARD AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_BOARD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_BOARD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_BOARD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_BOARD AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_UI AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_UI AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_d_UI AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_w_UI AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_UI AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_d_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_w_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_e_ONLINE_DB AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_APP_IDLE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_APP_IDLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_APP_IDLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_APP_IDLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_APP_IDLE AT(.LOG_TAG_CONST) = 1;

/* const char log_tag_const_v_APP AT(.LOG_TAG_CONST) = 0; */
/* const char log_tag_const_i_APP AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_d_APP AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_w_APP AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_e_APP AT(.LOG_TAG_CONST) = 1; */

const char log_tag_const_v_AT_CMD AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_AT_CMD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_AT_CMD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_AT_CMD AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_AT_CMD AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_USER_CFG AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_USER_CFG AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_USER_CFG AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_USER_CFG AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_USER_CFG AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_LE_TRANS AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_LE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_LE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_LE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_LE_TRANS AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_BLE_TRANS AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_BLE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_BLE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_BLE_TRANS AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_BLE_TRANS AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_AT_COM AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_AT_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_AT_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_AT_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_AT_COM AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_AT_CHAR_COM AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_AT_CHAR_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_AT_CHAR_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_AT_CHAR_COM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_AT_CHAR_COM AT(.LOG_TAG_CONST) = 1;


const char log_tag_const_v_LE_CTL AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_LE_CTL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_LE_CTL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_LE_CTL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_LE_CTL AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_USBSTACK AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_USBSTACK AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_d_USBSTACK AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_USBSTACK AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_USBSTACK AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_APP_CHARGE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_APP_CHARGE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_APP_CHARGE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_w_APP_CHARGE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_APP_CHARGE AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_DONGLE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_DONGLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_DONGLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_DONGLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_DONGLE AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_APP_POWER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_POWER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_POWER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_POWER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_POWER AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_TONE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_TONE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_TONE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_TONE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_TONE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_MULTI_CONN AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_MULTI_CONN AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_MULTI_CONN AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_MULTI_CONN AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_MULTI_CONN AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_COMM_PROC AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_COMM_PROC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_COMM_PROC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_COMM_PROC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_COMM_PROC AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_COMM_BLE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_COMM_BLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_COMM_BLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_COMM_BLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_COMM_BLE AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_NCON_24G AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_NCON_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_NCON_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_NCON_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_NCON_24G AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_LLSYNC AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_LLSYNC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_LLSYNC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_LLSYNC AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_LLSYNC AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_CENTRAL AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_CENTRAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_CENTRAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_CENTRAL AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_CENTRAL AT(.LOG_TAG_CONST) = 1;


const char log_tag_const_v_BEACON AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_BEACON AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_BEACON AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_BEACON AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_BEACON AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_ELECTROCAR AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_ELECTROCAR AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_ELECTROCAR AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_ELECTROCAR AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_ELECTROCAR AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_GATT_COMM AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_GATT_COMM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_GATT_COMM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_GATT_COMM AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_GATT_COMM AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_GATT_SERVER AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_GATT_SERVER AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_GATT_SERVER AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_GATT_SERVER AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_GATT_SERVER AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_GATT_CLIENT AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_GATT_CLIENT AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_GATT_CLIENT AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_GATT_CLIENT AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_GATT_CLIENT AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_CONN_24G AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_CONN_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_CONN_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_CONN_24G AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_CONN_24G AT(.LOG_TAG_CONST) = 1;

/* const char log_tag_const_v_ AT(.LOG_TAG_CONST) = 0; */
/* const char log_tag_const_i_ AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_d_ AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_w_ AT(.LOG_TAG_CONST) = 1; */
/* const char log_tag_const_e_ AT(.LOG_TAG_CONST) = 1; */

const char log_tag_const_v_TUYA AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_TUYA AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_TUYA AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_TUYA AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_TUYA AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_HILINK AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_HILINK AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_HILINK AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_HILINK AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_HILINK AT(.LOG_TAG_CONST) = 1;


