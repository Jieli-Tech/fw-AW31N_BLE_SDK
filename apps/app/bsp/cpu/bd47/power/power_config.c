#include "asm/power_interface.h"
#include "gpio.h"
#include "app_config.h"

//-----------------------------------------------------------------------------------------------------------------------
/* config
 */
#define POWER_PARAM_CONFIG				TCFG_LOWPOWER_LOWPOWER_SEL
#define POWER_PARAM_BTOSC_HZ			TCFG_CLOCK_OSC_HZ
#define POWER_PARAM_VDDIOM_LEV			TCFG_LOWPOWER_VDDIOM_LEVEL
#define POWER_PARAM_VDDIOW_LEV			TCFG_LOWPOWER_VDDIOW_LEVEL
#define POWER_PARAM_OSC_TYPE			TCFG_LOWPOWER_OSC_TYPE

#define CONFIG_POWER_MODE				TCFG_LOWPOWER_POWER_SEL
#define CONFIG_DCDC_TYPE				PWR_DCDC12
#define CONFIG_CHARGE_ENABLE			0

#define GPIO_CONFIG_INIT()				//gpio_config_init()

//-----------------------------------------------------------------------------------------------------------------------
/* power_param
 */
struct _power_param power_param = {
    .config         = POWER_PARAM_CONFIG,
    .btosc_hz       = POWER_PARAM_BTOSC_HZ,
    .vddiom_lev     = POWER_PARAM_VDDIOM_LEV,
    .vddiow_lev     = POWER_PARAM_VDDIOW_LEV,
    .osc_type       = POWER_PARAM_OSC_TYPE,
};

//----------------------------------------------------------------------------------------------------------------------
/* power_pdata
 */
struct _power_pdata power_pdata = {
    .power_param_p  = &power_param,
};

//----------------------------------------------------------------------------------------------------------------------
void charge_wakeup_init();
void key_wakeup_init();

static const char *power_support_type[] = {"IOVDD", "VPWR"};

void board_power_init()
{
    GPIO_CONFIG_INIT();

    printf("power_supply:%s, lowper_mode:%d", power_support_type[TCFG_POWER_SUPPLY_MODE], TCFG_LOWPOWER_LOWPOWER_SEL);

    power_control(PCONTROL_POWER_SUPPLY, TCFG_POWER_SUPPLY_MODE);
    power_control(PCONTROL_PD_VDDIO_KEEP, VDDIO_KEEP_TYPE_NORMAL);
    power_control(PCONTROL_SF_VDDIO_KEEP, VDDIO_KEEP_TYPE_NORMAL);
    //power_control(PCONTROL_SF_KEEP_LRC, 1);

    power_init(&power_pdata);

#if (!CONFIG_CHARGE_ENABLE)

    if (CONFIG_POWER_MODE == PWR_DCDC15) {
        extern_dcdc_switch(1);
    }

#endif

    key_wakeup_init();
}
