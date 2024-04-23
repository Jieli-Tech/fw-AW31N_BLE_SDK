/*********************************************************************************************
    *   Filename        :led_control.c

    *   Description     :指示灯控制

    *   Author          :JL

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/

#include "includes.h"
#include "app_power_mg.h"
#include "gpio.h"
#include "app_config.h"
#include "led_control.h"

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[LED_CONTROL]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf

#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#if TCFG_LED_ENABLE
static uint8_t  led_state;
static uint8_t  led_next_state;
static uint8_t  led_io_flash;
static bool led_ble_connect;
static uint32_t led_timer_id = 0;
static uint32_t led_timeout_count;

static void led_timer_stop(void);
static void led_timer_start(uint32_t time_ms);

#ifdef TCFG_LED_PIN1
#define LED1_ON()    gpio_write(TCFG_LED_PIN1, 1)
#define LED1_OFF()   gpio_write(TCFG_LED_PIN1, 0)
#define LED1_INIT()  gpio_set_mode(IO_PORT_SPILT(TCFG_LED_PIN1), PORT_OUTPUT_LOW)
#endif

#ifdef TCFG_LED_PIN2
#define LED2_ON()    gpio_write(TCFG_LED_PIN2, 1)
#define LED2_OFF()   gpio_write(TCFG_LED_PIN2, 0)
#define LED2_INIT()  gpio_set_mode(IO_PORT_SPILT(TCFG_LED_PIN2), PORT_OUTPUT_LOW)
#endif

/*************************************************************************************************/
/*!
 *  \brief    led操作控制
 *
 *  \param   [in] state:led控制行为状态
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/

static void led_operate(uint8_t state)
{

    uint8_t prev_state = led_state;
    // 低电状态时屏蔽按键led
    if (prev_state == LED_LOW_POWER && (state == LED_KEY_UP || state == LED_KEY_HOLD)) {
        return;
    }
    /* log_info("led_state: %d>>>%d\n", led_state, state); */
    led_state = state;
    led_io_flash = 0;

    switch (state) {
    case LED_INIT:
#if BLE_SLAVE_CLIENT_LED_OP_EN
        LED1_INIT();
        LED2_INIT();
#endif
        break;

    case LED_INIT_FLASH:
#ifdef TCFG_LED_PIN2
        LED2_INIT();
#endif
        LED1_INIT();
        led_timeout_count = POWER_LED_ON_TIME_MS / 1000; //
        led_timer_start(1000);
        led_next_state = LED_WAIT_CONNECT;
        LED1_ON();
        led_ble_connect = 0;
        break;

    case LED_WAIT_CONNECT:

#if(LED_FLASH_1S)
        led_timeout_count = (SOFT_OFF_TIME_MS / 1000) * 2;
        led_timer_start(500);
#else
        led_timeout_count = (SOFT_OFF_TIME_MS / 1000) * 4;
        led_timer_start(250);
#endif

        led_io_flash = BIT(7) | BIT(0);
        led_next_state = LED_POWER_OFF;
        break;

    case LED_AUTO_CONNECT:
        led_timeout_count = 1;
        led_timer_start(AUTO_CONNECT_TIME_MS);
        led_next_state = LED_WAIT_CONNECT;
        LED1_ON();
        break;

    case LED_KEY_UP:
        led_timeout_count = 1;
        led_timer_start(300);
        if (led_ble_connect) {
            led_next_state = LED_CLOSE;
        } else {
            led_next_state = LED_WAIT_CONNECT;
        }
        LED1_ON();
        break;

    case LED_KEY_IO_VAILD:
        led_timeout_count = 1;
        led_timer_start(650);
        LED1_ON();
        break;


    case LED_KEY_HOLD:
        led_timeout_count = 4;
        led_timer_start(500);
        LED1_ON();
        break;

    case LED_CLOSE:
        LED1_OFF();
        led_timeout_count = 0;
        led_timer_stop();
        break;

    case LED_POWER_OFF:
        LED1_OFF();
        led_timeout_count = 0;
        led_timer_stop();
        /* app_power_event_to_user(POWER_EVENT_POWER_SOFTOFF); */
        break;

    case LED_LOW_POWER:
        led_io_flash = BIT(7) | BIT(0);
        led_timeout_count = (LOW_POWER_SOFTOFF_TIME / 1000) * 4;
        led_timer_start(250);
        if (led_ble_connect) {
            led_next_state = LED_CLOSE;
        } else {
            led_next_state = LED_WAIT_CONNECT;
        }
        break;

    case LED_ON:
        LED1_ON();
        break;

    case LED_OFF:
        LED1_OFF();
        break;

    default:
        log_info("Unknow led_state:%d", led_state);
        break;
    }
}

