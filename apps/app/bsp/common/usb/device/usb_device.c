#include "app_config.h"
#include "usb/device/usb_stack.h"
#include "usb/usb_config.h"
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb/device/hid.h"
#include "usb/device/custom_hid.h"
#include "usb/device/cdc.h"
#include "usb/usb_config.h"
#include "usb/device/uac_audio.h"
#include "irq.h"
#include "gpio.h"
#include "usb_common_def.h"
#include "hwi.h"


#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "log.h"

static u8 otg_prevent_detect;

static void usb_device_init(const usb_dev usb_id)
{
    usb_device_config(usb_id);
    usb_g_sie_init(usb_id);
    usb_slave_init(usb_id);
    usb_set_dma_raddr(usb_id, 0, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 1, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 2, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 3, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 4, usb_get_ep_buffer(usb_id, 0));

    usb_write_intr_usbe(usb_id, INTRUSB_RESET_BABBLE | INTRUSB_SUSPEND);
    usb_clr_intr_txe(usb_id, -1);
    usb_clr_intr_rxe(usb_id, -1);
    usb_set_intr_txe(usb_id, 0);
    usb_set_intr_rxe(usb_id, 0);
    usb_g_isr_reg(usb_id, IRQ_USB_IP, 0);
    /* usb_sof_isr_reg(usb_id,3,0); */
    /* usb_sofie_enable(usb_id); */
    /* USB_DEBUG_PRINTF("ep0 addr %x %x\n", usb_get_dma_taddr(0), ep0_dma_buffer); */
}
static void usb_device_hold(const usb_dev usb_id)
{
    usb_g_hold(usb_id);
    usb_release(usb_id);
}


int usb_device_mode(const usb_dev usb_id, const u32 class)
{
    log_info("%s[class:0x%x]", __func__, class);
    /* usb_device_set_class(CLASS_CONFIG); */
    u8 class_index = 0;
    if (class == 0) {
        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DP + (usb_id * 2)), PORT_HIGHZ);
        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DM + (usb_id * 2)), PORT_HIGHZ);

        os_time_dly(20);

        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DP + (usb_id * 2)), PORT_INPUT_FLOATING);
        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DM + (usb_id * 2)), PORT_INPUT_FLOATING);

#if (USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS) == MASSSTORAGE_CLASS
        msd_release(usb_id);
#endif
        /* uac_release(usb_id); */
#if (USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS) == AUDIO_CLASS
        uac_release_api();
#endif
        usb_device_hold(usb_id);
        otg_prevent_detect = 0;
        return 0;
    }

    usb_add_desc_config(usb_id, MAX_INTERFACE_NUM, NULL);
    if ((class & AUDIO_CLASS) == AUDIO_CLASS) {
#if (USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS) == AUDIO_CLASS
        uac_register(&_uac_info);
        usb_add_desc_config(usb_id, class_index++, uac_audio_desc_config);
        log_info("add desc audio");
#else
        log_error("audio class not enable");
#endif
    } else if ((class & SPEAKER_CLASS) == SPEAKER_CLASS) {
#if (USB_DEVICE_CLASS_CONFIG & SPEAKER_CLASS) == SPEAKER_CLASS
        uac_register(&_uac_info);
        usb_add_desc_config(usb_id, class_index++, uac_spk_desc_config);
        log_info("add desc speaker");
#else
        log_error("speaker class not enable");
#endif
    } else if ((class & MIC_CLASS) == MIC_CLASS) {
#if (USB_DEVICE_CLASS_CONFIG & MIC_CLASS) == MIC_CLASS
        uac_register(&_uac_info);
        usb_add_desc_config(usb_id, class_index++, uac_mic_desc_config);
        log_info("add desc mic");
#else
        log_error("mic class not enable");
#endif
    }

    if ((class & CUSTOM_HID_CLASS) == CUSTOM_HID_CLASS) {
#if (USB_DEVICE_CLASS_CONFIG & CUSTOM_HID_CLASS) == CUSTOM_HID_CLASS
        custom_hid_register(0, &_custom_hid_var);
        usb_add_desc_config(usb_id, class_index++, custom_hid_desc_config);
        log_info("add desc std custom_hid\n");
#else
        log_error("custom_hid class not enable\n");
#endif
    }

    if ((class & HID_CLASS) == HID_CLASS) {
#if (USB_DEVICE_CLASS_CONFIG & HID_CLASS) == HID_CLASS
        //应用层添加宏控制,选择开哪些usb_interface
        /* usb_hid_media_register(0, usb_hid_media_init()); */
        /* usb_add_desc_config(usb_id, class_index++, usb_hid_media_desc_config); */
        usb_hid_mouse_register(0, usb_hid_mouse_init());
        usb_add_desc_config(usb_id, class_index++, usb_hid_mouse_desc_config);
        /* usb_hid_keyboard_register(0, usb_hid_keyboard_init()); */
        /* usb_add_desc_config(usb_id, class_index++, usb_hid_keyboard_desc_config); */
        log_info("add desc std hid\n");
#else
        log_error("hid class not enable\n");
#endif
    }

    /*     if ((class & MASSSTORAGE_CLASS) == MASSSTORAGE_CLASS) { */
    /* #if (USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS) == MASSSTORAGE_CLASS */
    /*         msd_register(&_msd_var); */
    /*         msd_register_disk_api(); */
    /*         usb_add_desc_config(usb_id, class_index++, msd_desc_config); */
    /*         log_info("add desc msd"); */
    /* #else */
    /*         log_error("msd class not enable"); */
    /* #endif */
    /*     } */
    /*  */
    /*     if ((class & IAP_CLASS) == IAP_CLASS) { */
    /* #if (USB_DEVICE_CLASS_CONFIG & IAP_CLASS) */
    /*         extern u32 iap_desc_config(const usb_dev usb_id, u8 * ptr, u32 * cur_itf_num); */
    /*         usb_add_desc_config(usb_id, class_index++, iap_desc_config); */
    /*         log_info("add desc iap"); */
    /* #else */
    /*         log_error("iap class not enable"); */
    /* #endif */
    /*     } */
    /*  */
    /*     if ((class & CDC_CLASS) == CDC_CLASS) { */
    /* #if (USB_DEVICE_CLASS_CONFIG & CDC_CLASS) */
    /*         cdc_register(usb_id); */
    /*         usb_add_desc_config(usb_id, class_index++, cdc_desc_config); */
    /*         log_info("add desc cdc"); */
    /* #else */
    /*         log_error("cdc class not enable"); */
    /* #endif */
    /*     } */

    usb_device_init(usb_id);
    /* #if (USB_DEVICE_CLASS_CONFIG & IAP_CLASS) */
    user_setup_filter_install(usb_id2device(usb_id));
    /* #endif */
    return 0;
}
/*
    * @brief otg检测中sof初始化，不要放在TCFG_USB_SLAVE_ENABLE里
     * @parm id usb设备号
      * @return 0 ,忽略sof检查，1 等待sof信号
       */
u32 usb_otg_sof_check_init(const usb_dev id)
{
    u32 ep = 0;
    u8 *ep_buffer = usb_get_ep_buffer(id, ep);

    usb_g_sie_init(id);

    usb_set_dma_raddr(id, ep, ep_buffer);

    for (ep = 1; ep < USB_MAX_HW_EPNUM; ep++) {
        usb_disable_ep(id, ep);
    }
    usb_sof_clr_pnd(id);
    return 1;
}
/* module_initcall(usb_device_mode); */

