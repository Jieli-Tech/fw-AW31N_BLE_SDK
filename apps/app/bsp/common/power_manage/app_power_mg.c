#include "app_power_mg.h"
#include "adc_api.h"
#include "key.h"
#include "clock.h"
#include "led_control.h"
#include "app_main.h"
#include "sys_timer.h"
#include "tick_timer_driver.h"
#include "msg.h"

#define LOG_TAG_CONST     NORM
#define LOG_TAG           "[app_power]"
#include "log.h"


#define     POWEROFF_TONE_V             2500
#define     BATTERY_FULL_VALUE          4200

static uint8_t app_power_low_check = 0;
//低电检测电压，比lvd电压大300mV
static uint16_t app_power_lvd_warning;
static uint16_t app_power_vbat_voltage;
static uint16_t app_power_vbat_check_timer = 0;

extern void ble_module_enable(u8 en);
void app_power_event_to_user(uint8_t event)
{
    struct sys_event *e = event_pool_alloc();
    if (e == NULL) {
        log_info("Memory allocation failed for sys_event");
        return;
    }
    e->type = SYS_DEVICE_EVENT;
    e->arg  = (void *)DEVICE_EVENT_FROM_POWER;
    e->u.dev.event = event;
    e->u.dev.value = 0;
    main_application_operation_event(NULL, e);
}

int app_power_event_handler(struct device_event *dev, void (*set_soft_poweroff_call)(void))
{
    int ret = false;

    switch (dev->event) {
#if(TCFG_SYS_LVD_EN == 1)
    case POWER_EVENT_POWER_NORMAL:
        break;

    case POWER_EVENT_POWER_LOW:
        if (app_power_vbat_check_timer) {
            sys_timer_del(app_power_vbat_check_timer);
            app_power_vbat_check_timer = 0;
        }

        if (set_soft_poweroff_call) {
            log_info("POWER_LOW TO SOFT_POWEROFF");
            set_soft_poweroff_call();
        }
        break;
#endif

    case POWER_EVENT_POWER_SOFTOFF:
        set_soft_poweroff_call();
    default:
        break;
    }
    return ret;
}

void app_power_set_lvd(uint16_t lvd_value)
{
    app_power_lvd_warning = lvd_value;
}

#if (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
__attribute__((weak))
void app_to_recover(void)
{

}

static void soft_by_power_mode(void)
{
    sys_timeout_del(app_power_soft.power_soft_handle);//删除sys_timer没来得及处理的timer
    /* sys_timer_exit();//关闭sys_timer */
    app_power_soft.power_soft_handle = 0;
    app_power_soft.power_soft_flag |= BIT(0);
    extern void sys_power_down(u32 usec);
    putchar('{');
    sys_power_down(LOW_POWER_KEEP);//sys set no wukeup time
}

void power_wakeup_init(void)
{
    if (app_power_soft.power_soft_flag) {
        putchar('}');
        app_power_soft.power_soft_flag &= ~BIT(0);
        //open bt
        wdt_init(WDT_8S);
        /* sys_timer_init(); */
        ble_module_enable(1);
        app_to_recover();
    }
}
#endif

void app_power_set_soft_poweroff(void *priv)
{
#if (TCFG_LOWPOWER_PATTERN == SOFT_MODE)
    log_info(">>>>>>>Enter softoff");
    power_set_soft_poweroff();
#elif (TCFG_LOWPOWER_PATTERN == SOFT_BY_POWER_MODE)
    log_info(">>>>>>>Enter softoff by poweroff");
    key_active_num_set(0);
    app_power_soft.power_soft_handle = sys_timeout_add(NULL, soft_by_power_mode, LOW_POWER_SOFTOFF_BT_EXIT_TIME);//等待蓝牙基带close
#endif
}

static void app_power_lvd_warning_init(void)
{
    app_power_lvd_warning = POWEROFF_TONE_V + 300;
}

static void app_power_scan(void *priv)
{
#if TCFG_SYS_LVD_EN && TCFG_ADC_VBAT_CH_EN
    static uint16_t low_power_cnt = 0;
    uint16_t vol = adc_get_voltage(AD_CH_PMU_VBAT) * 4;
    if (0 == vol) {
        return;
    }

    /* log_info("vbat voltage : %d\n", vol); */
    if (vol <= LOW_POWER_VOL)  {
        log_info("vbat voltage : %d\n", vol);
        log_error(LOW_POWER_LOG);
        low_power_cnt++;
        // 10s后打开低电led闪烁行为
#if TCFG_LED_ENABLE
        if (low_power_cnt == 5) {
            led_low_power(1);
            app_power_low_check = 1;
        }
#endif
        // 检测低电多次后进入软关机
        if (low_power_cnt == (LOW_POWER_SOFTOFF_TIME / LOW_POWER_CHECK_INTERVAL) + 1) {
            app_power_event_to_user(POWER_EVENT_POWER_LOW);
        }
    } else {
        low_power_cnt = 0;
#if TCFG_LED_ENABLE
        if (app_power_low_check) {
            led_low_power(0);
            app_power_low_check = 0;
        }
#endif
    }

    app_power_vbat_voltage = vol;
#endif
}

void app_power_init(void)
{
#if TCFG_ADC_VBAT_CH_EN
    app_power_lvd_warning_init();
    adc_add_sample_ch(AD_CH_PMU_VBAT);
    app_power_scan(NULL);
#endif
}

uint16_t app_power_get_vbat(void)
{
    return app_power_vbat_voltage;
}

__attribute__((weak)) uint8_t app_power_remap_vbat_percent(uint16_t bat_val)
{
    return 0;
}

uint16_t app_power_get_vbat_level(void)
{
    return (adc_get_voltage(AD_CH_PMU_VBAT) * 4);
}

uint8_t app_power_get_vbat_percent(void)
{
    uint16_t tmp_bat_val;
    uint16_t bat_val = app_power_get_vbat_level();

    if (bat_val <= POWEROFF_TONE_V) {
        return 0;
    }

    tmp_bat_val = app_power_remap_vbat_percent(bat_val);
    if (!tmp_bat_val) {
        tmp_bat_val = (uint32_t)bat_val * 100  / BATTERY_FULL_VALUE;
        log_info("bat_val:%d, POWEROFF_TONE_V: %d, BATTERY_FULL_VALUE: %d\n", bat_val, POWEROFF_TONE_V, BATTERY_FULL_VALUE);
        if (tmp_bat_val > 100) {
            tmp_bat_val = 100;
        }
    }
    return (uint8_t)tmp_bat_val;
}

void app_power_vbat_check()
{
    app_power_vbat_check_timer = sys_timer_add(NULL, app_power_scan, LOW_POWER_CHECK_INTERVAL);
}

