#ifndef _APP_DONGLE_H
#define _APP_DONGLE_H

/*测试两个usb设备上行 send*/
#define CONFIG_HIDKEY_REPORT_TEST    0//(BIT(0)|BIT(1))/*for test usb channel:bit0~ch1,bit1-ch2*/

//配置选择上报PC的描述符
//---------------------------------------------------------------------
//==========hid_key
#define HIDKEY_REPORT_ID               0x1

//2.4G模式: 0---ble, 非0---2.4G配对码,2.4G配对码主从需要相同
#define CFG_RF_24G_CODE_ID       (0) //32bits
// #define CFG_RF_24G_CODE_ID       (0xAF9A9357) //32bits,可用void access_addr_generate(u8 *aa);生成

// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020
#define CONSUMER_SCAN_FRAME_FORWARD     0x0040
#define CONSUMER_SCAN_FRAME_BACK        0x0080

//==========键盘 1
#define KEYBOARD_REPORT_ID          0x1
#define COUSTOM_CONTROL_REPORT_ID   0x2
#define MOUSE_POINT_REPORT_ID       0x3


static const u8 sHIDReportDesc_hidkey[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, HIDKEY_REPORT_ID,  //   Report ID (1)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB3,        //   Usage (Fast Forward)
    0x09, 0xB4,        //   Usage (Rewind)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    // 35 bytes
};


static const u8 sHIDReportDesc_keyboard1[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, KEYBOARD_REPORT_ID,//   Report ID (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x03,        //   Report Count (3)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x03,        //   Usage Maximum (Scroll Lock)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, COUSTOM_CONTROL_REPORT_ID,//   Report ID (3)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0x8C, 0x02,  //   Logical Maximum (652)
    0x19, 0x00,        //   Usage Minimum (Unassigned)
    0x2A, 0x8C, 0x02,  //   Usage Maximum (AC Send)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
//
    // Dummy mouse collection starts here
    //
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
    0x09, 0x02,                         // USAGE (Mouse)                    2
    0xa1, 0x01,                         // COLLECTION (Application)         4
    0x85, MOUSE_POINT_REPORT_ID,               //   REPORT_ID (Mouse)              6
    0x09, 0x01,                         //   USAGE (Pointer)                8
    0xa1, 0x00,                         //   COLLECTION (Physical)          10
    0x05, 0x09,                         //     USAGE_PAGE (Button)          12
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     16
    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
    0x75, 0x01,                         //     REPORT_SIZE (1)              22
    0x95, 0x02,                         //     REPORT_COUNT (2)             24
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
    0x95, 0x06,                         //     REPORT_COUNT (6)             28
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
    0x09, 0x30,                         //     USAGE (X)                    34
    0x09, 0x31,                         //     USAGE (Y)                    36
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       38
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        40
    0x75, 0x08,                         //     REPORT_SIZE (8)              42
    0x95, 0x02,                         //     REPORT_COUNT (2)             44
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)         46
    0xc0,                               //   END_COLLECTION                 48
    0xc0                                // END_COLLECTION                   49/50
};


//==========键盘 2
static const u8 sHIDReportDesc_stand_keyboard2[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x04,//   Report ID (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x03,        //   Report Count (3)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x03,        //   Usage Maximum (Scroll Lock)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x05,//   Report ID (3)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0x8C, 0x02,  //   Logical Maximum (652)
    0x19, 0x00,        //   Usage Minimum (Unassigned)
    0x2A, 0x8C, 0x02,  //   Usage Maximum (AC Send)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
//
    // Dummy mouse collection starts here
    //
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
    0x09, 0x02,                         // USAGE (Mouse)                    2
    0xa1, 0x01,                         // COLLECTION (Application)         4
    0x85, 0x06,               //   REPORT_ID (Mouse)              6
    0x09, 0x01,                         //   USAGE (Pointer)                8
    0xa1, 0x00,                         //   COLLECTION (Physical)          10
    0x05, 0x09,                         //     USAGE_PAGE (Button)          12
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     16
    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
    0x75, 0x01,                         //     REPORT_SIZE (1)              22
    0x95, 0x02,                         //     REPORT_COUNT (2)             24
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
    0x95, 0x06,                         //     REPORT_COUNT (6)             28
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
    0x09, 0x30,                         //     USAGE (X)                    34
    0x09, 0x31,                         //     USAGE (Y)                    36
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       38
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        40
    0x75, 0x08,                         //     REPORT_SIZE (8)              42
    0x95, 0x02,                         //     REPORT_COUNT (2)             44
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)         46
    0xc0,                               //   END_COLLECTION                 48
    0xc0                                // END_COLLECTION                   49/50
};

#define MOUSE_REPORT_ID       0x01

//==========鼠标
static const u8 sHIDReportDesc_mouse[] = {
    0x05, 0x01,                    // Usage Page (Generic Desktop Ctrls) - 定义了后续字段的用途范畴，这里是通用桌面控制。
    0x09, 0x02,                    // Usage (Mouse) - 指定当前设备为鼠标。
    0xA1, 0x01,                    // Collection (Application) - 开始应用层集合，标志着一个应用集合的开始。
    0x85, 0x01,                    //   Report ID (1) - 报告ID为1。
    0x09, 0x01,                    //   Usage (Pointer) - 指明是一个指针设备（在这里，是个鼠标）。
    0xA1, 0x00,                    //   Collection (Physical) - 开始物理集合，表示这是个物理设备。
    // Buttons (5 buttons)
    0x95, 0x05,                    //     Report Count (5) - 报告包含5项数据。
    0x75, MOUSE_REPORT_ID,                    //     Report Size (1) - 每项数据大小为1位。
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
    0x85, 0x02, // Report ID (2)
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

int dongle_ble_hid_input_handler(uint8_t *packet, uint16_t size);
int dongle_second_ble_hid_input_handler(uint8_t *packet, uint16_t size);
#endif
