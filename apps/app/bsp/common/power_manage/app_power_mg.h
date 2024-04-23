#ifndef _APP_POWER_MG_H
#define _APP_POWER_MG_H
#include "typedef.h"
#include "app_main.h"

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
#endif
