#ifndef _SYS_MEMORY_H_
#define _SYS_MEMORY_H_
#include "typedef.h"

#define   CFG_BT_IDX_NUM        4
struct btif_item {
    u16 id;
    u16 data_len;
};

typedef enum {
    //================================================================================//
    //                          系统lib使用，预留32个id，不可修改顺序                 //
    //================================================================================//
    SYSMEM_INDEX_DEMO           = 0,
    CFG_BT_TRIM_INFO            = 1,
    VM_BLE_LOCAL_INFO           = 2,
    CFG_BT_FRE_OFFSET           = 3,
    VM_PMU_VOLTAGE              = 4,
    VM_VIR_RTC_TIME             = 5,
    VM_VIR_RTC_ALM              = 6,
    VM_VIR_RTC_CNT              = 7,


    VM_OSC_1PIN_LRC             = 5,//单脚osc牵引
    //=================================================================================//
    //                             只存VM配置项[50 ~ 99]                         	   //
    //=================================================================================//

    //================================================================================//
    //                              sdk保留配置项:100~128                             //
    //================================================================================//
    CFG_BLE_MODE_INFO 		   = 100,
    CFG_TWS_LOCAL_ADDR,

    //=========== btif & cfg_tool.bin & vm ============//
    CFG_BT_NAME,
    CFG_BT_MAC_ADDR,
    // VM_GMA_ALI_PARA				,
    // VM_DMA_RAND					,
    // VM_TME_AUTH_COOKIE			,
    VM_BLE_REMOTE_DB_INFO,
    VM_BLE_REMOTE_DB_00,
    VM_BLE_REMOTE_DB_01,
    VM_BLE_REMOTE_DB_02,
    VM_BLE_REMOTE_DB_03,
    VM_BLE_REMOTE_DB_04,
    VM_BLE_REMOTE_DB_05,
    VM_BLE_REMOTE_DB_06,
    VM_BLE_REMOTE_DB_07,
    VM_BLE_REMOTE_DB_08,
    VM_BLE_REMOTE_DB_09,
    //蓝牙类配置项[]
    CFG_BT_RF_POWER_ID			,
    CFG_LRC_ID   				,

} SYSMEM_INDEX;

int sysmem_init_api(u32 mem_addr, u32 mem_size);
int sysmem_read_api(u32 id, u8 *data_buf, u16 len);
int sysmem_write_api(u32 id, u8 *data_buf, u16 len);
void sysmem_pre_erase_api(void);

// 蓝牙vm使用接口
int syscfg_write(u16 item_id, const void *buf, u16 len);
int syscfg_read(u16 item_id, void *buf, u16 len);

#endif

