#include "OMSensor_manage.h"
#include "includes.h"
#include "app_config.h"
#include "cpu.h"
#include "app_main.h"
#include "sys_timer.h"

#ifdef TCFG_OMSENSOR_ENABLE

#define LOG_TAG             "[OMSENSOR]"
#include "log.h"

static const OMSENSOR_INTERFACE *OMSensor_hdl = NULL;

static s16 avg_filter(s16 *pdata, u8 num)
{
    u8 i = 0;
    s16 sum = 0;

    for (i = 0; i < num; i++) {
        sum += pdata[i];
    }

    return (sum / num);
}

extern struct application *main_application_operation_event(struct application *app, struct sys_event *event);
static void optical_mouse_sensor_event_to_usr(u8 event, s16 x, s16 y)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg = "omsensor_axis";
    e.u.axis.event = event;
    e.u.axis.x = x;
    e.u.axis.y = y;
    main_application_operation_event(NULL, &e);
}

static u8 optical_mouse_sensor_data_ready(void)
{
    u8 ret = 0;
    if (OMSensor_hdl->OMSensor_data_ready) {
        ret = OMSensor_hdl->OMSensor_data_ready();
    } else {
        ret = 0;
    }
    return ret;
}


void mouse_optical_sensor_event(u8 event, s16 x, s16 y);
void optical_mouse_sensor_read_motion_handler(void *priv)
{
    s16 x = 0, y = 0;

    if (optical_mouse_sensor_data_ready()) {
        s16 temp = 0;
        if (OMSensor_hdl->OMSensor_read_motion) {
            OMSensor_hdl->OMSensor_read_motion(&x, &y);
        }

        temp = x;
        x = y;
        y = temp;

        VECTOR_REVERS(x);
        VECTOR_REVERS(y);
        mouse_optical_sensor_event(0, x, y);
    }
}


u16 optical_mouse_sensor_set_cpi(u16 dst_cpi)
{
    u16 cpi = 0;
    log_info(">>>>>>>>>set cpi: %d\n", dst_cpi);
    if (OMSensor_hdl->OMSensor_set_cpi) {
        cpi = OMSensor_hdl->OMSensor_set_cpi(dst_cpi);
    }

    return cpi;
}


u8 get_optical_mouse_sensor_status(void)
{
    u8 ret = 0;

    if (OMSensor_hdl->OMSensor_status_dump) {
        ret =  OMSensor_hdl->OMSensor_status_dump();
    }

    return ret;
}

void optical_mouse_sensor_force_wakeup(void)
{
    if (OMSensor_hdl->OMSensor_wakeup) {
        OMSensor_hdl->OMSensor_wakeup();
    }
}

void optical_mouse_sensor_led_switch(u8 led_status)
{
    if (OMSensor_hdl->OMSensor_led_switch) {
        OMSensor_hdl->OMSensor_led_switch(led_status);
    }
}

bool optical_mouse_sensor_init(OMSENSOR_PLATFORM_DATA *priv)
{
    int retval = false;
    //查询驱动列表，匹配设备
    list_for_each_omsensor(OMSensor_hdl) {
        if (strncmp((const char *)OMSensor_hdl->OMSensor_id, (const char *)priv->OMSensor_id, sizeof(priv->OMSensor_id)) == 0) {
            retval = true;
            break;
        }
    }

    //匹配失败
    if (retval == false) {
        log_error("no match optical mouse sensor driver");
        return retval;
    }

    //初始化设备驱动
    retval = false;
    if (OMSensor_hdl->OMSensor_init) {
        retval = OMSensor_hdl->OMSensor_init(priv);
    }
    //设置optical mouse sensor的采样率
    if (retval == true) {
        log_info("optical mouse start>>>>>>>>>>>>>>>>>>>>");
        sys_s_hi_timer_add(NULL, optical_mouse_sensor_read_motion_handler, OPTICAL_SENSOR_SAMPLE_PERIOD);
    }
    return retval;
}

#endif


