#pragma bss_seg(".usb_slave.hid.data.bss")
#pragma data_seg(".usb_slave.hid.data")
#pragma const_seg(".usb_slave.hid.text.const")
#pragma code_seg(".usb_slave.hid.text")
#pragma str_literal_override(".usb_slave.hid.text.const")

#include "usb/device/usb_stack.h"
#include "usb/device/hid.h"
#include "usb_config.h"
#include "usb.h"
#include "app_config.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[HID_MOUSE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "log.h"

#if USB_DEVICE_CLASS_CONFIG & HID_CLASS

static const u8 sHIDDescriptor[] = {
//HID
    //InterfaceDeszcriptor:
    USB_DT_INTERFACE_SIZE,     // Length
    USB_DT_INTERFACE,          // DescriptorType
    /* 0x04,                      // bInterface number */
    0x00,                       // bInterface number
    0x00,                      // AlternateSetting
    0x01,                      // NumEndpoint
    USB_CLASS_HID,             // Class = Human Interface Device
    0x01,                      // Subclass, 0 No subclass, 1 Boot Interface subclass
    0x02,                      // Procotol, 0 None, 1 Keyboard, 2 Mouse
    0x00,                      // Interface Name


    //HIDDescriptor:
    0x09,                      // bLength
    USB_HID_DT_HID,            // bDescriptorType, HID Descriptor
    0x01, 0x02,                // bcdHID, HID Class Specification release NO.
    0x00,                      // bCuntryCode, Country localization (=none)
    0x01,                       // bNumDescriptors, Number of descriptors to follow
    0x22,                       // bDescriptorType, Report Desc. 0x22, Physical Desc. 0x23
    0,//LOW(ReportLength)
    0, //HIGH(ReportLength)

    //EndpointDescriptor:
    USB_DT_ENDPOINT_SIZE,       // bLength
    USB_DT_ENDPOINT,            // bDescriptorType, Type
    USB_DIR_IN | HID_MOUSE_EP_IN,     // bEndpointAddress
    USB_ENDPOINT_XFER_INT,      // Interrupt
    LOBYTE(HID_MOUSE_EP_IN_MAX_SIZE), HIBYTE(HID_MOUSE_EP_IN_MAX_SIZE),// Maximum packet size
    0x01,     // Poll every 10msec seconds
};

#define HID_MOUSE_REPORT_ID               0x2
static const u8 sHIDReportDesc[] = {
    0x05, 0x01,                    // Usage Page (Generic Desktop Ctrls) - 定义了后续字段的用途范畴，这里是通用桌面控制。
    0x09, 0x02,                    // Usage (Mouse) - 指定当前设备为鼠标。
    0xA1, 0x01,                    // Collection (Application) - 开始应用层集合，标志着一个应用集合的开始。
    0x85, HID_MOUSE_REPORT_ID,                    //   Report ID (1) - 报告ID为1。
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
};

__attribute__((always_inline))
u32 usb_get_hid_mouse_report_id(void)
{
    return HID_MOUSE_REPORT_ID;
}

static struct hid_device_var_t _hid_var AT(.hid_config_var);
static struct hid_device_var_t *hid_var AT(.usb_hid.keep_ram);

void *usb_hid_mouse_init(void)
{
    memset((void *)&_hid_var, 0, sizeof(_hid_var));
    _hid_var.ep_in_buffer = usb_get_ep_buffer(0, HID_MOUSE_EP_IN | USB_DIR_IN);
#if HID_MOUSE_EP_OUT_EN
    _hid_var.ep_out_buffer = usb_get_ep_buffer(0, HID_MOUSE_EP_OUT | USB_DIR_OUT);
#endif
    return (void *)&_hid_var;
}

//add dongle set
static u8 *hid_report_map = (u8 *)sHIDReportDesc;
static int hid_report_map_size = sizeof(sHIDReportDesc);
void usb_hid_mouse_set_report_map(const u8 *map, int size)
{
    hid_report_map_size = size;
    hid_report_map = (u8 *)map;
}

static u32 get_hid_report_desc_len(u32 index)
{
    u32 len = 0;
    len = hid_report_map_size;
    return len;
}
static const void *get_hid_report_desc(u32 index)
{
    u8 *ptr  = NULL;
    ptr = hid_report_map;
    return ptr;
}

//仅针对 sHIDReportDesc_mouse 描述符进行处理
#include "spinlock.h"
#include "clock.h"
#define MOUSE_1K_TEST 0
DEFINE_SPINLOCK(mouse_data_lock);
static struct mouse_data_info {
    s32 x_axis;
    s32 y_axis;
    s8 wheel;
    u8 button;
    u8 id;
    u8 flag;
    u8 len;
} mouse_data;
static void hid_sof_tx_data(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);

#if MOUSE_1K_TEST
#else
    if (mouse_data.flag == 0) {
        return;
    }
