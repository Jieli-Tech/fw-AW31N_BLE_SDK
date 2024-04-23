#include "asm/power_interface.h"
#include "gpio.h"
#include "app_config.h"
#include "clock.h"

#define CONFIG_POWER_MODE				TCFG_LOWPOWER_POWER_SEL
#define CONFIG_DCDC_PORT				TCFG_DCDC_PORT
#define CONFIG_DCDC_DELAY_US			TCFG_DCDC_DELAY_US

static bool extern_dcdc_sw = 0;

AT_VOLATILE_RAM_CODE_POWER
void extern_dcdc_en(u32 en, u32 delay_us)
{
    if (CONFIG_POWER_MODE != PWR_DCDC15) {
        return;
    }

    if (CONFIG_DCDC_PORT != -1) {
        if (en) {
            gpio_set_mode(IO_PORT_SPILT(CONFIG_DCDC_PORT), PORT_OUTPUT_HIGH);
            if (delay_us) {
                udelay(delay_us);
            }
        } else {
            gpio_set_mode(IO_PORT_SPILT(CONFIG_DCDC_PORT), PORT_OUTPUT_LOW);
        }
    }
}

void ldo_en(u32 en)
{
    if (CONFIG_POWER_MODE != PWR_DCDC15) {
        return;
    }

    if (en) {
        if (CONFIG_DCDC_PORT != -1) {
            dvdd_vol_sel(DVDD_VOL_099V);
            dcvdd_vol_sel(DCVDD_VOL_1200V);
            clock_adaptive_sw(0);
            clk_set_max_frequency(128 * MHz_UNIT);
        }
    } else {
        dvdd_vol_sel(DVDD_VOL_099V);
        dcvdd_vol_sel(DCVDD_VOL_1100V);
        clock_adaptive_sw(0);
        clk_set_max_frequency(128 * MHz_UNIT);
    }
}

void extern_dcdc_switch(u32 sw)
{
    //printf("extern_dcdc: %d, sw: %d\n", extern_dcdc_sw, sw);

    if (extern_dcdc_sw == sw) {
        return;
    }

    extern_dcdc_sw = sw;

    local_irq_disable();

    if (sw) {
        extern_dcdc_en(1, CONFIG_DCDC_DELAY_US);
        ldo_en(0);
    } else {
        ldo_en(1);
        extern_dcdc_en(0, 0);
    }

    local_irq_enable();
}

void extern_dcdc_test()
{
    wdt_close();

    while (1) {
#if 1
        extern_dcdc_switch(1);
        printf("dcvdd: %d\n", get_dcvdd_vol_sel());
        mdelay(5000);
        extern_dcdc_switch(0);
        printf("dcvdd: %d\n", get_dcvdd_vol_sel());
        mdelay(5000);
#else
        extern_dcdc_en(1);
        mdelay(10000);
        extern_dcdc_en(0);
        mdelay(10000);
#endif
    }
}









