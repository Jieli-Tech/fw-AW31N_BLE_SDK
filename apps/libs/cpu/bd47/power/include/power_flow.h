#ifndef __POWER_FLOW_H__
#define __POWER_FLOW_H__

#include "asm/power_interface.h"

//---------------------------------------
/*define
 */
#include "power_config.h"
#include "power_depend.h"
#include "power_debug.h"
#include "power_test.h"

//---------------------------------------
/*hw: regs and api
 */

#include "p33/p33.h"
//#include "p11/p11.h"
#include "misc_hw.h"
#include "lp_flow.h"
#include "power_lib_api.h"
#include "power_hw.h"
#include "pmu_wakeup.h"
#include "reset_hw.h"
#include "boot_mode.h"

//---------------------------------------
/*driver
 */
#include "power_driver.h"

#endif
