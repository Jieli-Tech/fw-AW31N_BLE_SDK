#ifndef _USER_CFG_ID_H_
#define _USER_CFG_ID_H_

//=================================================================================//
//定义与APP CASE相关配置项范围[33 ~ 99], 其他范围id已有定义使用,详见sys_memory.h     //
//NOTE: 不同的example可以复用同一个ID,没用过该功能的ID也可以重新分配占用           //
//=================================================================================//
enum {
    CFG_BLE_BONDING_REMOTE_INFO = 33,
    CFG_BLE_BONDING_REMOTE_INFO2 = 34,
    CFG_AAP_MODE_INFO           = 35,
    CFG_COORDINATE_ADDR         = 36,
};

#endif /* #ifndef _USER_CFG_ID_H_ */
