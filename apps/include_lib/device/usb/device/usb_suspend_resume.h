#ifndef  __USB_SUSPEND_RESUME_H__
#define  __USB_SUSPEND_RESUME_H__

#include "typedef.h"
#include "usb.h"
#include "asm/power_interface.h"

#define USB_SUSPEND_RESUME      1   //usb从机休眠使能:TODO
#define USB_REMOTE_WAKEUP_EN    1
#if USB_REMOTE_WAKEUP_EN
#undef USB_SUSPEND_RESUME
#define USB_SUSPEND_RESUME      1
#endif

typedef enum {
    USB_SLAVE_RESUME = 0,
    USB_SLAVE_SUSPEND,
} usb_slave_status;

void usb_slave_suspend_resume_init(const usb_dev usb_id);
void usb_slave_suspend_resume_deinit(const usb_dev usb_id);
u8 usb_slave_status_get();
// void usb_slave_status_set(const usb_dev usb_id, usb_slave_status status);
void usb_slave_reset(const usb_dev usb_id);
void usb_slave_suspend(const usb_dev usb_id);
void usb_slave_resume(const usb_dev usb_id);
void usb_remote_wakeup(const usb_dev usb_id);
void usb_slave_dp_wakeup_enable(const usb_dev usb_id);
void usb_slave_dp_wakeup_disable(const usb_dev usb_id);
void usb_slave_dp_wakeup_init(const usb_dev usb_id, void (*dp_wkup_cb)(P33_IO_WKUP_EDGE edge));
void usb_slave_dp_wakeup_deinit(const usb_dev usb_id);

typedef void(*usb_status_hander)(const usb_dev usb_id, usb_slave_status status);
void usb_slave_set_status_hander(usb_status_hander hander); //注册的回调函数在中断调用

#endif  /*USB_STACK_H*/
