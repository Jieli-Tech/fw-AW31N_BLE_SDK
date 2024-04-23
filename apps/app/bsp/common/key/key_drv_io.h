#ifndef __KEY_DRV_IO_H__
#define __KEY_DRV_IO_H__

#include "typedef.h"
#include "gpio.h"
#include "key.h"

#define ONE_PORT_TO_LOW 		0 		//按键一个端口接低电平, 另一个端口接IO
#define ONE_PORT_TO_HIGH		1 		//按键一个端口接高电平, 另一个端口接IO
#define DOUBLE_PORT_TO_IO		2		//按键两个端口接IO
#define CUST_DOUBLE_PORT_TO_IO	3


#define KEY_NOT_SUPPORT  0x01
struct one_io_key {
    uint8_t port;
};

struct two_io_key {
    uint8_t in_port;
    uint8_t out_port;
};

union key_type {
    struct one_io_key one_io;
    struct two_io_key two_io;
};

struct iokey_port {
    union key_type key_type;
    uint8_t connect_way;
    uint8_t key_value;
};

struct iokey_platform_data {
    uint8_t enable;
    uint8_t num;
    const struct iokey_port *port;
};

extern const key_interface_t key_io_info;
void io_key_init(void);
uint8_t get_iokey_value(void);

#endif/*__KEY_DRV_IO_H__*/
