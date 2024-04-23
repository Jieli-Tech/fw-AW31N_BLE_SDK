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

#define LOG_TAG_CONST       COMM_PROC
#define LOG_TAG             "[COMM_PROC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */
static int clear_wdt_cnt;
#define CLEAR_WDT_CNT_MAX       (0x100)

int app_comm_process_handler(int *msg)
{
#if UPDATE_V2_EN && TESTBOX_BT_UPDATE_EN
    testbox_update_msg_handle(msg[0]);
#endif

#if UPDATE_V2_EN && TESTBOX_UART_UPDATE_EN
    uart_update_msg_handle(msg[0]);
#endif

    clear_wdt_cnt++;
    if (clear_wdt_cnt > CLEAR_WDT_CNT_MAX) {
        putchar('~');
        clear_wdt_cnt = 0;
        wdt_clear();
    }

#if !TCFG_NORMAL_SET_DUT_MODE
    sys_power_down(4000000);
#endif

    return 0;
}

