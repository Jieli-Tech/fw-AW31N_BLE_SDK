#ifndef __POWER_TEST_H__
#define __POWER_TEST_H__

//
//
//                    test
//
//
//******************************************************************
#define PCONFIG_TEST_ENABLE				0

#define PCONFIG_PMU_WAKEUP_TEST			0
#define PCONFIG_LOWPOWER_TEST			0

//-----------------------------------------------------------------
/*test  init
 */
#define PTCONFIG_IO_WAKEUP    			0
#define PTCONFIG_WAKEUP_IO     			IO_PORTA_02

//-------------------------------------------------------------------
/*soff
 */
#define PCONFIG_PDOWN_CURRENT		    0

void pmu_test();



#endif
