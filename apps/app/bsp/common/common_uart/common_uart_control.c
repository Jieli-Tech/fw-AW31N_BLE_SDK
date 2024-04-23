#include "includes.h"
#include "uart_v2.h"
#include "gpio.h"
#include "app_config.h"
#include "common_uart_control.h"

#define LOG_TAG         "[common_uart]"
#include "log.h"

#if TCFG_COMMON_UART_ENABLE
NOT_KEEP_RAM
static uint8_t uart_rx_ptr[COMMON_UART_RX_BUF_SIZE];

static void (*common_uart_irq_rx_handler_callback)(uint8_t *packet, uint32_t size) = NULL;
static int common_uart_num = COMMON_UART_INDEX;


/*************************************************************************************************/
/*!
 *  \brief    注册转串口函数
 *
 *  \param    rx_cb:函数指针
 *
 *  \return
 *
 *  \note       内部调用
 */
/*************************************************************************************************/

void common_uart_regiest_receive_callback(void *rx_cb)
{
    common_uart_irq_rx_handler_callback = rx_cb;
}

/*************************************************************************************************/
/*!
 *  \brief      发送数据到串口
 *
 *  \param      [in] buffer:数据buf   size：大小
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/

s32 common_uart_send_data(const void *buffer, uint32_t size)
{
    return uart_send_bytes(common_uart_num, buffer, size);
}

/*************************************************************************************************/
/*!
 *  \brief      uart操作回调函数
 *
 *  \param      [in] uart_num:串口号 uart_event:事件
 *
 *  \return
 *
 *  \note       内部调用
 */
/*************************************************************************************************/
static void common_uart_irq_func(int uart_num, enum uart_event event)
{
    if (event & UART_EVENT_TX_DONE) {
        log_info("uart[%d] tx done", uart_num);
    }

    if (event & UART_EVENT_RX_DATA) {
        log_info("uart[%d] rx data", uart_num);
    }
    if (event & UART_EVENT_RX_TIMEOUT) {
        // 串口RX功能需要关低功耗
        uint8_t uart_rx_data[COMMON_UART_RX_BUF_SIZE] = {0};
        uart_recv_bytes(uart_num, uart_rx_data, COMMON_UART_RX_BUF_SIZE);
        log_info_hexdump(uart_rx_data, COMMON_UART_RX_BUF_SIZE);
        if (common_uart_irq_rx_handler_callback) {
            common_uart_irq_rx_handler_callback(uart_rx_data, COMMON_UART_RX_BUF_SIZE);
        }
        log_info("uart[%d] rx timerout data", uart_num);
    }

    if (event & UART_EVENT_RX_FIFO_OVF) {
        log_info("uart[%d] rx fifo ovf", uart_num);
    }
}
/*************************************************************************************************/
/*!
 *  \brief      获取当前uart的串口号
 *
 *  \param      [in]
 *
 *  \return     true or false
 *
 *  \note
 */
/*************************************************************************************************/
int common_uart_get_num(void)
{
    return common_uart_num;
}

/*************************************************************************************************/
/*!
 *  \brief      uart初始化
 *
 *  \param      [in]
 *
 *  \return     true or false
 *
 *  \note
 */
/*************************************************************************************************/
void common_uart_init(uint32_t _baud_rate, uint16_t _tx_pin, uint16_t _rx_pin)
{
    struct uart_config config = {
        .baud_rate = _baud_rate,
        .tx_pin = _tx_pin,
        .rx_pin = _rx_pin,
        .parity = UART_PARITY_DISABLE,
        .tx_wait_mutex = 0,//1:不支持中断调用,互斥,0:支持中断,不互斥
    };
    log_info(">>>>>>>>>>>>>>>>>>>>");
    log_info("uart_rx_ptr:%d", uart_rx_ptr);
    /* log_info("frame_ptr:%d", frame_ptr); */

    struct uart_dma_config dma = {
        .rx_timeout_thresh = 100,
        .frame_size = COMMON_UART_RX_BUF_SIZE,
        .event_mask = UART_EVENT_TX_DONE | UART_EVENT_RX_DATA | UART_EVENT_RX_FIFO_OVF | UART_EVENT_RX_TIMEOUT,
        .irq_priority = 3,
        .irq_callback = common_uart_irq_func,
        .rx_cbuffer = uart_rx_ptr,
        .rx_cbuffer_size = COMMON_UART_RX_BUF_SIZE,
    };

    int ut = uart_init(COMMON_UART_INDEX, &config);
    common_uart_num = ut;
    if (ut < 0) {
        log_error("init error %d", ut);
    } else {
        log_info("init ok %d", ut);
    }
    int r = uart_dma_init(ut, &dma);
    if (r < 0) {
        log_error("dma init error %d", ut);
    } else {
        log_info("dma init ok %d", ut);
    }
    // 打印当前所有串口资源
    uart_dump();
}
#endif

