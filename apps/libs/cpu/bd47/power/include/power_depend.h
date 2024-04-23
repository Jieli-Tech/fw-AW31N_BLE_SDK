#ifndef __POWER_DEPEND_H__
#define __POWER_DEPEND_H__

//#include "device/device.h"
#include "list.h"
//#include "cpu/includes.h"
#include "cpu.h"
//#include "syscfg_id.h"
//#include "timer.h"
//#include "asm/charge.h"
#include "gpio.h"
#include "clock.h"
#include "efuse.h"
#include "sys_timer.h"
#include "sys_memory.h"
#include "device.h"

//
//
//                    sytem & core
//
//
//
//******************************************************************
/*  malloc
 */
#define POWER_LIST_MALLOC_USE_SYSTEM	0

void IcuEnable();

//******************************************************************
/*  timer
 */
u16 usr_timer_add(void *priv, void (*func)(void *priv), u32 msec, u8 priority);
int usr_timer_modify(u16 id, u32 msec);

#define USR_TIMER_ADD(priv, func, msec, priority) \
				sys_s_hi_timer_add(priv, func, msec)

#define USR_TIMER_MODIFIED(id, msec) \
				sys_s_hi_timer_modify(id, msec)

//******************************************************************
/*  interrupt
 */
#define p_request_irq(index, ip, hdl, cpu_id)	request_irq(index, ip, hdl, cpu_id)
#define p_irq_disable()							OS_ENTER_CRITICAL()
#define p_irq_enable()							OS_EXIT_CRITICAL()


//******************************************************************
/*  pdown
 */
void vPortSysSleepInit(void);
void timer1_sleep_init(void);

#define system_sleep_init()	 vPortSysSleepInit()
#define TIMER1_SLEEP_INIT()	 //timer1_sleep_init()

//******************************************************************
/*  typedef
 */
#define __always_inline__	_INLINE_
#define __noinline__		_NOINLINE_
#define __weak__			_WEAK_
#define __interrupt__		SET_INTERRUPT

//******************************************************************
/*  vm
 */
#define vm_read_pmu(buf, len)	syscfg_read(VM_PMU_VOLTAGE, (u8 *)buf, len)
#define vm_write_pmu(buf, len)	syscfg_write(VM_PMU_VOLTAGE, (u8 *)buf, len)

//******************************************************************
/* poff
 */
void lp_signature_set(u32 signature_addr, u32 code_addr, u32 usp, u32 ssp);

#define LP_SIGNATURE_SET(signature_addr, code_addr, usp, ssp) \
						lp_signature_set(signature_addr, code_addr, usp, ssp)

//
//
//                    peripherals
//
//
//
//******************************************************************
/* usb
 */
void usb_lowpower_enter_sleep();
void usb_lowpower_exit_sleep();

#define usb_close(lp_mode)		//usb_lowpower_enter_sleep()
#define usb_open(lp_mode)		//usb_lowpower_exit_sleep()

//******************************************************************
/* sfc
 */
void sfc_drop_cache(void *ptr, u32 len);
void sfc_suspend(u32 enable_spi);
void sfc_resume(u32 disable_spi);

#define SFC_DROP_CACHE(ptr, len)	sfc_drop_cache(ptr, len)
#define SFC_SUSPEND(enable_spi)		sfc_suspend(enable_spi)
#define SFC_RESUME(disable_spi)		sfc_resume(disable_spi)

u8 get_sfc_read_mode_t();
void flash_poweron_base(u32 port, u32 udly_time, u32 mdly_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32));
void flash_poweroff_base(u32 port, u32 udly_time, u32 mdly_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32));
void spi_io_mount_t();
void spi_io_unmount_t();
void spi_port_io_init(u32 port, u32 read_mode);
void spi_port_io_uninit(u32 port, u32 read_mode);

#define GET_SFC_BIT_MODE()			get_sfc_read_mode_t()

#define FLASH_POWERON_BASE(port, udly_time, mdly_time, custom_udelay, custom_mdelay)	\
	flash_poweron_base(port, udly_time, mdly_time, custom_udelay, custom_mdelay)

#define FLASH_POWEROFF_BASE(port, udly_time, mdly_time, custom_udelay, custom_mdelay)	\
	flash_poweroff_base(port, udly_time, mdly_time, custom_udelay, custom_mdelay)

#define SPI_IO_MOUNT()				spi_io_mount_t()

#define SPI_IO_UNMOUNT()			spi_io_unmount_t()

//******************************************************************
/* flash
 */
void norflash_entry_sleep(struct device *device);
void norflash_exit_sleep(struct device *device);
void norflash_enter_4byte_addr();

#define NORFLASH_ENTRY_SLEEP(device) norflash_entry_sleep(device)
#define NORFLASH_EXIT_SLEEP(device)  norflash_exit_sleep(device)
#define NORFLASH_ENTER_4BYTE_ADDR()  norflash_enter_4byte_addr()

//******************************************************************
/* clock
 */
u32 xosc_resume(void (*lrc_delay)(u32), u32 delay_us, void (*xosc_safety_check)(u32));
void xosc_fast_boot_demo(void (*lrc_delay)(u32), u32 delay_us, void (*xosc_safety_check)(u32));

#define PLL_CLK_Hz	clk_get("pll")


#define XOSC_FAST_BOOT(delay_fun, delay_us, xosc_safety_check)	xosc_resume(delay_fun, delay_us, xosc_safety_check)
//#define XOSC_FAST_BOOT(delay_fun, delay_us, xosc_safety_check)	xosc_fast_boot_demo(delay_fun, delay_us, xosc_safety_check)

#define UPDATE_VDD_TABLE(val)	update_vdd_table(val)

//******************************************************************
/* lpctmu
 */
u8 lpctmu_is_sf_keep(void);
void p33_ctmu_key_event_irq_handler();

#define IS_SF_KEEP_LPCTMU()         0//lpctmu_is_sf_keep()
#define LPCTMU_WAKEUP_HANDLER()		//p33_ctmu_key_event_irq_handler()

//******************************************************************
/* adc
 */
void adc_close_demo(enum LOW_POWER_LEVEL lp_mode);
void adc_open_demo(enum LOW_POWER_LEVEL lp_mode);

#define ADC_CLOSE(lp_mode)	adc_close_demo(lp_mode)
#define ADC_OPEN(lp_mode)	adc_open_demo(lp_mode)

//******************************************************************
/* charge
 */
void charge_close_demo(enum LOW_POWER_LEVEL lp_mode);

#define CHARGE_CLOSE(lp_mode)	charge_close_demo(lp_mode)

//******************************************************************
/* efuse
 */
u8 efuse_get_wvdd_level();
extern u32 efuse_page1;

#define GET_WVDD_LEVEL_TRIM()	efuse_get_wvdd_level()

#define GET_CHIP_KEY()			efuse_page1


#endif
