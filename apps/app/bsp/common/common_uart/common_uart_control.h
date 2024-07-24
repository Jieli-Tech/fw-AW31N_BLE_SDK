#ifndef __BLE_UART_CONTROL__
#define __BLE_UART_CONTROL__

#define  COMMON_UART_RX_BUF_SIZE              16

void common_uart_init(uint32_t _baud_rate, uint16_t _tx_pin, uint16_t _rx_pin);
void common_uart_regiest_receive_callback(void *rx_cb);
s32 common_uart_send_data(const void *buffer, uint32_t size);
#endif
