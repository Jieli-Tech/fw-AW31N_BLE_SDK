/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "app_power_mg.h"
/* #include "update.h" */
/* #include "update_loader_download.h" */
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "asm/charge.h" */

#include "app_modules.h"
#if 0//TCFG_KWS_VOICE_RECOGNITION_ENABLE
#include "jl_kws/jl_kws_api.h"
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

#if 0//APP_BT_BLE 暂时
NOT_KEEP_RAM
u8 btNKRAM_static[BTCTLER_NK_RAM_SIZE] ALIGNED(4);
u8 pNVRAM_static[BTCTLER_NV_MEMORY_SIZE] ALIGNED(4);

const u16 btctler_nk_ram_size = BTCTLER_NK_RAM_SIZE;
const u16 btctler_nv_memory_size = BTCTLER_NV_MEMORY_SIZE;
#endif

struct application *main_application_operation_state(struct application *app, enum app_state state);

//*********************************************************************************//
//                                  蓝牙NVRAM配置                                  //
//                                  !!!禁止修改!!!                                 //
//*********************************************************************************//
NOT_KEEP_RAM
u8 btNKRAM_static[BTCTLER_NK_RAM_SIZE] ALIGNED(4);
u8 pNVRAM_static[BTCTLER_NV_MEMORY_SIZE] ALIGNED(4);

const u16 btctler_nk_ram_size = BTCTLER_NK_RAM_SIZE;
const u16 btctler_nv_memory_size = BTCTLER_NV_MEMORY_SIZE;
//*********************************************************************************//
//                                  中断优先级配置                                 //
//*********************************************************************************//
const int IRQ_IRTMR_IP        = 6;
const int IRQ_WFILE_IP        = 1;
const int IRQ_ADC_IP          = 1;
const int IRQ_TICKTMR_IP      = 3;
const int IRQ_USB_IP	      = 3;
const int IRQ_SD_IP		      = 3;
const int IRQ_CTMU_IP	      = 2;
const int IRQ_LEDC_IP         = 1;
const int IRQ_SLCD_IP         = 2;
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

    printf(">>>>>>>>>>>>>>>>>app_main...\n");
    printf(">>> v220,2022-11-23 >>>\n");

#if TCFG_SYS_LVD_EN
    app_power_vbat_check();
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
    const struct application *dev = NULL;

    init_intent(&it);
// 选择应用分支
#if CONFIG_APP_LE_TRANS
    it.name = "le_trans";
    it.action = ACTION_LE_TRANS_MAIN;

#elif CONFIG_APP_DONGLE
    it.name = "dongle";
    it.action = ACTION_DONGLE_MAIN;

#elif CONFIG_APP_IDLE
    it.name = "idle";
    it.action = ACTION_IDLE_MAIN;

#elif CONFIG_APP_NONCONN_24G
    it.name = "nonconn_24g";
    it.action = ACTION_NOCONN_24G_MAIN;

#elif CONFIG_APP_MULTI
    it.name = "multi_conn";
    it.action = ACTION_MULTI_MAIN;

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
#if CONFIG_APP_LE_TRANS
    it.name = "le_trans";
    it.action = ACTION_LE_TRANS_MAIN;

#elif CONFIG_APP_DONGLE
    it.name = "dongle";
    it.action = ACTION_DONGLE_MAIN;

#elif CONFIG_APP_IDLE
    it.name = "idle";
    it.action = ACTION_IDLE_MAIN;

#elif CONFIG_APP_NONCONN_24G
    it.name = "nonconn_24g";
    it.action = ACTION_NOCONN_24G_MAIN;

#elif CONFIG_APP_MULTI
    it.name = "multi_conn";
    it.action = ACTION_MULTI_MAIN;

#else
    while (1) {
        printf("no app!!!");
    }
#endif

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
    e.type = 0x0010;
    if (addr != NULL) {
        memcpy(e.u.bt.args, addr, 6);
    }
    e.arg  = (void *)type;
    e.u.bt.event = event;
    e.u.bt.value = value;

    main_application_operation_event(NULL, &e);
}