#endif
    u32 time_ot = 20;
    while (usb_read_txcsr(usb_id, ep) & BIT(0)) {
        if (time_ot == 0) {
            return;
        }
        time_ot--;
        udelay(1);
        /* putchar('t'); */
    }

    struct mouse_data_info data;
    spin_lock(&mouse_data_lock);
    memcpy(&data, &mouse_data, sizeof(struct mouse_data_info));
    memset(&mouse_data, 0, sizeof(struct mouse_data_info));
    spin_unlock(&mouse_data_lock);

    if (data.x_axis >= 0) {
        data.x_axis = MIN(data.x_axis, (1 << 11) - 1);
    } else {
        data.x_axis = MAX(data.x_axis, 0 - (1 << 11));
    }
    if (data.y_axis >= 0) {
        data.y_axis = MIN(data.y_axis, (1 << 11) - 1);
    } else {
        data.y_axis = MAX(data.y_axis, 0 - (1 << 11));
    }

    u8 buf[6] = {0};
    buf[0] = data.id;
    buf[1] = data.button;
    buf[2] = data.wheel; //滚轮暂不考虑溢出情况
    buf[3] = data.x_axis & 0xff;
    buf[4] = ((data.y_axis & 0xf) << 4) | ((data.x_axis >> 8) & 0xf);
    buf[5] = data.y_axis >> 4;
    u8 len = data.len;


#if MOUSE_1K_TEST //测试USB_1k回报率
    static s8 test_x = 10;
    test_x = -test_x;
    s32 x_axis = test_x;
    s32 y_axis = test_x;
    buf[0] = 1; //id
    buf[1] = 0; //button
    buf[2] = 0; //wheel
    buf[3] = x_axis & 0xff; // X
    buf[4] = ((y_axis & 0xf) << 4) | ((x_axis >> 8) & 0xf); //X and Y
    buf[5] = y_axis >> 4; //Y
    len = 6;
#endif

    usb_g_intr_write(usb_id, HID_MOUSE_EP_IN, buf, len);
}
static s32 sign_extend(s32 x, u32 n_bits) //将n_bits位有符号整数转为32位有符号整数
{
    int sign_bit_position = n_bits - 1;

    // 提取符号位
    int32_t sign_bit = (x >> sign_bit_position) & 1;

    // 如果符号位为1，进行符号扩展
    if (sign_bit) {
        // 创建一个掩码，在符号位之前的所有位设为1
        int32_t mask = ~((1 << n_bits) - 1);
        x |= mask;
    }

    return x;
}

static u32 hid_tx_data(struct usb_device_t *usb_device, const u8 *buffer, u32 len)
{
    u32 ret;
    const usb_dev usb_id = usb_device2id(usb_device);
    if (usb_g_sof_intr_hander_check(usb_id, HID_MOUSE_EP_IN, hid_sof_tx_data) == 0) { //发送接口注册了sof回调函数
        ret = len;

        //仅针对 sHIDReportDesc_mouse 描述符进行处理, id = 1, len = 6
        s32 x_axis = ((buffer[4] & 0xf) << 8) | buffer[3];
        s32 y_axis = (buffer[5] << 4) | ((buffer[4] >> 4) & 0xf);
        x_axis = sign_extend(x_axis, 12);
        y_axis = sign_extend(y_axis, 12);
        spin_lock(&mouse_data_lock);
        mouse_data.id = buffer[0];
        mouse_data.button = buffer[1];
        mouse_data.x_axis += x_axis;
        mouse_data.y_axis += y_axis;
        mouse_data.wheel += buffer[2];
        mouse_data.flag = 1;
        mouse_data.len = len;
        spin_unlock(&mouse_data_lock);
    } else {
        ret = usb_g_intr_write(usb_id, HID_MOUSE_EP_IN, buffer, len);
    }
    return ret;
}
#if HID_MOUSE_EP_OUT_EN
static void hid_rx_data(struct usb_device_t *usb_device, u32 ep)
{
    if (hid_var->rx_data_wakeup) {
        hid_var->rx_data_wakeup(usb_device, ep);
    }
}
#endif

static void hid_endpoint_init(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer = hid_var->ep_in_buffer;
    usb_g_ep_config(usb_id, HID_MOUSE_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_INT, 0, ep_buffer, HID_MOUSE_EP_IN_MAX_SIZE);
    usb_g_set_sof_intr_hander(usb_id, HID_MOUSE_EP_IN | USB_DIR_IN, hid_sof_tx_data);
    spin_lock_init(&mouse_data_lock);

#if HID_MOUSE_EP_OUT_EN
    ep_buffer = hid_var->ep_out_buffer;
    if (ep_buffer) {
        usb_g_set_intr_hander(usb_id, HID_MOUSE_EP_OUT, hid_rx_data);
        usb_g_ep_config(usb_id, HID_MOUSE_EP_OUT, USB_ENDPOINT_XFER_INT, 1, ep_buffer, HID_MOUSE_EP_IN_MAX_SIZE);
    }
#endif
    usb_enable_ep(usb_id, HID_MOUSE_EP_OUT);
}

