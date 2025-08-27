#include "app_config.h"
#ifdef CONFIG_BOARD_AW313A_MOUSE
#include "key.h"
#include "init.h"
#include "includes.h"
#include "OMSensor_manage.h"
#include "code_switch.h"
#include "app_power_mg.h"
#include "gpadc.h"
#include "key_drv_io.h"

#define LOG_TAG_CONST       BOARD
#define LOG_TAG             "[BOARD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

void mouse_send_data_timer_deinit(void);

void board_init()
{
    // 按键初始化
#if (KEY_AD_EN || TCFG_SYS_LVD_EN)
    adc_init();
#endif
#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN)
    key_init();
#endif
}

/************************ OPTICAL_MOUSE_SENSOR  config********************/
#ifdef TCFG_OMSENSOR_ENABLE
OMSENSOR_PLATFORM_DATA_BEGIN(OMSensor_data)
#if TCFG_HAL3205_EN
.OMSensor_id      = "hal3205",
#endif
#if TCFG_HAL3212_EN
 .OMSensor_id      = "hal3212",
#endif

  .OMSensor_sclk_io = TCFG_OPTICAL_SENSOR_SCLK_PORT,
   .OMSensor_data_io = TCFG_OPTICAL_SENSOR_DATA_PORT,
    .OMSensor_int_io  = TCFG_OPTICAL_SENSOR_INT_PORT,
     OMSENSOR_PLATFORM_DATA_END();
#endif  /*  TCFG_HAL3205_EN */

/**************************** CODE_SWITCH  config************************/
#if TCFG_CODE_SWITCH_ENABLE
SW_PLATFORM_DATA_BEGIN(sw_data)
.a_phase_io = TCFG_CODE_SWITCH_A_PHASE_PORT,
 .b_phase_io = TCFG_CODE_SWITCH_B_PHASE_PORT,
  SW_PLATFORM_DATA_END();
#endif  /*  TCFG_CODE_SWITCH_ENABLE*/

void mouse_board_devices_init(void)
{

#if TCFG_CODE_SWITCH_ENABLE
    code_switch_init(&sw_data);
#endif /*  TCFG_CODE_SWITCH_ENABLE*/

#ifdef TCFG_OMSENSOR_ENABLE
    optical_mouse_sensor_init(&OMSensor_data);
#endif /* TCFG_OMSENSOR_ENABLE*/
}

/************************** IO KEY ****************************/
#if KEY_IO_EN
const struct iokey_port iokey_list[] = {
    {
        .connect_way = ONE_PORT_TO_LOW,                     //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_MOUSE_LK_PORT,   //IO按键对应的引脚
        .key_value = KEY_LK_VAL,                            //按键值
    },

    {
        .connect_way = ONE_PORT_TO_LOW,                     //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_MOUSE_RK_PORT,   //IO按键对应的引脚
        .key_value = KEY_RK_VAL,                            //按键值
    },

    {
        .connect_way = ONE_PORT_TO_LOW,                     //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_MOUSE_HK_PORT,   //IO按键对应的引脚
        .key_value = KEY_HK_VAL,                            //按键值
    },
    {
        .connect_way = ONE_PORT_TO_LOW,                     //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_MOUSE_CPI_PORT,   //IO按键对应的引脚
        .key_value = KEY_CPI_VAL,                            //按键值
    },


};

const struct iokey_platform_data iokey_data = {
    .enable = KEY_IO_EN,                              //是否使能IO按键
    .num = ARRAY_SIZE(iokey_list),                            //IO按键的个数
    .port = iokey_list,                                       //IO按键参数表
};

#if MULT_KEY_ENABLE
//组合按键消息映射表
//配置注意事项:单个按键按键值需要按照顺序编号,如power:0, prev:1, next:2
//bit_value = BIT(0) | BIT(1) 指按键值为0和按键值为1的两个按键被同时按下,
//remap_value = 3指当这两个按键被同时按下后重新映射的按键值;
const struct key_remap iokey_remap_table[] = {

    {
        .bit_value = BIT(KEY_LK_VAL) | BIT(KEY_RK_VAL),
        .remap_value = KEY_LK_RK_VAL,
    },

    {
        .bit_value = BIT(KEY_LK_VAL) | BIT(KEY_HK_VAL),
        .remap_value = KEY_LK_HK_VAL,
    },

    {
        .bit_value = BIT(KEY_RK_VAL) | BIT(KEY_HK_VAL),
        .remap_value = KEY_RK_HK_VAL,
    },

    {
        .bit_value = BIT(KEY_LK_VAL) | BIT(KEY_RK_VAL) | BIT(KEY_HK_VAL),
        .remap_value = KEY_LK_RK_HK_VAL,
    },
};

