#ifndef ASM_INCLUDES_H
#define ASM_INCLUDES_H

#include "cpu.h"
#include "crc16.h"
#include "clock.h"
#include "bt_log.h"
#include "hwi.h"
#include "clock_hw.h"
#include "spinlock.h"
// #include "asm/uart_types.h"
// #include "uart.h"
// #include "gpio.h"
// #include "spiflash.h"
// #include "csfr.h"
#include "power_interface.h"
// #include "asm/power/power_port.h"
// #include "asm/efuse.h"
#include "wdt.h"
#include "tick_timer_driver.h"
// #include "asm/debug.h"
// #include "asm/power/p33.h"
// #include "asm/timer.h"
// #include "asm/rtc.h"

#define CORE_V42_PHY_DEBUG_EN 0
// #define CONFIG_FPGA_ENABLE
#define THIRD_PARTY_PROFILE_ENABLE 0
#define CONFIG_NEW_BREDR_ENABLE
#define TCFG_USER_TWS_ENABLE 0
#define CLOSE_EDR_API 0

enum {
    OS_NO_ERR = 0,
    OS_TRUE,
    OS_ERR_EVENT_TYPE,
    OS_ERR_PEND_ISR,
    OS_ERR_POST_NULL_PTR,
    OS_ERR_PEVENT_NULL,
    OS_ERR_POST_ISR,
    OS_ERR_QUERY_ISR,
    OS_ERR_INVALID_OPT,
    OS_ERR_TASK_WAITING,
    OS_ERR_PDATA_NULL,
    OS_TIMEOUT,
    OS_TIMER,
    OS_TASKQ,
    OS_TASK_NOT_EXIST,
    OS_ERR_EVENT_NAME_TOO_LONG,
    OS_ERR_FLAG_NAME_TOO_LONG,
    OS_ERR_TASK_NAME_TOO_LONG,
    OS_ERR_PNAME_NULL,
    OS_ERR_TASK_CREATE_ISR,
    OS_MBOX_FULL,
    OS_Q_FULL,
    OS_Q_EMPTY,
    OS_Q_ERR,
    OS_ERR_NO_QBUF,
    OS_PRIO_EXIST,
    OS_PRIO_ERR,
    OS_PRIO_INVALID,
    OS_SEM_OVF,
    OS_TASK_DEL_ERR,
    OS_TASK_DEL_IDLE,
    OS_TASK_DEL_ISR,
    OS_NO_MORE_TCB,
    OS_TIME_NOT_DLY,
    OS_TIME_INVALID_MINUTES,
    OS_TIME_INVALID_SECONDS,
    OS_TIME_INVALID_MILLI,
    OS_TIME_ZERO_DLY,
    OS_TASK_SUSPEND_PRIO,
    OS_TASK_SUSPEND_IDLE,
    OS_TASK_RESUME_PRIO,
    OS_TASK_NOT_SUSPENDED,
    OS_MEM_INVALID_PART,
    OS_MEM_INVALID_BLKS,
    OS_MEM_INVALID_SIZE,
    OS_MEM_NO_FREE_BLKS,
    OS_MEM_FULL,
    OS_MEM_INVALID_PBLK,
    OS_MEM_INVALID_PMEM,
    OS_MEM_INVALID_PDATA,
    OS_MEM_INVALID_ADDR,
    OS_MEM_NAME_TOO_LONG,
    OS_ERR_MEM_NO_MEM,
    OS_ERR_NOT_MUTEX_OWNER,
    OS_TASK_OPT_ERR,
    OS_ERR_DEL_ISR,
    OS_ERR_CREATE_ISR,
    OS_FLAG_INVALID_PGRP,
    OS_FLAG_ERR_WAIT_TYPE,
    OS_FLAG_ERR_NOT_RDY,
    OS_FLAG_INVALID_OPT,
    OS_FLAG_GRP_DEPLETED,
    OS_ERR_PIP_LOWER,
    OS_ERR_MSG_POOL_EMPTY,
    OS_ERR_MSG_POOL_NULL_PTR,
    OS_ERR_MSG_POOL_FULL,
};

#define REGISTER_UPDATE_TARGET(target) \
        const struct update_target target sec(.update_target)


//tag TODO
#define __BANK_AVCTP_ENTRY
#define __BANK_AVCTP
#define __BANK_RF
#define __BANK_RF_TRIM
#define __BANK_RF_ENTRY


#endif
