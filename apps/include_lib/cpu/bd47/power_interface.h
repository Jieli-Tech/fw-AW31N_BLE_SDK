#ifndef ___POWER_INTERFACE_H___
#define ___POWER_INTERFACE_H___

#include "asm/power_interface.h"

void vPortSuppressTicksAndSleep(u32 usec);

void sleep_overlay_check_reload(void);
void sleep_overlay_set_destroy(void);
void sleep_run_check_enalbe(u8 enable);


#endif
