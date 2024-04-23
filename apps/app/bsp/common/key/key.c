#include "key.h"
#include "msg.h"
#include "app_config.h"
#include "app_main.h"
#include "tick_timer_driver.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[key]"
#include "log.h"

extern struct application *main_application_operation_event(struct application *app, struct sys_event *event);
#if KEY_IO_EN
#include "key_drv_io.h"
#endif

#if KEY_MATRIX_EN
#include "key_drv_matrix.h"
#endif

#if KEY_AD_EN
#include "key_drv_ad.h"
#endif

static const key_interface_t *key_list[] = {
#if KEY_IO_EN
    &key_io_info,
#endif
#if KEY_AD_EN
    &key_ad_info,
#endif
#if KEY_MATRIX_EN
    &key_matrix_info,
#endif
};

static volatile uint8_t is_key_active = 0;
static volatile uint8_t key_poweron_flag = 0;
/* 按键驱动扫描参数列表 */
#if MOUSE_KEY_SCAN_MODE
struct key_driver_para key_scan_para = {
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 3,  			//按键判定长按数量
    .hold_time 		  = (3),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
};
#else
struct key_driver_para key_scan_para = {
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
};
#endif


//=======================================================//
// 按键值重新映射函数:
// 用户可以实现该函数把一些按键值重新映射, 可用于组合键的键值重新映射
//=======================================================//
int __attribute__((weak)) key_event_remap(struct sys_event *e)
{
    return true;
}

//=======================================================//
// 设置按键开机标志位
//=======================================================//
void set_key_poweron_flag(uint8_t flag)
{
    key_poweron_flag = flag;
}

//=======================================================//
// 获取按键开机标志位
//=======================================================//
uint8_t get_key_poweron_flag(void)
{
    return key_poweron_flag;
}

//=======================================================//
// 清除按键开机标志位
//=======================================================//
void clear_key_poweron_flag(void)
{
    key_poweron_flag = 0;
}


void key_init(void)
{
    key_puts("key init\n");

#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN)
    for (int i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++) {
        if (key_list[i]->key_init) {
            key_list[i]->key_init();
        }
    }
#endif

}

/*----------------------------------------------------------------------------*/
/**@brief   按键值获取函数
   @param
   @param
   @param
   @return  key_io_t类型按键参数，包含按键类型和键值
   @note
*/
/*----------------------------------------------------------------------------*/
static key_io_t get_key_value(void)
{
    key_io_t key;
    key.key_type = NO_KEY;

#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN)
    for (uint8_t i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++) {
        if (key_list[i]->key_get_value) {
            key.key_num = key_list[i]->key_get_value();

            if (NO_KEY != key.key_num) {
                key.key_type = key_list[i]->key_type;
                return key;
            }
        }
    }
#endif
    key.key_num = NO_KEY;
    return key;
}


