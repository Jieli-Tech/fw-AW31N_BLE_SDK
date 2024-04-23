/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Author          : Wangzhongqiang by Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2023-08-29 14:54

    *   Copyright:(c)JIELI  2023-2023  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "app_power_mg.h"
#include "app_modules.h"


#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */


struct application *main_application_operation_state(struct application *app, enum app_state state);

//*********************************************************************************//
//                                  蓝牙NVRAM配置                                  //
//                                  !!!禁止修改!!!                                 //
//*********************************************************************************//
//                                  蓝牙内存配置                                   //
NOT_KEEP_RAM
u8 btNKRAM_static[BTCTLER_NK_RAM_SIZE] ALIGNED(4);
u8 pNVRAM_static[BTCTLER_NV_MEMORY_SIZE] ALIGNED(4);

const u16 btctler_nk_ram_size = BTCTLER_NK_RAM_SIZE;
const u16 btctler_nv_memory_size = BTCTLER_NV_MEMORY_SIZE;

//*********************************************************************************//
//                                  中断优先级配置                                 //
//*********************************************************************************//
const int IRQ_IRTMR_IP        = 6;	//红外接收
const int IRQ_WFILE_IP        = 1;	//no use
const int IRQ_ADC_IP          = 1;	//adc
const int IRQ_TICKTMR_IP      = 3;	//tick_timer
const int IRQ_USB_IP	      = 3;	//usb
const int IRQ_SD_IP		      = 3;	//sd
const int IRQ_CTMU_IP	      = 2;	//no use
const int IRQ_LEDC_IP         = 1;	//no use
const int IRQ_SLCD_IP         = 2;	//no use
//BT
const int IRQ_BT_TIMEBASE_IP  = 6;   //BT TIMEBASE
const int IRQ_BLE_EVENT_IP    = 5;   //BT RX_EVT
const int IRQ_BLE_RX_IP       = 5;   //BT RX
const int IRQ_BTSTACK_MSG_IP  = 3;   //BT STACK
const int IRQ_BREDR_IP        = 3;   //no use
const int IRQ_BT_RXMCH_IP     = 3;   //no use
const int IRQ_AES_IP          = 3;   //aes

extern void timer_task_scan();
void app_timer_loop(void)
{
    timer_task_scan();
}

APP_VAR app_var;

void app_var_init(void)
{
    app_var.play_poweron_tone = 1;

    app_var.auto_off_time =  0; //TCFG_AUTO_SHUT_DOWN_TIME;
    app_var.warning_tone_v = 340;
    app_var.poweroff_tone_v = 330;
}

__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

void clr_wdt(void);

void app_main()
{
    //TODO
    /* if (!UPDATE_SUPPORT_DEV_IS_NULL()) { */
    /*     int update = 0; */
    /*     update = update_result_deal(); */
    /* } */

    printf("\n >>>>>>>>>>>>>>>>>app_main...\n");

#if TCFG_SYS_LVD_EN
    app_power_vbat_check();
#endif

#if TCFG_CHARGE_ENABLE
    set_charge_event_flag(1);
#endif

    main_application_operation_state(NULL, APP_STA_START);

}

/*
 * app模式切换
 */
void app_switch(const char *name, int action)
{
}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    /* log_info("100ms wakeup"); */
    return 1;
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}


struct application *main_application_operation_state(struct application *app, enum app_state state)
{
    struct intent it;
    const struct  application *dev = NULL;

    init_intent(&it);
    // 选择应用分支
#if(CONFIG_APP_KEYBOARD)
    it.name = "hid_key";
    it.action = ACTION_HID_MAIN;

#elif(CONFIG_APP_KEYFOB)
    it.name = "keyfob";
    it.action = ACTION_KEYFOB;

#elif(CONFIG_APP_KEYPAGE)
    it.name = "keypage";
    it.action = ACTION_KEYPAGE;

#elif(CONFIG_APP_REMOTE_CONTROL)
    it.name = "hid_rc";
    it.action = ACTION_REMOTE_CONTROL;

#elif(CONFIG_APP_MOUSE_SINGLE)
    it.name = "mouse_single";
    it.action = ACTION_MOUSE_MAIN;

#else
    while (1) {
        printf("no app!!!");
    }
#endif

    log_info("run app>>> %s", it.name);
    log_info("%s,%s", __DATE__, __TIME__);

    list_for_each_app_main(dev) {
        if (memcmp(dev->name, it.name, strlen(it.name)) == 0) {
            if (dev->ops) {
                dev->ops->state_machine(app, state, &it);
            }
        } else {
            printf("no app run");
        }
    }

    return NULL;
}

struct application *main_application_operation_event(struct application *app, struct sys_event *event)
{
    struct intent it;
    const struct application *dev = NULL;

    init_intent(&it);
    // 选择应用分支
#if(CONFIG_APP_KEYBOARD)
    it.name = "hid_key";
    it.action = ACTION_HID_MAIN;

#elif(CONFIG_APP_KEYFOB)
    it.name = "keyfob";
    it.action = ACTION_KEYFOB;

#elif(CONFIG_APP_KEYPAGE)
    it.name = "keypage";
    it.action = ACTION_KEYPAGE;

#elif(CONFIG_APP_REMOTE_CONTROL)
    it.name = "hid_rc";
    it.action = ACTION_REMOTE_CONTROL;

#elif(CONFIG_APP_MOUSE_SINGLE)
    it.name = "mouse_single";
    it.action = ACTION_MOUSE_MAIN;

#else
    while (1) {
        printf("no app!!!");
    }
#endif

    /* printf("run app>>> %s", it.name); */
    /* printf("%s,%s", __DATE__, __TIME__); */

    list_for_each_app_main(dev) {
        if (memcmp(dev->name, it.name, strlen(it.name)) == 0) {
            if (dev->ops) {
                dev->ops->event_handler(app, event);
            } else {
                printf("no event run");
            }
        }
    }

    return NULL;
}

void bt_event_update_to_user(u8 *addr, u32 type, u8 event, u32 value)
{
    printf("bt_event_update_to_user type:%d\n", type);

    struct sys_event e;
    e.type = SYS_BT_EVENT;
    if (addr != NULL) {
        memcpy(e.u.bt.args, addr, 6);
    }
    e.arg  = (void *)type;
    e.u.bt.event = event;
    e.u.bt.value = value;

    main_application_operation_event(NULL, &e);
}

