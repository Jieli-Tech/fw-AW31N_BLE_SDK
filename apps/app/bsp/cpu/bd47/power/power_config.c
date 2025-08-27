#include "asm/power_interface.h"
#include "gpio.h"
#include "app_config.h"
#include "gpadc_hw.h"

#define SUPPORT_KEY_WAKEUP_EN          1 //是否支持io 唤醒功能

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

const u32 osc_1pin_soff_wkup_time = 10;
const u32 osc_restart_cnt = 10;

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

    //供电模式 和 电池采样通道 匹配检查
    if (gpadc_power_supply_mode == 0) { //VDDIO 供电
        if (gpadc_ch_power != AD_CH_IOVDD) {
            assert(0, "gpadc_power_supply_mode and gpadc_ch_power not match");
        }
        if (gpadc_battery_mode == 0) {
            assert(0, "gpadc_power_supply_mode == IOVDD not supply gpadc_battery_mode == MEAN_FILTERING_MODE");
        }
    } else if (gpadc_power_supply_mode == 1) { //vpwr 供电
        if (gpadc_ch_power != AD_CH_PMU_VPWR_4) {
            assert(0, "gpadc_power_supply_mode and gpadc_ch_power not match");
        }
    }

    //电池采样通道 和 分压系数检查
    if (gpadc_ch_power == AD_CH_PMU_VPWR_4) {
        assert(gpadc_ch_power_div == 4, "gpadc_ch_power and gpadc_ch_power_div not match gpadc_ch_power_div:%d", gpadc_ch_power_div);
    }

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

#if SUPPORT_KEY_WAKEUP_EN
    key_wakeup_init();
#endif
}
