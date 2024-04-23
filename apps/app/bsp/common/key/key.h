#ifndef _KEY_H_
#define _KEY_H_

#include "typedef.h"
#include "power_interface.h"

// #define KEY_UART_DEBUG

#ifdef KEY_UART_DEBUG
#define key_puts           log_info
#define key_printf         log_info
#define key_buf            log_info_hexdump
#else
#define key_puts(...)
#define key_printf(...)
#define key_buf(...)
#endif/*KEY_UART_DEBUG*/

/*按键输出注册接口*/
typedef int (*key_emit_t)(uint8_t key_status, uint8_t key_num, uint8_t key_type);

/*按键类型*/
typedef enum {
    KEY_TYPE_IO,
    KEY_TYPE_AD,
    KEY_TYPE_MATRIX,
    KEY_TYPE_IR,
    KEY_TYPE_TOUCH,
    KEY_TYPE_MIC,
    KEY_TYPE_LPTOUCH,
} KEY_TYPE;

/*按键门槛值*/
#define KEY_BASE_CNT  2

struct key_driver_para {
    uint8_t last_key;  			//上一次get_value按键值
//== 用于消抖类参数
    uint8_t filter_value; 		//用于按键消抖
    uint8_t filter_cnt;  		//用于按键消抖时的累加值
    const uint8_t filter_time;	//当filter_cnt累加到base_cnt值时, 消抖有效
//== 用于判定长按和HOLD事件参数
    const uint8_t long_time;  	//按键判定长按数量
    const uint8_t hold_time;  	//按键判定HOLD数量
    uint8_t press_cnt;  		 	//与long_time和hold_time对比, 判断long_event和hold_event
//== 用于判定连击事件参数
    uint8_t click_cnt;  			//单击次数
    uint8_t click_delay_cnt;  	//按键被抬起后等待连击事件延时计数
    const uint8_t click_delay_time;	////按键被抬起后等待连击事件延时数量
    uint8_t notify_value;  		//在延时的待发送按键值
};

#define NO_KEY          0xff

typedef struct {
    KEY_TYPE key_type;
    void (*key_init)(void);
    uint8_t(*key_get_value)(void);
} key_interface_t;

typedef struct {
    uint8_t key_type;
    uint8_t key_num;
} key_io_t;

//组合按键映射按键值
struct key_remap {
    uint8_t bit_value;
    uint8_t remap_value;
};

struct key_remap_data {
    uint8_t remap_num;
    const struct key_remap *table;
};

void key_init(void);
void key_scan();
void key_driver_scan(void *_scan_para);
void key_active_set(P33_IO_WKUP_EDGE edge);
#endif
