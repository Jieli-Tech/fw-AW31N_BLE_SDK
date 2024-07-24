#include <stdlib.h>
#include "cpu_debug.h"
#include "msg.h"
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_power_mg.h"
#include "clock.h"
#include "sys_timer.h"
#include "gpio.h"
#include "user_cfg.h"
#include "my_malloc.h"
#include "app_comm_proc.h"

#if (CONFIG_APP_IDLE)
#define LOG_TAG_CONST       APP_IDLE
#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#if (CONFIG_APP_IDLE)

#if TCFG_USER_BLE_ENABLE
//默认应用不需要打开配置,有需要自己添加
#error "confirm, need disable !!!!!!"
#endif
#endif

static volatile u8 timer_wakeup_id;

static void idle_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
    app_power_set_soft_poweroff(NULL);
}

static void idle_key_event_handler(struct sys_event *event)
{
    log_info("idle_key_evnet: %d,%d\n", event->u.key.event, event->u.key.value);

    uint8_t key_type = event->u.key.event;
    uint8_t key_value = event->u.key.value;

    if (key_type == KEY_EVENT_TRIPLE_CLICK
        && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) {
        idle_set_soft_poweroff();
        return;
    }
}

static void idle_timer_handle_test(void)
{
    log_info("wakeup");
}

static void idle_app_start()
{

    log_info("=======================================");
    log_info("---------idle demo---------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    clk_set("sys", TCFG_CLOCK_SYS_HZ);
    clk_set("lsb", TCFG_CLOCK_LSB_HZ);

    timer_wakeup_id = sys_timer_add(NULL, (void *)idle_timer_handle_test, 2000);

    int msg[4] = {0};
    while (1) {
        get_msg(sizeof(msg) / sizeof(int), msg);
        app_comm_process_handler(msg);
    }

}

static int idle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        idle_key_event_handler(event);
        return 0;
    case SYS_BT_EVENT:
        return 0;
    case SYS_DEVICE_EVENT:
        return 0;
    default:
        return false;
    }
}


static int idle_state_machine(struct application *app, enum app_state state,
                              struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_IDLE_MAIN:
            log_info("ACTION_IDLE_MAIN\n");
            /* os_taskq_flush(); */
            idle_app_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        sys_timer_del(timer_wakeup_id);// 关闭idle定时器
        break;
    }

    return 0;
}

static const struct application_operation app_idle_ops = {
    .state_machine  = idle_state_machine,
    .event_handler 	= idle_event_handler,
};

REGISTER_APPLICATION(app_app_idle) = {
    .name 	= "idle",
    .action	= ACTION_IDLE_MAIN,
    .ops 	= &app_idle_ops,
    .state  = APP_STA_DESTROY,
};

#endif

