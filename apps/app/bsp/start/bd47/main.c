/*********************************************************************************************
    *   Filename        : main.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#pragma bss_seg(".main.data.bss")
#pragma data_seg(".main.data")
#pragma const_seg(".main.text.const")
#pragma code_seg(".main.text")
#pragma str_literal_override(".main.text.const")

#include "cpu_debug.h"
#include "app_main.h"
#include "config.h"
#include "app_config.h"
#include "common.h"
#include "gpio.h"
#include "clock.h"
#include "maskrom.h"
#include "init.h"
#include "asm/power_interface.h"
#include "efuse.h"
#include "app_modules.h"
#include "my_malloc.h"
#include "sys_timer.h"
#include "flash_init.h"
#include "tick_timer_driver.h"
#include "cpu_debug.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

__attribute__((noreturn))

//extern void emu_init();
extern void stack_inquire(void);
extern void system_init(void);
extern void board_init();

#if TESTBOX_UART_UPDATE_EN
extern u8 g_testbox_uart_up_flag;
#endif

void boot_osc_1pin_init();
void c_main(int cfg_addr)
{
    struct maskrom_argv mask_argv = {0};
    mask_argv.pchar = (void *)putchar;
    mask_argv.exp_hook = (void *)exception_analyze;
    mask_init(&mask_argv);
    efuse_init();

#ifdef CONFIG_SDK_DEBUG_LOG
    sdk_cpu_debug_main_init();
#endif

    power_early_flowing();
    //放在时钟初始化前,时钟会读取vm数据
    boot_osc_1pin_init();

    board_init();
    tick_timer_init();
#if TCFG_UART0_ENABLE
    log_init(TCFG_UART0_BAUDRATE);
#endif
    log_info("1 hello word bd47\n");
    /* mask_argv.local_irq_enable = NULL; */
    /* mask_argv.local_irq_disable = NULL; */
    /* mask_argv.flt = NULL; */


    HWI_Install(1, (u32)exception_irq_handler, 7) ;
    emu_init();

    clk_voltage_init(TCFG_CLOCK_MODE, DVDD_VOL_129V);
    clk_early_init(TCFG_CLOCK_SYS_PLL_SRC, TCFG_CLOCK_OSC_HZ, TCFG_CLOCK_SYS_PLL_HZ);
    wdt_init(WDT_8S);

    //test: pmu_oe
    /* JL_WLA->WLA_CON10 |= BIT(15); */
    //reinit uart io
    /*JL_OMAP->PA5_OUT = 0;*/
    /*JL_PORTA->DIR &= ~BIT(6);*/
    /*JL_OMAP->PA6_OUT = FO_UART1_TX;*/

    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_info("       bd47 setup %s %s", __DATE__, __TIME__);
    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    clock_dump();
    efuse_dump();

    sys_timer_init();

    board_power_init();

    system_init();
    power_later_flowing();

    app_main();
    wdt_close();

#if 0
    wdt_close();

    mask_init_for_app();
    irq_init();
    local_irq_disable();
    local_irq_enable();

    //上电初始化所有IO
    port_init();

    log_init(TCFG_UART0_BAUDRATE);

    pll_sel(TCFG_PLL_SEL, TCFG_PLL_DIV, TCFG_HSB_DIV);

    dump_clock_info();

    debug_init();

    wdt_init(WDT_8S);

    P3_PINR_CON &= ~BIT(0); //关闭长按复位

    /* gpio_clk_out(IO_PORTC_00, CLK_OUT_HSB); */

    log_info("time & date %s %s \n  OTP c_main\n", __TIME__, __DATE__);

    power_reset_source_dump();
    power_wakeup_reason_dump();
    sys_power_init();

    system_init();
    app();
    while (1) {
        wdt_clear();
    }
#endif
}

#if 0   //蓝牙底层状态io debug
void ble_rx_irq_iodebug_in(void)
{
    JL_PORTA->DIR &= ~BIT(0);
    JL_PORTA->OUT |=  BIT(0);
}
void ble_rx_irq_iodebug_out(void)
{
    JL_PORTA->OUT &= ~BIT(0);
}
void btstack_iodebug_in(void)
{
    JL_PORTA->DIR &= ~BIT(1);
    JL_PORTA->OUT |=  BIT(1);
}
void btstack_iodebug_out(void)
{
    JL_PORTA->OUT &= ~BIT(1);
}
#else
void ble_rx_irq_iodebug_in(void)
{
}
void ble_rx_irq_iodebug_out(void)
{
}
void btstack_iodebug_in(void)
{
}
void btstack_iodebug_out(void)
{
}
#endif