//=======================================================//
// 按键扫描函数: 扫描所有注册的按键驱动
//=======================================================//
void key_driver_scan(void *_scan_para)
{
    struct key_driver_para *scan_para = (struct key_driver_para *)_scan_para;

    key_io_t key;
    uint8_t key_event = 0;
    uint8_t cur_key_value = NO_KEY;
    uint8_t key_value = 0;
    struct sys_event e;
    static uint8_t poweron_cnt = 0;

    //为了滤掉adkey与mic连在一起时电容充放电导致的开机按键误判,一般用于type-c耳机
    /* if (poweron_cnt <= 250) { */
    /*     poweron_cnt++; */
    /*     return; */
    /* } */
    key = get_key_value();
    cur_key_value = key.key_num;
    /* if (cur_key_value != NO_KEY) { */
    /*     printf(">>>cur_key_value: %d\n", cur_key_value); */
    /* } */

    if (cur_key_value != NO_KEY) {
        is_key_active = 35;      //35*10Ms
    } else if (is_key_active) {
        is_key_active --;
    }
//===== 按键消抖处理
    if (cur_key_value != scan_para->filter_value && scan_para->filter_time) {	//当前按键值与上一次按键值如果不相等, 重新消抖处理, 注意filter_time != 0;
        scan_para->filter_cnt = 0; 		//消抖次数清0, 重新开始消抖
        scan_para->filter_value = cur_key_value;	//记录上一次的按键值
        return; 		//第一次检测, 返回不做处理
    } 		//当前按键值与上一次按键值相等, filter_cnt开始累加;
    if (scan_para->filter_cnt < scan_para->filter_time) {
        scan_para->filter_cnt++;
        return;
    }
    //为了滤掉adkey与mic连在一起时电容充放电导致的按键误判,一般用于type-c耳机
    /* if ((cur_key_value != scan_para->last_key) && (scan_para->last_key != NO_KEY) && (cur_key_value != NO_KEY)) { */
    /*     return; */
    /* } */
//===== 按键消抖结束, 开始判断按键类型(单击, 双击, 长按, 多击, HOLD, (长按/HOLD)抬起)
    if (cur_key_value != scan_para->last_key) {
        if (cur_key_value == NO_KEY) {  //cur_key = NO_KEY; last_key = valid_key -> 按键被抬起

#if MOUSE_KEY_SCAN_MODE
            /* if (scan_para->press_cnt >= scan_para->long_time) {  //长按/HOLD状态之后被按键抬起; */
            key_event = KEY_EVENT_UP;
            key_value = scan_para->last_key;
            goto _notify;  	//发送抬起消息
            /* } */
#else
            if (scan_para->press_cnt >= scan_para->long_time) { //长按/HOLD状态之后被按键抬起;
                key_event = KEY_EVENT_UP;
                key_value = scan_para->last_key;
                goto _notify;  	//发送抬起消息
            }
#endif

            scan_para->click_delay_cnt = 1;  //按键等待下次连击延时开始
        } else {  //cur_key = valid_key, last_key = NO_KEY -> 按键被按下
            scan_para->press_cnt = 1;  //用于判断long和hold事件的计数器重新开始计时;
            if (cur_key_value != scan_para->notify_value) {  //第一次单击/连击时按下的是不同按键, 单击次数重新开始计数
                scan_para->click_cnt = 1;
                scan_para->notify_value = cur_key_value;
            } else {
                scan_para->click_cnt++;  //单击次数累加
            }
        }
        goto _scan_end;  //返回, 等待延时时间到
    } else { 	//cur_key = last_key -> 没有按键按下/按键长按(HOLD)
        if (cur_key_value == NO_KEY) {  //last_key = NO_KEY; cur_key = NO_KEY -> 没有按键按下
            if (scan_para->click_cnt > 0) {  //有按键需要消息需要处理

#ifdef ALL_KEY_EVENT_CLICK_ONLY//彩屏方案支持单击
                key_event = KEY_EVENT_CLICK;  //单击
                key_value = scan_para->notify_value;
                goto _notify;

#endif

#ifdef KEY_EVENT_CLICK_ONLY_SUPPORT 	//是否支持某些按键只响应单击事件
                if (scan_para->notify_value & BIT(7)) {  //BIT(7)按键特殊处理标志, 只发送单击事件, 也可以用于其它扩展
                    key_event = KEY_EVENT_CLICK;  //单击
                    key_value = scan_para->notify_value;
                    goto _notify;
                }
#endif
                if (scan_para->click_delay_cnt > scan_para->click_delay_time) { //按键被抬起后延时到
                    //TODO: 在此可以添加任意多击事件
                    if (scan_para->click_cnt >= 5) {
                        key_event = KEY_EVENT_FIRTH_CLICK;  //五击
                    } else if (scan_para->click_cnt >= 4) {
                        key_event = KEY_EVENT_FOURTH_CLICK;  //4击
                    } else if (scan_para->click_cnt >= 3) {
                        key_event = KEY_EVENT_TRIPLE_CLICK;  //三击
                    } else if (scan_para->click_cnt >= 2) {
                        key_event = KEY_EVENT_DOUBLE_CLICK;  //双击
                    } else {
                        key_event = KEY_EVENT_CLICK;  //单击
                    }
                    key_value = scan_para->notify_value;
                    goto _notify;
                } else {	//按键抬起后等待下次延时时间未到
                    scan_para->click_delay_cnt++;
                    goto _scan_end; //按键抬起后延时时间未到, 返回
                }
            } else {
                goto _scan_end;  //没有按键需要处理
            }
        } else {  //last_key = valid_key; cur_key = valid_key, press_cnt累加用于判断long和hold
            scan_para->press_cnt++;
            if (scan_para->press_cnt == scan_para->long_time) {
                key_event = KEY_EVENT_LONG;
            } else if (scan_para->press_cnt == scan_para->hold_time) {
                key_event = KEY_EVENT_HOLD;
                scan_para->press_cnt = scan_para->long_time;
            } else {
                goto _scan_end;  //press_cnt没到长按和HOLD次数, 返回
            }
            //press_cnt没到长按和HOLD次数, 发消息
            key_value = cur_key_value;
            goto _notify;
        }
    }

_notify:
    key_value &= ~BIT(7);  //BIT(7) 用作按键特殊处理的标志
    e.type = SYS_KEY_EVENT;
    e.u.key.init = 1;
    e.u.key.type = key.key_type;//区分按键类型
    e.u.key.event = key_event;
    e.u.key.value = key_value;


    scan_para->click_cnt = 0;  //单击次数清0
    scan_para->notify_value = NO_KEY;

    e.arg  = (void *)DEVICE_EVENT_FROM_KEY;
    /* printf("key_value: 0x%x, event: %d, key_poweron_flag: %d\n", key_value, key_event, key_poweron_flag); */
    if (key_poweron_flag) {
        if (key_event == KEY_EVENT_UP) {
            clear_key_poweron_flag();
            return;
        }
        return;
    }
    if (key_event_remap(&e)) {
        main_application_operation_event(NULL, &e);
    }
_scan_end:
    scan_para->last_key = cur_key_value;
    return;
}



#include "power_interface.h"
void key_active_set(P33_IO_WKUP_EDGE edge)
{
#if KEY_AD_EN || KEY_IO_EN || KEY_MATRIX_EN
    is_key_active = 35;      //35*10Ms
#endif
}
uint8_t key_idle_query(void)
{
    return !is_key_active;
}
REGISTER_LP_TARGET(key_lp_target) = {
    .name = "key",
    .is_idle = key_idle_query,
};

