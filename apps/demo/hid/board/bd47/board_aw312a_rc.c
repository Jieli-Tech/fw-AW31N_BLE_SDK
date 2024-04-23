#include "app_config.h"
#include "adc_api.h"
#include "key.h"
#include "key_drv_io.h"

#ifdef CONFIG_BOARD_AW31A_RC
#include "includes.h"
/* #include "user_cfg.h" */

#define LOG_TAG_CONST       BOARD
#define LOG_TAG             "[BOARD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

void board_init()
{
    // 按键初始化
    adc_init();
#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN)
    key_init();
#endif
}

/************************** IO KEY ****************************/
#if KEY_IO_EN
const struct iokey_port iokey_list[] = {
    {
        .connect_way = TCFG_IOKEY_CONNECT_WAY,          //IO按键的连接方式
        .key_type.one_io.port = TCFG_IOKEY_POWER_ONE_PORT,    //IO按键对应的引脚
        .key_value = TCFG_IOKEY_POWER_ONE_PORT_VALUE,         //按键值
    }
};

const struct iokey_platform_data iokey_data = {
    .enable = KEY_IO_EN,                              //是否使能IO按键
    .num = ARRAY_SIZE(iokey_list),                            //IO按键的个数
    .port = iokey_list,                                       //IO按键参数表
};

#endif  /* #if TCFG_IO_KEY_ENABLE */

/************************** SOFTOFF IO PROTECT****************************/
void gpio_config_soft_poweroff(void)
{
    PORT_TABLE(g);

#if KEY_IO_EN
    PORT_PROTECT(TCFG_IOKEY_POWER_ONE_PORT);
#endif

#if KEY_MATRIX_EN
    PORT_PROTECT(MATRIX_KEY_ROW1);
    PORT_PROTECT(MATRIX_KEY_ROW2);
    PORT_PROTECT(MATRIX_KEY_ROW3);
    PORT_PROTECT(MATRIX_KEY_ROL1);
    PORT_PROTECT(MATRIX_KEY_ROL2);
    PORT_PROTECT(MATRIX_KEY_ROL3);
#endif

    __port_init((u32)gpio_config);
}

/************************** SOFTOFF IO PROTECT****************************/


/************************** IO WAKE UP CONFIG****************************/
#define        WAKE_IO_MAX_NUMS                 5
static struct _p33_io_wakeup_config keys_config[WAKE_IO_MAX_NUMS];
static void init_key_io_wakeup(const struct _p33_io_wakeup_config *config)
{
    p33_io_wakeup_port_init(config);
    p33_io_wakeup_enable(config->gpio, 1);
}

struct _p33_io_wakeup_config create_key_io_wakeup_config(u32 gpio, enum gpio_mode pullup_down_mode,
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
    // iokey唤醒口
#if KEY_IO_EN
    keys_config[index++] = create_key_io_wakeup_config(TCFG_IOKEY_POWER_ONE_PORT, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
#endif

    // adkey按键唤醒口
#if KEY_AD_EN
    keys_config[index++] = create_key_io_wakeup_config(AD_KEY_IO, PORT_INPUT_FLOATING,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
#endif

    // 矩阵按键列唤醒口
#if KEY_MATRIX_EN
    keys_config[index++] = create_key_io_wakeup_config(MATRIX_KEY_ROL1, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
    keys_config[index++] = create_key_io_wakeup_config(MATRIX_KEY_ROL2, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
    keys_config[index++] = create_key_io_wakeup_config(MATRIX_KEY_ROL3, PORT_INPUT_PULLUP_10K,
                           PORT_FLT_DISABLE, FALLING_EDGE, key_active_set);
#endif

    // 初始化和启用IO唤醒
    for (u8 i = 0; i < index; i++) {
        init_key_io_wakeup(&keys_config[i]);
    }
}
/************************** IO WAKE UP CONFIG****************************/

#endif

