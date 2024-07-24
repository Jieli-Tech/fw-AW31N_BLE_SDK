#ifndef _OMSM_H_
#define _OMSM_H_

#include "includes.h"

#define VECTOR_REVERS(vec)	vec = ~vec; vec++

// HID鼠标的报告格式
typedef struct {
    uint8_t buttonMask;        // 按钮状态，5位用于按钮，3位用于填充
    uint8_t wheel;             // 滚轮移动，8位
    uint8_t xymovement[3];         // 数组来存放X和Y轴的移动量
} mouse_packet_data_t;

typedef struct {
    uint8_t button_send_flag;
    uint8_t wheel_send_flag;
    uint8_t sensor_send_flag;
} mouse_send_flags_t;


typedef struct {
    u8 OMSensor_id[20];
    u8 OMSensor_sclk_io;
    u8 OMSensor_data_io;
    u8 OMSensor_int_io;
} OMSENSOR_PLATFORM_DATA;

typedef struct {
    u8 OMSensor_id[20];
    u8(*OMSensor_init)(OMSENSOR_PLATFORM_DATA *);
    u8(*OMSensor_read_motion)(s16 *, s16 *);
    u8(*OMSensor_data_ready)(void);
    u8(*OMSensor_status_dump)(void);
    void (*OMSensor_wakeup)(void);
    void (*OMSensor_led_switch)(u8);
    u16(*OMSensor_set_cpi)(u16 dst_cpi);
} OMSENSOR_INTERFACE;


extern OMSENSOR_INTERFACE OMSensor_dev_begin[];
extern OMSENSOR_INTERFACE OMSensor_dev_end[];

#define REGISTER_OMSENSOR(OMSensor) \
	static OMSENSOR_INTERFACE OMSensor SEC_USED(.omsensor_dev)

#define list_for_each_omsensor(c) \
	for (c=OMSensor_dev_begin; c<OMSensor_dev_end; c++)

#define OMSENSOR_PLATFORM_DATA_BEGIN(data) \
		OMSENSOR_PLATFORM_DATA data = {

#define OMSENSOR_PLATFORM_DATA_END() \
};

bool optical_mouse_sensor_init(OMSENSOR_PLATFORM_DATA *priv);
void optical_mouse_sensor_read_motion_handler(void *priv);
void optical_mouse_read_sensor_handler_high(mouse_packet_data_t *mouse_packet, mouse_send_flags_t *mouse_flags);
u16 optical_mouse_sensor_set_cpi(u16 dst_cpi);
u8 get_optical_mouse_sensor_status(void);
void optical_mouse_sensor_force_wakeup(void);
void optical_mouse_sensor_led_switch(u8 led_status);

#endif // _OMSM_H_

