#ifndef _APP_POWER_MG_H
#define _APP_POWER_MG_H
#include "typedef.h"
#include "app_main.h"
#include "app_config.h"
#include "power_interface.h"

#define LVD_WARNING_FOR_LOW_POWER   1//1:LVD作为低电警戒线 0:固定值作为警戒线

#if LVD_WARNING_FOR_LOW_POWER
#define LOW_POWER_VOL               app_power_lvd_warning
#define LOW_POWER_LOG               "Waring!!! Vbat is near to lvd!\n"
#else
#define LOW_POWER_VOL               3300//3.3V
#define LOW_POWER_LOG               "low power\n"
#endif

//TODO by bt
#define LOW_POWER_WARN_VAL          240
#define LOW_POWER_OFF_VAL           220

#define LOW_POWER_CHECK_INTERVAL    2000  // 2s
#define LOW_POWER_SOFTOFF_TIME      (60 * 1000)

//low power param
#define LOW_POWER_SET_MAX_TIME  4000000l  //睡眠时间,usec
#define LOW_POWER_KEEP          -2        //进入低功耗并一直保持不退出
#define LOW_POWER_NO_USE        -1        //系统此次低功耗传参无效,睡眠时间由蓝牙决定(使用时注意蓝牙是否打开)


#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
#define LOW_POWER_SOFTOFF_BT_EXIT_TIME  100 //ms, 基带关闭ADV/SCAN
struct power_soft {
    u8 wait_disconn;
    u8 power_soft_flag;
    int power_soft_handle;
};

struct power_soft app_power_soft;
#endif

enum {
    POWER_EVENT_POWER_NORMAL,
    POWER_EVENT_POWER_WARNING,
    POWER_EVENT_POWER_LOW,
    POWER_EVENT_POWER_CHANGE,
    POWER_EVENT_SYNC_TWS_VBAT_LEVEL,
    POWER_EVENT_POWER_CHARGE,
    POWER_EVENT_POWER_SOFTOFF
};

void app_power_init(void);
u16 app_power_get_vbat(void);
void app_power_event_to_user(u8 event);
int app_power_event_handler(struct device_event *dev, void (*set_soft_poweroff_call)(void));
void app_power_vbat_check();
void app_power_set_lvd(u16 lvd_value);
void app_power_set_soft_poweroff(void *priv);
void p33_io_wakeup_edge(u32 gpio, P33_IO_WKUP_EDGE edge);
#endif
