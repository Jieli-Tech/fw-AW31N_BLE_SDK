#include "asm/power_interface.h"
#include "uart.h"
#include "gpio.h"
#include "app_config.h"

//-------------------------------------------------------------------
/*config
 */
#define CONFIG_UART_DEBUG_ENABLE	UART_DEBUG//CONFIG_DEBUG_ENABLE
#ifdef TCFG_UART0_TX_PORT
#define CONFIG_UART_DEBUG_PORT		TCFG_UART0_TX_PORT
#else
#define CONFIG_UART_DEBUG_PORT		-1
#endif

#define DO_PLATFORM_UNINITCALL()			//do_platform_uninitcall()
#define GPIO_CONFIG_UNINIT()				//gpio_config_uninit()

static void usb_high_res()
{

}

/*-----------------------------------------------------------------------
 *进入、退出低功耗函数回调状态，函数单核操作、关中断，请勿做耗时操作
 *
 */
static u32 usb_io_con = 0;
void sleep_enter_callback(u8 step)
{
#if KEY_MATRIX_EN
    extern void set_matrixkey_row_port_output();
    // 矩阵按键进低功耗前先把行IO拉低使其可以被唤醒
    set_matrixkey_row_port_output();
#endif
    /* 此函数禁止添加打印 */
    putchar('<');

    //USB IO打印引脚特殊处理
#if (CONFIG_UART_DEBUG_ENABLE && ((CONFIG_UART_DEBUG_PORT == IO_PORT_DP) || (CONFIG_UART_DEBUG_PORT == IO_PORT_DM)))
    usb_io_con = JL_PORTUSB->DIR;
#endif

    usb_high_res();
}

void sleep_exit_callback(u32 usec)
{
    //USB IO打印引脚特殊处理
#if (CONFIG_UART_DEBUG_ENABLE && ((CONFIG_UART_DEBUG_PORT == IO_PORT_DP) || (CONFIG_UART_DEBUG_PORT == IO_PORT_DM)))
    JL_PORTUSB->DIR = usb_io_con;
#endif

    putchar('>');

}

static void __mask_io_cfg()
{
    struct app_soft_flag_t app_soft_flag = {0};
#if TCFG_CLOCK_OSC_1PIN_EN
    app_soft_flag.sfc_fast_boot = 0;
#else
    app_soft_flag.sfc_fast_boot = 1;
#endif
    mask_softflag_config(&app_soft_flag);
}

u8 power_soff_callback()
{
    DO_PLATFORM_UNINITCALL();

    extern_dcdc_switch(0);

    __mask_io_cfg();

#if KEY_MATRIX_EN
    extern void set_matrixkey_row_port_output();
    // 矩阵按键进softoff前先把行IO拉低使其可以被唤醒
    set_matrixkey_row_port_output();
#endif

    void gpio_config_soft_poweroff(void);
    gpio_config_soft_poweroff();

    GPIO_CONFIG_UNINIT();

    return 0;
}

void power_early_flowing()
{
    /*默认把低功耗部分代码加载到power overlay段*/
    lowpower_init();

    PORT_TABLE(g);

    init_boot_rom();

    printf("get_boot_rom(): %d", get_boot_rom());

    //默认关闭MCLR
    p33_mclr_sw(0);

    // 默认关闭长按复位0，由key_driver配置
    gpio_longpress_pin0_reset_config(IO_PORTA_03, 0, 0, 1, PORT_INPUT_PULLUP_10K);
    //长按复位1默认配置8s，写保护
    //gpio_longpress_pin1_reset_config(IO_LDOIN_DET, 1, 8, 1);

#if CONFIG_UART_DEBUG_ENABLE
    PORT_PROTECT(CONFIG_UART_DEBUG_PORT);
#endif
#if KEY_AD_EN
    PORT_PROTECT(AD_KEY_IO);
#endif
    power_early_init((u32)gpio_config);
}

int power_later_flowing()
{
    pmu_trim(0, 0);

    power_later_init(0);

    return 0;
}

//late_initcall(power_later_flowing);