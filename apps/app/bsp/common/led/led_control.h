#ifndef __LED_CONTROL__
#define __LED_CONTROL__

#define LED1                1
#define LED2                2
#define LED_ON_STATUS       1
#define LED_OFF_STATUS      0
#define LED_ON_OFF_OPCODE   0x66

#if defined(TCFG_LED_PIN1) && defined(TCFG_LED_PIN2)
#define BLE_SLAVE_CLIENT_LED_OP_EN          1        // 是否开启主从灯控效果
#else
#define BLE_SLAVE_CLIENT_LED_OP_EN          0        // 是否开启主从灯控效果
#endif
#define LED_FLASH_1S                        1                    // 未连接闪灯时间，1秒 or 0.5s
#define SOFT_OFF_TIME_MS                    (10 * 60 * 1000)     // 未连接状态10min后关机
#define AUTO_CONNECT_TIME_MS                (8000L)
#define POWER_LED_ON_TIME_MS                (1000L)

enum {
    LED_NULL = 0,
    LED_INIT,
    LED_INIT_FLASH,
    LED_WAIT_CONNECT,
    LED_AUTO_CONNECT,
    LED_KEY_UP,
    LED_KEY_HOLD,
    LED_KEY_IO_VAILD,
    LED_CLOSE,
    LED_POWER_OFF,
    LED_LOW_POWER,
    LED_ON,
    LED_OFF
};

typedef struct {
    u8 led_opcode;
    u8 led_num;
    u8 led_status;
} on_off_opcode_t;

void led_operate(u8 state);
void led_low_power(bool enable);
void led_set_connect_flag(bool is_connect);
void led_onoff_op(on_off_opcode_t *opcode);
#endif