static void hid_reset(struct usb_device_t *usb_device, u32 itf)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_info("%s\n", __func__);
    hid_endpoint_init(usb_device, itf);
}
static void hid_suspend(struct usb_device_t *usb_device, u32 itf)
{
    hid_var->suspend = 1;
}
static u32 hid_recv_output_report(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    if (hid_var->output_report) {
        return hid_var->output_report(usb_device, setup);
    }
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ret = 0;
    u8 read_ep[8];
    u8 mute;
    u16 volume = 0;
    usb_read_ep0(usb_id, read_ep, MIN(sizeof(read_ep), setup->wLength));
    ret = USB_EP0_STAGE_SETUP;
    log_info_hexdump(read_ep, setup->wLength);

    return ret;
}

static u32 hid_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *req)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = req->bRequestType & USB_TYPE_MASK;
    switch (bRequestType) {
    case USB_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_DESCRIPTOR:
            switch (HIBYTE(req->wValue)) {
            case USB_HID_DT_HID:
                tx_payload = (u8 *)sHIDDescriptor + USB_DT_INTERFACE_SIZE;
                tx_len = 9;
                tx_payload = usb_set_data_payload(usb_device, req, tx_payload, tx_len);
                tx_payload[7] = LOBYTE(get_hid_report_desc_len(req->wIndex));
                tx_payload[8] = HIBYTE(get_hid_report_desc_len(req->wIndex));
                break;
            case USB_HID_DT_REPORT:
                hid_endpoint_init(usb_device, req->wIndex);
                tx_len = get_hid_report_desc_len(req->wIndex);
                tx_payload = (void *)get_hid_report_desc(req->wIndex);
                usb_set_data_payload(usb_device, req, tx_payload, tx_len);
                break;
            }// USB_REQ_GET_DESCRIPTOR
            break;
        case USB_REQ_SET_DESCRIPTOR:
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_SET_INTERFACE:
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                //只有一个interface 没有Alternate
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
            break;
        case USB_REQ_GET_INTERFACE:
            if (req->wLength) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                tx_payload[0] = 0x00;
                usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            }
            break;
        case USB_REQ_GET_STATUS:
            if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
            break;
        }//bRequest @ USB_TYPE_STANDARD
        break;

    case USB_TYPE_CLASS: {
        switch (req->bRequest) {
        case USB_REQ_SET_IDLE:
            hid_var->idle_speed = HIBYTE(req->wValue);
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            hid_var->suspend = 0;
            break;
        case USB_REQ_GET_IDLE:
            tx_len = 1;
            tx_payload[0] = hid_var->idle_speed;
            usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            break;
        case USB_REQ_SET_REPORT:
            usb_set_setup_recv(usb_device, hid_recv_output_report);
            break;
        case USB_REQ_GET_REPORT:
            //根据报告描述符,填写长度和数据
            tx_len = 0;
            /* tx_payload[0] = ; */
            usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            break;
        case USB_REQ_SET_PROTOCOL:
            hid_var->protocol = req->wValue;
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            break;
        case USB_REQ_GET_PROTOCOL:
            tx_len = 1;
            tx_payload[0] = hid_var->protocol;
            usb_set_data_payload(usb_device, req, tx_payload, tx_len);
            break;
        }//bRequest @ USB_TYPE_CLASS
    }
    break;
    }
    return 0;
}
void usb_hid_mouse_register(const usb_dev usb_id, void *p)
{
    hid_var = p;
    hid_var->suspend = 1;
}
u32 usb_hid_mouse_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    log_info("%s hid interface num:%d\n", __func__, *cur_itf_num);
    u8 *_ptr = ptr;

    memcpy(ptr, sHIDDescriptor, sizeof(sHIDDescriptor));
    ptr[2] = *cur_itf_num;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, hid_itf_hander) != *cur_itf_num) {
        ASSERT(0, "hid set interface_hander fail");
    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, hid_reset) != *cur_itf_num) {
        ASSERT(0, "hid set interface_reset_hander fail");
    }
    if (usb_set_suspend_hander(usb_id, *cur_itf_num, hid_suspend) != *cur_itf_num) {
        ASSERT(0, "hid set interface_reset_hander fail");
    }

    ptr[USB_DT_INTERFACE_SIZE + 7] = LOBYTE(get_hid_report_desc_len(0));
    ptr[USB_DT_INTERFACE_SIZE + 8] = HIBYTE(get_hid_report_desc_len(0));
    *cur_itf_num = *cur_itf_num + 1;
    return sizeof(sHIDDescriptor) ;
}

u32 usb_hid_mouse_send_data(const void *p, u32 len)
{
    if (hid_var->suspend) {
        return 0;
    }
    return hid_tx_data(NULL, p, len);
}
#endif