const struct key_remap_data iokey_remap_data = {
    .remap_num = ARRAY_SIZE(iokey_remap_table),
    .table = iokey_remap_table,
};
#endif
#endif

#ifdef TCFG_IOKEY_MOUSE_SWITCH_PORT
static void mouse_switch_softoff_callback(P33_IO_WKUP_EDGE edge)
{
    log_info(">>>>>>>switch to softoff");
    p33_io_wakeup_edge(TCFG_IOKEY_MOUSE_SWITCH_PORT, RISING_EDGE);
    app_power_set_soft_poweroff(NULL);
}
#endif

/************************** SOFTOFF IO PROTECT****************************/
void gpio_config_soft_poweroff(void)
{
    PORT_TABLE(g);

#ifdef TCFG_IOKEY_MOUSE_SWITCH_PORT
    PORT_PROTECT(TCFG_IOKEY_MOUSE_SWITCH_PORT);
#endif

#if KEY_IO_EN
    PORT_PROTECT(TCFG_IOKEY_MOUSE_LK_PORT);
    PORT_PROTECT(TCFG_IOKEY_MOUSE_RK_PORT);
#endif

    __port_init((u32)gpio_config);
}
/************************** SOFTOFF IO PROTECT****************************/

// 软关机前需要做的操作可以放到这个函数
void set_before_softoff(void)
{
    log_info(">>>> set before softoff");
    mouse_send_data_timer_deinit();
    gpio_config_soft_poweroff();
}

platform_uninitcall(set_before_softoff);

/************************** IO WAKE UP CONFIG****************************/
#define        WAKE_IO_MAX_NUMS                 3
static struct _p33_io_wakeup_config keys_config[WAKE_IO_MAX_NUMS];
static void init_key_io_wakeup(const struct _p33_io_wakeup_config *config)
{
    p33_io_wakeup_port_init(config);
    p33_io_wakeup_enable(config->gpio, 1);
}

static struct _p33_io_wakeup_config create_key_io_wakeup_config(u32 gpio, enum gpio_mode pullup_down_mode,
        P33_IO_WKUP_FLT filter, P33_IO_WKUP_EDGE edge, void (*callback)(P33_IO_WKUP_EDGE))
{
    return (struct _p33_io_wakeup_config) {
        .gpio = gpio,
         .pullup_down_mode = pullup_down_mode,
          .filter = filter,
           .edge = edge,
            .callback = callback,
    };
}


void key_wakeup_init()
{
    u8 index = 0;
    // 鼠标按键唤醒设置
#if KEY_IO_EN
    keys_config[index++] = create_key_io_wakeup_config(TCFG_IOKEY_MOUSE_LK_PORT, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
    keys_config[index++] = create_key_io_wakeup_config(TCFG_IOKEY_MOUSE_RK_PORT, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
#endif

    // 鼠标gsensor唤醒口设置,滑动唤醒，软关机功耗会高
    /* #ifdef TCFG_OMSENSOR_ENABLE */
    /*     keys_config[index++] = create_key_io_wakeup_config(TCFG_OPTICAL_SENSOR_INT_PORT, PORT_INPUT_PULLUP_10K, */
    /*                            PORT_FLT_DISABLE, FALLING_EDGE, NULL); */
    /* #endif */

    // 鼠标开关模式切换设置
#ifdef TCFG_IOKEY_MOUSE_SWITCH_PORT
    keys_config[index++] = create_key_io_wakeup_config(TCFG_IOKEY_MOUSE_SWITCH_PORT, PORT_INPUT_PULLUP_1M,
                           PORT_FLT_DISABLE, FALLING_EDGE, mouse_switch_softoff_callback);
#endif

    // 初始化和启用IO唤醒
    for (u8 i = 0; i < index; i++) {
        init_key_io_wakeup(&keys_config[i]);
    }
}

/************************** IO WAKE UP CONFIG****************************/

u8 get_power_on_status(void)
{
#if KEY_IO_EN
    const struct iokey_port *power_io_list = NULL;
    power_io_list = iokey_data.port;

    if (iokey_data.enable) {
        if (gpio_read(power_io_list->key_type.one_io.port) == power_io_list->connect_way) {
            return 1;
        }
    }
#endif
    return 0;
}

#endif