/*************************************************************************************************/
/*!
 *  \brief    led定时器函数
 *
 *  \param
 *
 *  \return
 *
 *  \note    当led_timer_count小于2时进入下一个状态
 */
/*************************************************************************************************/
static void led_timer_handle(void *priv)
{
    if (led_timeout_count < 2) {
        if (led_state == LED_LOW_POWER) {
            led_timer_stop();
        } else {
            led_operate(led_next_state);
        }
        return;
    }
    led_timeout_count--;

    //io 反转推灯才需要
    if (led_io_flash & BIT(7)) {
        led_io_flash ^= BIT(0);
        if (led_io_flash & BIT(0)) {
            LED1_ON();
        } else {
            LED1_OFF();
        }
    }

}
/*************************************************************************************************/
/*!
 *  \brief    led定时器删除
 *
 *  \param
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void led_timer_stop(void)
{
    if (led_timer_id) {
        /* log_info("led_timer stop"); */
        sys_timer_del(led_timer_id);
        led_timer_id = 0;
    }
}

/*************************************************************************************************/
/*!
 *  \brief    led定时器开始
 *
 *  \param    [in] time_ms:定时时间ms
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void led_timer_start(uint32_t time_ms)
{
    led_timer_stop();
    /* log_info("led_timer start %d ms", time_ms); */
    led_timer_id = sys_timer_add(NULL, led_timer_handle, time_ms);
}

/*************************************************************************************************/
/*!
 *  \brief    灯控制解析
 *
 *  \param    [in] opcode:见on_off_opcode_t
 *
 *  \return
 *
 *  \note   简单的灯控制符号码解析函数，用于测试ble主从控制效果
 */
/*************************************************************************************************/

void led_onoff_op(on_off_opcode_t *opcode)
{
#if BLE_SLAVE_CLIENT_LED_OP_EN
    if (opcode->led_opcode == LED_ON_OFF_OPCODE) {
        const char *log_prefix = (opcode->led_num == LED1) ? ">>>>>led1" : ">>>>>led2";
        if (opcode->led_num == LED1) {
            log_info("%s %s", log_prefix, opcode->led_status ? "on" : "off");
            opcode->led_status ? LED1_ON() : LED1_OFF();
        } else {
            log_info("%s %s", log_prefix, opcode->led_status ? "on" : "off");
            opcode->led_status ? LED2_ON() : LED2_OFF();
        }
    }
#endif
}

/*************************************************************************************************/
/*!
 *  \brief   设置ble连接标志
 *
 *  \param    [in] is_connect: 0:未连接 1:已连接
 *
 *  \return
 *
 *  \note   用于判断led下一状态控制
 */
/*************************************************************************************************/

void led_set_connect_flag(bool is_connect)
{
    led_ble_connect = is_connect;
}

/*************************************************************************************************/
/*!
 *  \brief   低电灯闪烁行为
 *
 *  \param    [in] enbale: 0:关闭 1:打开
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void led_low_power(bool enable)
{
    if (enable) {
        led_operate(LED_LOW_POWER);
    } else {
        led_operate(led_next_state);
    }
}


#endif
