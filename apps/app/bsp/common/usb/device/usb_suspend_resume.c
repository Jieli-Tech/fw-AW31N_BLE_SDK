#include "usb/device/usb_suspend_resume.h"
#include "usb/device/usb_stack.h"
#include "usb/otg.h"
#include "gpio.h"
#include "clock.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB_S&R]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define log_debug_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

static u8 usb_status = USB_SLAVE_SUSPEND;
static usb_status_hander status_hander;
void usb_slave_set_status_hander(usb_status_hander hander)
{
    status_hander = hander;
}
#if USB_SUSPEND_RESUME
static void usb_dp_wkup_cb(P33_IO_WKUP_EDGE edge)
{
    log_debug("func:%s()\n", __func__);
    usb_slave_resume(0);
}
#endif

void usb_slave_suspend_resume_init(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
#if USB_SUSPEND_RESUME
    usb_slave_dp_wakeup_init(usb_id, usb_dp_wkup_cb);
    usb_slave_dp_wakeup_disable(usb_id);
#endif
}
void usb_slave_suspend_resume_deinit(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
#if USB_SUSPEND_RESUME
    usb_slave_dp_wakeup_disable(usb_id);
    usb_slave_dp_wakeup_deinit(usb_id);
#endif
}
u8 usb_slave_status_get()
{
    return (u32)usb_status;
}
static void usb_slave_status_set(const usb_dev usb_id, usb_slave_status status)
{
    usb_status = status;
    if (status_hander) {
        status_hander(usb_id, status);
    }
}
void usb_slave_reset(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    usb_slave_status_set(usb_id, USB_SLAVE_RESUME);
#if USB_SUSPEND_RESUME
    usb_write_intr_usbe(usb_id, INTRUSB_RESET_BABBLE | INTRUSB_SUSPEND);
    u32 reg = usb_read_power(usb_id);
    usb_write_power(usb_id, (reg | INTRUSB_SUSPEND));
#endif
}
void usb_slave_suspend(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    usb_otg_suspend(usb_id, OTG_KEEP_STATE);
    usb_slave_status_set(usb_id, USB_SLAVE_SUSPEND);
#if USB_SUSPEND_RESUME
    usb_slave_phy_suspend(usb_id);
    usb_slave_dp_wakeup_enable(usb_id);
#endif
}
void usb_slave_resume(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    usb_slave_status_set(usb_id, USB_SLAVE_RESUME);
#if USB_SUSPEND_RESUME
    usb_slave_dp_wakeup_disable(usb_id);
    usb_slave_phy_resume(usb_id);
    struct usb_device_t *usb_device = usb_id2device(usb_id);
    usb_write_faddr(usb_id, usb_device->baddr);
    if (usb_device->baddr == 0) {
        usb_device->bDeviceStates = USB_DEFAULT;
    } else {
        usb_device->bDeviceStates = USB_CONFIGURED;
    }
#endif
    usb_otg_resume(usb_id);
}
void usb_remote_wakeup(const usb_dev usb_id)
{
#if USB_REMOTE_WAKEUP_EN
    if (usb_slave_status_get() == USB_SLAVE_SUSPEND) {
        usb_slave_resume(usb_id);
        u32 reg = usb_read_power(usb_id);
        usb_write_power(usb_id, reg | BIT(2));
        mdelay(8); //1ms~15ms
        usb_write_power(usb_id, reg);
    }
#endif
}

_WEAK_
void usb_slave_phy_suspend(const usb_dev usb_id)
{
    log_debug("__WEAK__ func:%s()\n", __func__);
}
_WEAK_
void usb_slave_phy_resume(const usb_dev usb_id)
{
    log_debug("__WEAK__ func:%s()\n", __func__);
}
void usb_slave_dp_wakeup_enable(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    p33_io_wakeup_enable(IO_PORT_DP, 1);
    putchar('{');
}
void usb_slave_dp_wakeup_disable(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    p33_io_wakeup_enable(IO_PORT_DP, 0);
    putchar('}');
}
static struct _p33_io_wakeup_config port_dp = {
    .gpio = IO_PORT_DP,
    .pullup_down_mode = PORT_INPUT_PULLUP_10K,
    .filter = PORT_FLT_1ms,
    .edge = FALLING_EDGE,
    .callback = NULL,
};
void usb_slave_dp_wakeup_init(const usb_dev usb_id, void (*dp_wkup_cb)(P33_IO_WKUP_EDGE edge))
{
    log_debug("func:%s()\n", __func__);
    if (dp_wkup_cb) {
        port_dp.callback = dp_wkup_cb;
    }
    p33_io_wakeup_port_init(&port_dp);
}
void usb_slave_dp_wakeup_deinit(const usb_dev usb_id)
{
    log_debug("func:%s()\n", __func__);
    p33_io_wakeup_port_uninit(IO_PORT_DP);
}

REGISTER_LP_TARGET(usb_slave_lp_target) = {
    .name = "usb_slave",
    .is_idle = usb_slave_status_get,
};
