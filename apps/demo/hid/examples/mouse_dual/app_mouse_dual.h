#ifndef __APP_MOUSE__
#define __APP_MOUSE__
#include "typedef.h"
#include "app_config.h"
#include "ble_hogp_profile.h"

//2.4G模式: 0---ble, 非0---2.4G配对码
#define CFG_RF_24G_CODE_ID       (0xAF9A9357) //32bits,可用void access_addr_generate(u8 *aa);生成
// #define CFG_RF_24G_CODE_ID       (0) //32bits
//配置支持的切换模式，默认支持双模
#define SWITCH_MODE_BLE_ENABLE     (1 & TCFG_USER_BLE_ENABLE)  //使能BLE模式
#define SWITCH_MODE_24G_ENABLE     (1 & (CFG_RF_24G_CODE_ID != 0) & TCFG_USER_BLE_ENABLE)//使能24G模式
// 可选USB
#define SWITCH_MODE_USB_ENABLE     TCFG_OTG_USB_DEV_EN // control by HAS_USB_EN(xxx_mouse_global_build_cfg.h)

//使能开配对管理,可自己修改按键方式
#define DOUBLE_KEY_HOLD_PAIR      (1)  //左按键键+右按键 长按数秒,进入当前蓝牙模块的配对模式
#define DOUBLE_KEY_HOLD_CNT       (2)  //长按中键计算次数 >= 2s

//模式(ble,24g)切换控制,可自己修改按键方式
#define MIDDLE_KEY_SWITCH          1   //左按键+中键,长按数秒切换 2.4g & ble
#define MIDDLE_KEY_HOLD_CNT       (2)  //长按中键计算次数 >= 2s

//for io deubg
#define MO_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define MO_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define MO_IO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}

// 模拟sensor发数使能
#define TEST_MOUSE_SIMULATION_ENABLE        0

#define BUTTONS_IDX							0
#define WHEEL_IDX               			(BUTTONS_IDX + 1)
#define SENSOR_XLSB_IDX         			0
#define SENSOR_YLSB_XMSB_IDX    			(SENSOR_XLSB_IDX + 1)
#define SENSOR_YMSB_IDX         			(SENSOR_YLSB_XMSB_IDX +1)

#define MOUSE_GTIMER_ID                         TIMER1
#define MOUSE_BLE_GTIMER_INIT_TIME              7500  // 7.5ms
#define MOUSE_USB_GTIMER_INIT_TIME              1000  // 1ms

#define	HID_VM_HEAD_TAG                     (0x3AA3)
#define MOUSE_SEND_DATA_REPORT_ID           1
#define CUSTOM_SEND_DATA_REPORT_ID          2

// 用户自定义按键使能(eg:鼠标侧边按键),可在profile自定义操作
#define CUSTOM_KEY_ENABLE                   1

// 发数函数reset wdt和睡眠时间最大数目,约2s移动重置
#define MOUSE_1MS_CLEAR_WDT_CNT_MAX            2000
#define MOUSE_7MS_CLEAR_WDT_CNT_MAX            280

typedef struct {
    uint16_t  head_tag;
    uint8_t   mode;
    uint8_t   res;
} hid_vm_cfg_t;

typedef enum {
    HID_MODE_NULL = 0,
    HID_MODE_BLE,
    HID_MODE_24G,
    HID_MODE_USB,
    HID_MODE_INIT = 0xff
} bt_mode_e;

typedef struct {
    int mouse_gtimer_tid;                   // gtimer id
    uint16_t mouse_auto_shutdown_timer;     // auto shutdown timer id
    uint8_t mouse_is_paired;                // bt is paired
    bt_mode_e mouse_hid_mode;               // bt mode
} mouse_info_t;


enum {
    MOUSE_CPI_800 = 0,
    MOUSE_CPI_1000,
    MOUSE_CPI_1200,
    MOUSE_CPI_1600,
};


// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020

