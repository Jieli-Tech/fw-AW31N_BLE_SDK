#ifndef _APP_COMM_PROC_H_
#define _APP_COMM_PROC_H_

#include "typedef.h"

extern void sys_power_down(u32 usec);
extern void wdt_clear(void);
int app_comm_process_handler(int *msg);

#endif    //
