#include "typedef.h"
#include "app_config.h"
#if USB_DEVICE_CLASS_CONFIG

#include "usb/device/usb_stack.h"
#include "usb/otg.h"
#include "usb/device/msd.h"
#include "usb/usb_config.h"

#define LOG_TAG "[MSG-UPDATE]"
#define LOG_INFO_ENABLE
#define LOG_ERROR_ENABLE
#include "log.h"

#define WRITE_FLASH                     0xFB
#define READ_FLASH                      0xFD
#define OTHER_CMD                       0xFC

typedef enum {
    UPGRADE_NULL = 0,
    UPGRADE_USB_HARD_KEY,
    UPGRADE_USB_SOFT_KEY,
    UPGRADE_UART_KEY,
} UPGRADE_STATE;

extern void nvram_set_boot_state(u32 state);
extern void ram_protect_close(void);

void go_mask_usb_updata()
{
    local_irq_disable();
    ram_protect_close();
    nvram_set_boot_state(UPGRADE_USB_SOFT_KEY);
    JL_CLOCK->PWR_CON |= (1 << 4);
    while (1);
}

#if TCFG_PC_UPDATE
u32 _usb_bulk_rw_test(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw);
u32 private_scsi_cmd(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw)
{
    /* if (_usb_bulk_rw_test(usb_device, cbw)) { */
    /*     return TRUE;                          */
    /* }                                         */

    switch (cbw->operationCode) {
//////////////////////Boot Loader Custom CMD
    case WRITE_FLASH:
    case READ_FLASH:
    case OTHER_CMD:
        log_info("goto mask pc mode\n");
        go_mask_usb_updata();
        break;

    default:

        return FALSE;
    }

    return TRUE;
}
#else
u32 private_scsi_cmd(const struct usb_device_t *usb_device, struct usb_scsi_cbw *cbw)
{
    return FALSE;
}
#endif
#endif
