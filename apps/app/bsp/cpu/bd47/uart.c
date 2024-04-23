
#pragma bss_seg(".uart.data.bss")
#pragma data_seg(".uart.data")
#pragma const_seg(".uart.text.const")
#pragma code_seg(".uart.text")
#pragma str_literal_override(".uart.text.const")

#include "config.h"
#include "uart.h"
#include "uart_v2.h"
#include "app_config.h"
#include "gpio.h"
#include "clock.h"



//new uart driver
#define     DEBUG_UART_NUM  0
#define     DEBUG_UART_DMA_EN   0

/* #define     TCFG_UART_BAUDRATE  1000000 */

#if DEBUG_UART_DMA_EN
#define     MAX_DEBUG_FIFO  256
static u8 debug_uart_buf[2][MAX_DEBUG_FIFO];
static u32 tx_jiffies = 0;
static u16 pos = 0;
static u8 uart_buffer_index = 0;

static void uart_irq(uart_dev uart_num, enum uart_event event)
{
    tx_jiffies = jiffies;
    uart_send_bytes(DEBUG_UART_NUM, debug_uart_buf[uart_buffer_index], pos);
    uart_buffer_index = !uart_buffer_index;
    pos = 0;
}
#endif
void debug_uart_init(u32 freq)
{
#ifdef CONFIG_DEBUG_ENABLE
    //    if (FALSE == LIB_DEBUG) {
    //return;
    //    }
    const struct uart_config debug_uart_config = {
        .baud_rate = TCFG_UART0_BAUDRATE/*TCFG_UART_BAUDRATE*/,
        .tx_pin = TCFG_UART0_TX_PORT,
#ifdef TCFG_UART_RX_PORT
        .rx_pin = TCFG_UART_RX_PORT,
#else
        .rx_pin = -1,
#endif
        .tx_wait_mutex = 0,//1:不支持中断调用,互斥,0:支持中断,不互斥
    };
    SFR(JL_CLOCK->PRP_CON0, 12, 3, 1);//uart std48
    uart_init(DEBUG_UART_NUM, &debug_uart_config);

#if DEBUG_UART_DMA_EN
    struct uart_dma_config dma_config = {
        .event_mask = UART_EVENT_TX_DONE,
        .irq_priority = 0,
        .irq_callback = uart_irq,
    };
    uart_dma_init(DEBUG_UART_NUM, &dma_config);
#endif
#endif
}

AT(.log_ut_text)
static void ut_putchar(char a)
{
#ifdef CONFIG_DEBUG_ENABLE
    //    if (FALSE == LIB_DEBUG) {
    //return;
    //    }
#if DEBUG_UART_DMA_EN

    debug_uart_buf[uart_buffer_index][pos] = a;
    pos++;
    if ((jiffies - tx_jiffies > 10) || (pos == MAX_DEBUG_FIFO)) {
        tx_jiffies = jiffies;
        uart_wait_tx_idle(DEBUG_UART_NUM, 10);
        uart_send_bytes(DEBUG_UART_NUM, debug_uart_buf[uart_buffer_index], pos);
        uart_buffer_index = !uart_buffer_index;
        pos = 0;
    }
#else
    /* if(a == '\n'){                          */
    /*     uart_putbyte(DEBUG_UART_NUM, '\r'); */
    /* }                                       */
    uart_putbyte(DEBUG_UART_NUM, a);

#endif
#endif
}

static char ut_getchar(void)
{
#ifdef CONFIG_DEBUG_ENABLE
    //if (FALSE == LIB_DEBUG) {
    //return 0;
    //}
    return uart_getbyte(DEBUG_UART_NUM);
#else
    return 0;
#endif
}



AT(.log_ut_text)
void putchar(char a)
{
    ut_putchar(a);
}

AT(.log_ut_text)
char getchar(void)
{
    return ut_getchar();
}


