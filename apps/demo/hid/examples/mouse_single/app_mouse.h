#ifndef __APP_MOUSE__
#define __APP_MOUSE__
#include "typedef.h"
#include "app_config.h"

//2.4G模式: 0---ble, 非0---2.4G配对码
#define CFG_RF_24G_CODE_ID         (0x239a239a) //32bits

//模式(edr,ble,24g)切换控制,可自己修改按键方式
#define MIDDLE_KEY_SWITCH          1   //左按键+中键,长按数秒切换 edr & ble , or 2.4g & ble, or ble & 2.4g & edr
#define MIDDLE_KEY_HOLD_CNT       (4)  //长按中键计算次数 >= 4s
//配置支持的切换模式
#define SWITCH_MODE_BLE_ENABLE     (1 & TCFG_USER_BLE_ENABLE)  //使能BLE模式
#define SWITCH_MODE_24G_ENABLE     (1 & (CFG_RF_24G_CODE_ID != 0) & TCFG_USER_BLE_ENABLE)//使能24G模式

//使能开配对管理,可自己修改按键方式
#define DOUBLE_KEY_HOLD_PAIR      (1)  //中键+右按键 长按数秒,进入当前蓝牙模块的配对模式
#define DOUBLE_KEY_HOLD_CNT       (2)  //长按中键计算次数 >= 4s

//for io deubg
#define MO_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define MO_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define MO_IO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}

//    定时器模拟sensor发数使能
//    测试时关掉TCFG_OMSENSOR_ENABLE 和 TCFG_CODE_SWITCH_ENABLE
#define TEST_MOUSE_SIMULATION_ENABLE        0
#define TEST_SET_TIMER_VALUE                10

#define BUTTONS_IDX							0
#define WHEEL_IDX               			(BUTTONS_IDX + 1)
#define SENSOR_XLSB_IDX         			0
#define SENSOR_YLSB_XMSB_IDX    			(SENSOR_XLSB_IDX + 1)
#define SENSOR_YMSB_IDX         			(SENSOR_YLSB_XMSB_IDX +1)

#define MOUSE_GTIMER_ID                     1
#define MOUSE_GTIMER_INIT_TIME              7500  // 7.5ms

#define	HID_VM_HEAD_TAG (0x3AA3)

typedef struct {
    uint8_t data[3];
    uint8_t event_type;
    uint8_t key_val;
} mouse_packet_data_t;

typedef struct {
    uint8_t button_send_flag;
    uint8_t wheel_send_flag;
    uint8_t sensor_send_flag;
} mouse_send_flags_t;

typedef struct {
    uint16_t  head_tag;
    uint8_t   mode;
    uint8_t   res;
} hid_vm_cfg_t;

typedef enum {
    HID_MODE_NULL = 0,
    HID_MODE_BLE,
    HID_MODE_24G,
    HID_MODE_INIT = 0xff
} bt_mode_e;

typedef struct {
    int mouse_gtimer_tid;                 // gtimer id
    uint16_t mouse_auto_shutdown_timer;  // auto shutdown timer id
    uint8_t mouse_is_paired;             // bt is paired
    bt_mode_e mouse_hid_mode;                // bt mode
} mouse_info_t;


enum {
    MOUSE_CPI_800 = 0,
    MOUSE_CPI_1000,
    MOUSE_CPI_1200,
    MOUSE_CPI_1600,
};

static const uint8_t mouse_report_map[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x95, 0x05,        //     Report Count (5)
    0x75, 0x01,        //     Report Size (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x05,        //     Usage Maximum (0x05)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x03,        //     Report Size (3)
    0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x0C,        //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,  //     Usage (AC Pan)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x01,        //   Usage (Consumer Control)
    0xA1, 0x00,        //   Collection (Physical)
    0x75, 0x0C,        //     Report Size (12)
    0x95, 0x02,        //     Report Count (2)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x01, 0xF8,  //     Logical Minimum (-2047)
    0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x01,        //   Report Count (1)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x0A, 0x83, 0x01,  //   Usage (AL Consumer Control Configuration)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x0A, 0x25, 0x02,  //   Usage (AC Forward)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x0A, 0x24, 0x02,  //   Usage (AC Back)
    0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x05,        //   Usage (Headphone)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    // 149 bytes
};

static const uint16_t mouse_cpi_value_table[4] = {800, 1000, 1200, 1600};
void mouse_board_devices_init(void);

#endif
