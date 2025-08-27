#ifndef __POWER_INTERFACE_H__
#define __POWER_INTERFACE_H__

//#include "generic/typedef.h"
#include "typedef.h"
#include "gpio.h"

//-------------------------------------------------------
/* p33
 */
#include "power/p33/p33_sfr.h"
#include "power/p33/p33_access.h"
#include "power/p33/charge_hw.h"
#include "power/p33/p33_api.h"
#include "power/p33/wdt.h"

//-------------------------------------------------------
/* p11
 */
//#include "power/p11/p11_csfr.h"
//#include "power/p11/p11_sfr.h"
//#include "power/p11/p11_mmap.h"
//#include "power/p11/p11_api.h"
//#include "power/p11/lp_ipc.h"


//-------------------------------------------------------
/* power
 */
#include "power/power_api.h"
#include "power/power_wakeup.h"
#include "power/power_reset.h"
#include "power/power_port.h"
#include "power/vbat_det_trim.h"
#include "power/power_trim.h"

//-------------------------------------------------------
/* app
 */
#include "power/power_app.h"
#include "power/power_compat.h"


//使能pdown模块
//TCFG_LOWPOWER_LOWPOWER_SEL != 0
extern const bool control_pdown;
//使能poff模块
//TCFG_LOWPOWER_LOWPOWER_SEL==DEEP_SLEEP_EN
extern const bool control_poff;
//使能soff模块
extern const bool control_soff;

#endif
