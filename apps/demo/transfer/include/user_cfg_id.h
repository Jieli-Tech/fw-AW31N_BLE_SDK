#ifndef _USER_CFG_ID_H_
#define _USER_CFG_ID_H_

//=================================================================================//
//      与APP CASE相关配置项[33 ~ 99], 其他id有定义使用                            //
//NOTE: 不同的example可以复用同一个ID,没用过该功能的ID也可以重新分配占用           //
//=================================================================================//
enum {
    CFG_BLE_BONDING_REMOTE_INFO = 33, //1~4
    CFG_BLE_BONDING_REMOTE_INFO2 = 34,
    CFG_AAP_MODE_INFO           = 35,
    CFG_COORDINATE_ADDR         = 36,
    AT_CHAR_DEV_NAME            = 37,
};

#endif /* #ifndef _USER_CFG_ID_H_ */
