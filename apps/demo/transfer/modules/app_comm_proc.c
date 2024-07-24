/*********************************************************************************************
    *   Filename        : app_comm_proc.c

    *   Description     :

    *   Author          :

    *   Email           : @zh-jieli.com

    *   Last modifiled  : 2024-03-10 14:01

    *   Copyright:(c)JIELI  2011-2026  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include <stdlib.h>
#include "app_action.h"
#include "app_config.h"
#include "app_comm_bt.h"
#include "app_modules.h"
#include "app_comm_proc.h"
#include "update_loader_download.h"
#include "usb_stack.h"
#include "app_main.h"
#include "msg.h"
#include "cpu_debug.h"
#include "rcsp_bluetooth.h"

#define LOG_TAG_CONST       COMM_PROC
#define LOG_TAG             "[COMM_PROC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */
static int logout_cnt;
#define LOGOUT_CNT_MAX       (0x100)

int app_comm_process_handler(int *msg)
{
    wdt_clear();//do everytime

    logout_cnt++;
    if (logout_cnt > LOGOUT_CNT_MAX) {
        putchar('~');
        logout_cnt = 0;
    }

#ifdef CONFIG_SDK_DEBUG_LOG
    sdk_cpu_debug_loop_call();
#endif

#if UPDATE_V2_EN && TESTBOX_BT_UPDATE_EN
    testbox_update_msg_handle(msg[0]);
#endif

#if UPDATE_V2_EN && TESTBOX_UART_UPDATE_EN
    uart_update_msg_handle(msg[0]);
#endif

#if TCFG_OTG_USB_DEV_EN && TCFG_PC_ENABLE
    usb_otg_event_handler(msg[0]);
#endif

    if (msg[0] == MSG_TYPE_EVENT) {
        main_sys_event_msg_handle(msg);
    }

#if CONFIG_APP_OTA_EN
    app_update_start(msg[0]);
    if (rcsp_update_is_start()) {
        //donot enter sleep
        return 0;
    }
#endif


#if (!TCFG_NORMAL_SET_DUT_MODE && CONFIG_BT_MODE == BT_NORMAL)
    sys_power_down(4000000);
#endif

    return 0;
}

