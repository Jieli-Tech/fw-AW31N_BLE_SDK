#ifndef __AT_CHAR_UART_H
#define __AT_CHAR_UART_H

#define UART_PREAMBLE           0xBED6

#define UART_CBUF_SIZE          0x100           //串口驱动缓存大小(单次数据超过cbuf, 则驱动会丢失数据)
#define UART_FRAM_SIZE          0x100           //单次数据包最大值(每次cbuf缓存达到fram或串口收到一次数据, 就会起一次中断)

#define UART_RX_SIZE            UART_CBUF_SIZE  //接收串口驱动数据的缓存
#define UART_TX_SIZE            0x400

#define WDT_CLEAR_TIMS          20              //每20个中断清一次狗(待定)


int ct_uart_get_uart_num(void);
s32 ct_uart_send_packet(const uint8_t *packet, int size);
s32 ct_uart_change_baud(uint32_t baud);
void at_uart_init(void *packet_handler);
#endif
