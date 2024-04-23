#include "asm/power_interface.h"
#include "app_config.h"
#include "gpio.h"

//-------------------------------------------------------------------
/*config
 */
#define CONFIG_UART_DEBUG_ENABLE	UART_DEBUG//CONFIG_DEBUG_ENABLE
#ifdef TCFG_UART0_TX_PORT
#define CONFIG_UART_DEBUG_PORT		TCFG_UART0_TX_PORT
#else
#define CONFIG_UART_DEBUG_PORT		-1
#endif



//-------------------------------------------------------------------
/*调试pdown进不去的场景，影响低功耗流程
 * 打印蓝牙和系统分别可进入低功耗的时间(msec)
 * 打印当前哪些模块处于busy,用于蓝牙已经进入sniff但系统无法进入低功耗的情况，如果usr_timer处于busy则会打印对应的func地址
 */
const char debug_is_idle = 0;

//-------------------------------------------------------------------
/* 调试快速起振信息，不影响低功耗流程
 */
const bool pdebug_xosc_resume = 0;

//-------------------------------------------------------------------
/* 调试低功耗流程
 */
//出pdown打印信息，不影响低功耗流程
const bool pdebug_pdown_info = 0;

//使能串口调试低功耗，在pdown、soff模式保持串口, pdebug_pubyte_pdown\pdebug_lp_dump_ram\pdebug_putbyte_soff\log_debug
const u32 pdebug_uart_lowpower = 0;
#ifdef CONFIG_UART_DEBUG_PORT
const u32 pdebug_uart_port = CONFIG_UART_DEBUG_PORT;
#else
const u32 pdebug_uart_port = -1;
#endif

//使能串口putbyte调试pdown流程
const bool pdebug_putbyte_pdown = 0;

//使能串口putbyte调试soff流程
const bool pdebug_putbyte_soff = 0;

//使能串口pdown/poff/soff打印所有的寄存器
const bool pdebug_lp_dump_ram = 0;

//使能uart_flowing
const bool pdebug_uart_flowing = 0;

//使能低功耗耗时检查
// 1:打印程序执行时间 2:程序执行时间有问题报错
const bool pdebug_reserve_time = 0;
