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
#include "clock_hw.h"
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


//extern void emu_init();
extern void stack_inquire(void);
extern void system_init(void);
extern void board_init();

#if TESTBOX_UART_UPDATE_EN
extern u8 g_testbox_uart_up_flag;
#endif

void boot_osc_1pin_init();


extern u8 osc_1pin_sta;
__attribute__((noreturn))
void c_main(int cfg_addr)
{
    struct maskrom_argv mask_argv = {0};
    mask_argv.pchar = (void *)putchar;
    mask_argv.exp_hook = (void *)exception_analyze;
    mask_init(&mask_argv);
    efuse_init();
    sys_timer_init();

#ifdef CONFIG_SDK_DEBUG_LOG
    sdk_cpu_debug_main_init();
#endif

    power_early_flowing();

    //放在时钟初始化前,时钟会读取vm数据
    boot_osc_1pin_init();

    board_init();
    tick_timer_init();

    HWI_Install(1, (u32)exception_irq_handler, 7) ;
    emu_init();

    clk_voltage_init(TCFG_CLOCK_MODE, DVDD_VOL_129V);

#if CONFIG_BLE_CONNECT_SLOT
    clk_early_init(TCFG_CLOCK_SYS_PLL_SRC, TCFG_CLOCK_OSC_HZ, 160000000);//低延时模式默认配置160M
#else
    clk_early_init(TCFG_CLOCK_SYS_PLL_SRC, TCFG_CLOCK_OSC_HZ, TCFG_CLOCK_SYS_PLL_HZ);
#endif

#if TCFG_UART0_ENABLE
    log_init(TCFG_UART0_BAUDRATE);
#endif


    wdt_init(WDT_8S);

    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_info("       bd47 setup %s %s", __DATE__, __TIME__);
    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    clock_dump();
    efuse_dump();

    board_power_init();

    system_init();
    power_later_flowing();

    //osc 牵引失败
    if ((config_xosc_1pin_en) && (get_osc_1pin_sta() == 0)) { //牵引起振失败
        power_set_soft_poweroff();
    }

    app_main();
    wdt_close();
    while (1) {
        ;
    }

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



