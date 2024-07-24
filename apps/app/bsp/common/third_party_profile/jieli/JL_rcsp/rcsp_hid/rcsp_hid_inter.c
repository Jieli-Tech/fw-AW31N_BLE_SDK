#include "rcsp_hid_inter.h"
#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "usb/device/custom_hid.h"
#include "app_config.h"
#include "app_config.h"
#include "custom_cfg.h"
#include "app_modules.h"

#if CONFIG_APP_OTA_EN

#define LOG_TAG_CONST       UPDATE
//#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[APP-UPDATE]"
#include "log.h"

#pragma bss_seg(".update.bss.overlay")
#pragma data_seg(".update.data.overlay")

#ifndef RCSP_HID_UPDATE_TYPE
#define RCSP_HID_UPDATE_TYPE  0
#else
#undef RCSP_HID_UPDATE_TYPE
#define RCSP_HID_UPDATE_TYPE  1
#endif

static void (*rcsp_hid_recieve_callback)(void *priv, void *buf, u16 len) = NULL;

void rcsp_hid_recieve(void *priv, void *buf, u16 len)
{
    if (rcsp_hid_recieve_callback) {
        rcsp_hid_recieve_callback(priv, buf, len);
    }
}

bool JL_rcsp_hid_fw_ready(void *priv)
{
#if USB_DEVICE_CLASS_CONFIG & CUSTOM_HID_CLASS
    return custom_hid_get_ready(0) ? true : false;
#else
    return 0;
#endif
}

static int update_send_user_data_do(void *priv, void *data, u16 len)
{
#if (RCSP_BTMATE_EN && TCFG_PC_ENABLE && (USB_DEVICE_CLASS_CONFIG & HID_CLASS) && RCSP_HID_UPDATE_TYPE)
    //-------------------!!!!!!!!!!考虑关闭RCSP_BTMATE_EN使能编译报错
    extern void dongle_send_data_to_pc_3(u8 * data, u16 len);
    dongle_send_data_to_pc_3(data, len);
#endif
    return 0;
}

static int update_regiest_recieve_cbk(void *priv, void *cbk)
{
    log_info("%s, %x\n", __func__, cbk);
    rcsp_hid_recieve_callback = cbk;
    return 0;
}

static int update_regiest_state_cbk(void *priv, void *cbk)
{
    return 0;
}

static const struct rcsp_hid_operation_t rcsp_hid_update_operation = {
    .send_data = update_send_user_data_do,
    .regist_recieve_cbk = update_regiest_recieve_cbk,
    .regist_state_cbk = update_regiest_state_cbk,
};

void rcsp_hid_get_operation_table(struct rcsp_hid_operation_t **interface_pt)
{
    *interface_pt = (void *)&rcsp_hid_update_operation;
}

u8 rcsp_hid_auth_flag_get(void)
{
#if (0 == BT_CONNECTION_VERIFY)
    return JL_rcsp_get_auth_flag();
#else
    return 0;
#endif
}


#endif