static const uint8_t mouse_report_map[] = {
    0x05, 0x01,                    // Usage Page (Generic Desktop Ctrls) - 定义了后续字段的用途范畴，这里是通用桌面控制。
    0x09, 0x02,                    // Usage (Mouse) - 指定当前设备为鼠标。
    0xA1, 0x01,                    // Collection (Application) - 开始应用层集合，标志着一个应用集合的开始。
    0x85, MOUSE_SEND_DATA_REPORT_ID,                    //   Report ID (1) - 报告ID为1。
    0x09, 0x01,                    //   Usage (Pointer) - 指明是一个指针设备（在这里，是个鼠标）。
    0xA1, 0x00,                    //   Collection (Physical) - 开始物理集合，表示这是个物理设备。
    // Buttons (5 buttons)
    0x95, 0x05,                    //     Report Count (5) - 报告包含5项数据。
    0x75, 0x01,                    //     Report Size (1) - 每项数据大小为1位。
    0x05, 0x09,                    //     Usage Page (Button) - 用途页面为按钮。
    0x19, 0x01,                    //     Usage Minimum (0x01) - 按钮的最小值为1（第一个按钮）。
    0x29, 0x05,                    //     Usage Maximum (0x05) - 按钮的最大值为5（第五个按钮）。
    0x15, 0x00,                    //     Logical Minimum (0) - 数据的逻辑最小值为0。
    0x25, 0x01,                    //     Logical Maximum (1) - 数据的逻辑最大值为1。
    0x81, 0x02,                    //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) - 输入类型，代表了按钮的状态。
    // Padding for the buttons - 3 bits
    0x95, 0x01,                    //     Report Count (1) - 一项填充数据。
    0x75, 0x03,                    //     Report Size (3) - 填充大小为3位，用于对齐字节。
    0x81, 0x01,                    //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position) - 输入类型为常量，用于占位不使用。
    // Wheel (8 bits)
    0x75, 0x08,                    //     Report Size (8) - 滚轮数据的大小为8位。
    0x95, 0x01,                    //     Report Count (1) - 一项数据。
    0x05, 0x01,                    //     Usage Page (Generic Desktop Ctrls) - 同上。
    0x09, 0x38,                    //     Usage (Wheel) - 滚轮。
    0x15, 0x81,                    //     Logical Minimum (-127) - 滚轮数据的逻辑最小值为-127。
    0x25, 0x7F,                    //     Logical Maximum (127) - 滚轮数据的逻辑最大值为127。
    0x81, 0x06,                    //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position) - 输入类型，代表了滚轮的滚动。
    // X and Y Axis (12 bits each)
    0x75, 0x0C,                    //     Report Size (12) - 每项数据的大小为12位。
    0x95, 0x02,                    //     Report Count (2) - 两项数据（X轴和Y轴）。
    0x05, 0x01,                    //     Usage Page (Generic Desktop Ctrls) - 同上，指定用途页面为通用桌面控制。
    0x09, 0x30,                    //     Usage (X) - X轴。
    0x09, 0x31,                    //     Usage (Y) - Y轴。
    0x16, 0x01, 0xF8,              //     Logical Minimum (-2047) - X和Y轴数据的逻辑最小值为-2047。
    0x26, 0xFF, 0x07,              //     Logical Maximum (2047) - X和Y轴数据的逻辑最大值为2047。
    0x81, 0x06,                    //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position) - 输入类型，代表了轴的移动。
    0xC0,                          //   End Collection - 结束当前集合。
    0xC0,                           // End Collection - 结束最外层的集合。
// Volume and media control report
    0x05, 0x0C, // Usage Page (Consumer Devices)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
    0x85, CUSTOM_SEND_DATA_REPORT_ID, // Report ID (2)
    0x09, 0xE9, // Usage (Volume Up)
    0x09, 0xEA, // Usage (Volume Down)
    0x09, 0xCD, // Usage (Play/Pause)
    0x09, 0xE2, // Usage (Mute)
    0x09, 0xB6, // Usage (Scan Next Track)
    0x09, 0xB5, // Usage (Scan Previous Track)
    0x15, 0x00, // Logical Minimum (0)
    0x25, 0x01, // Logical Maximum (1)
    0x75, 0x01, // Report Size (1)
    0x95, 0x06, // Report Count (6)
    0x81, 0x02, // Input (Data,Var,Abs)
    // Padding for alignment
    0x95, 0x01, // Report Count (1)
    0x75, 0x02, // Report Size (2)
    0x81, 0x01, // Input (Const,Array,Abs)
    0xC0 // End Collection
};


static const uint16_t mouse_cpi_value_table[4] = {800, 1000, 1200, 1600};
void mouse_board_devices_init(void);
mouse_info_t *mouse_get_status_info();
void mouse_send_data_timer_init(u32 resolu_us);
#endif
