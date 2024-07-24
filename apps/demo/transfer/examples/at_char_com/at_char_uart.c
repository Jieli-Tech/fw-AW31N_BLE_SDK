#include "includes.h"
#include "uart_v2.h"
#include "gpio.h"
#include "app_config.h"
#include "at.h"
#include "at_char_uart.h"
#include "app_main.h"

#if  CONFIG_APP_AT_CHAR_COM
#define LOG_TAG         "[at_char_uart]"
#include "log.h"

static int ct_uart_num;
extern uint8_t cur_atcom_cid;
const char at_change_channel_cmd[] = "AT>";
static void (*at_uart_handler_callback)(const uint8_t *packet, int size) = NULL;

NOT_KEEP_RAM
static uint8_t pRxBuffer_static[UART_RX_SIZE] __attribute__((aligned(4)));       //rx memory

/*************************************************************************************************/
/*!
 *  \brief      at 数据接收和分发处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void at_cmd_rx_handler(void)
{
    static uint8_t clear_dog_flag = 0;
    clear_dog_flag++;
    if (clear_dog_flag > WDT_CLEAR_TIMS) {
        clear_dog_flag = 0;
        wdt_clear();
    }

    s32 data_length = uart_get_recv_len(ct_uart_num);

#if FLOW_CONTROL
    if (data_length > UART_CBUF_SIZE) {
        log_error("\n uart overflow, Data loss!!!");
    }
#endif


    log_info("rx[%d]", data_length);
    uart_recv_bytes(ct_uart_num, pRxBuffer_static, data_length);

    uint8_t  *p_data = pRxBuffer_static;
    if (data_length > UART_RX_SIZE) {
        log_error("cmd overflow");
        data_length = 0;
        goto __cmd_rx_end;
    }

    log_info_hexdump(p_data, data_length);
    if (data_length > 3 && 0 == memcmp(at_change_channel_cmd, p_data, 3)) {
        cur_atcom_cid += 9;  //收到TA>命令后, 强行进入解析
        goto check_at_cmd;
    }

    if (cur_atcom_cid < 9) {
        log_info("rx_data[%d]:", data_length, p_data);
        //        put_buf(p_data,__this->data_length);
        if (at_uart_handler_callback) {
            at_uart_handler_callback(p_data, data_length);
        }
        data_length = 0;

        goto __cmd_rx_end;
    }

check_at_cmd:
    if (data_length > 1) {
        uint8_t get_cmd = 0;
        if (p_data[data_length - 1] == '\r') {
            p_data[data_length] = 0;
        } else if (p_data[data_length - 2] == '\r') {
            data_length--;
            p_data[data_length] = 0;
        } else {
            goto __cmd_rx_end;
        }

        log_info("rx_cmd[%d]:%s", data_length, p_data);

        if (at_uart_handler_callback) {
            at_uart_handler_callback(p_data, data_length);
        }
        data_length = 0;
    }

__cmd_rx_end:
#if FLOW_CONTROL
    uart1_flow_ctl_rts_resume();
#endif
    ;
}

/*************************************************************************************************/
/*!
 *  \brief      AT UART数据接收回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ct_uart_isr_cb(int uart_num, enum uart_event event)
{
    /* printf("{##%d}"); */
    if (event & UART_EVENT_RX_DATA || event & UART_EVENT_RX_TIMEOUT) {
#if FLOW_CONTROL // todo
        uart1_flow_ctl_rts_suspend();
#endif
        at_cmd_rx_handler();
    }
}

/*************************************************************************************************/
/*!
 *  \brief      AT UART初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
s32 ct_uart_init(uint32_t _baud_rate, uint16_t _tx_pin, uint16_t _rx_pin)
{
    const struct uart_config config = {
        .baud_rate = _baud_rate,
        .tx_pin = _tx_pin,
        .rx_pin = _rx_pin,
        .parity = UART_PARITY_DISABLE,
        .tx_wait_mutex = 0,//1:不支持中断调用,互斥,0:支持中断,不互斥
    };
    log_info(">>>>>>>>>>>>>>>>>>>>");
    /* log_info("uart_rx_ptr:%d", uart_rx_ptr); */
    /* log_info("frame_ptr:%d", frame_ptr); */

    const struct uart_dma_config dma = {
        .rx_timeout_thresh = 100,
        .frame_size = UART_RX_SIZE,
        .event_mask = UART_EVENT_TX_DONE | UART_EVENT_RX_DATA | UART_EVENT_RX_FIFO_OVF | UART_EVENT_RX_TIMEOUT,
        .irq_priority = 3,
        .irq_callback = ct_uart_isr_cb,
        .rx_cbuffer = pRxBuffer_static,
        .rx_cbuffer_size = UART_RX_SIZE,
    };

    ct_uart_num = uart_init(AT_UART_INDEX, &config);
    if (ct_uart_num < 0) {
        log_error("init error %d", ct_uart_num);
    } else {
        log_info("init ok %d", ct_uart_num);
    }
    int r = uart_dma_init(AT_UART_INDEX, &dma);
    if (r < 0) {
        log_error("dma init error %d", ct_uart_num);
    } else {
        log_info("dma init ok %d", ct_uart_num);
    }
    // 打印当前所有串口资源
    uart_dump();
    return r;
}

/*************************************************************************************************/
/*!
 *  \brief      获取AT UART串口号
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int ct_uart_get_uart_num(void)
{
    return ct_uart_num;
}

/*************************************************************************************************/
/*!
 *  \brief      改变AT UART 波特率
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
s32 ct_uart_change_baud(uint32_t baud)
{
    if (ct_uart_num) {
        s32 set_baud = uart_set_baudrate(ct_uart_num, baud);
        if (set_baud) {
            log_info(">>>>set baud = %d\n", set_baud);
        } else {
            log_info(">>>>set baud = fail\n");
        }
        return set_baud;
    }
    log_info("no init at char, change baud fail！！");
    return -1;
}

/*************************************************************************************************/
/*!
 *  \brief     at uart 发数
 *
 *  \param      [in]
 *
 *  \return     实际发送大小,0:error; >0:ok
 *
 *  \note
 */
/*************************************************************************************************/
static s32 ct_uart_write(const uint8_t *buf, uint32_t len)
{
    if (ct_uart_num) {
        return uart_send_bytes(ct_uart_num, buf, len);
    }
    log_info("no init at char, send data fail！！");
    return -1;
}

s32 ct_uart_send_packet(const uint8_t *packet, int size)
{
    log_info("ct_uart_send_packet:%d", size);
    return ct_uart_write(packet, size);
}

static s32 ct_dev_open(void)
{
    return ct_uart_init(TCFG_AT_UART_BAUDRATE, UART_DB_TX_PIN, UART_DB_RX_PIN);
}

/*************************************************************************************************/
/*!
 *  \brief     at uart 失能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static s32 ct_dev_close(void)
{
    if (ct_uart_num) {
        log_info("at uart close\n");
        return uart_deinit(ct_uart_num);
    }
    return -1;
}

/*************************************************************************************************/
/*!
 *  \brief     回调函数注册
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void ct_dev_register_packet_handler(void (*handler)(const uint8_t *packet, int size))
{
    at_uart_handler_callback = handler;
}

void at_uart_init(void *packet_handler)
{
    ct_dev_open();
    ct_dev_register_packet_handler(packet_handler);
}

static void clock_critical_enter(void)
{

}

static void clock_critical_exit(void)
{
    if (ct_uart_num) {
        uart_set_baudrate(ct_uart_num, TCFG_AT_UART_BAUDRATE);
    }
}

#endif




