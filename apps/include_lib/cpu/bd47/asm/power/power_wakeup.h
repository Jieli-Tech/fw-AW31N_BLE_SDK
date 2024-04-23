#ifndef __POWER_WAKEUP_H__
#define __POWER_WAKEUP_H__

//=========================唤醒原因==================================
enum WAKEUP_REASON {
    PWR_WK_REASON_PLUSE_CNT_OVERFLOW,   	    //pcnt唤醒复位
    PWR_WK_REASON_P11,   					    //P11唤醒复位
    PWR_WK_REASON_LPCTMU,   				    //触摸唤醒复位
    PWR_WK_REASON_PORT_EDGE,				    //数字io输入边沿唤醒复位
    PWR_WK_REASON_ANA_EDGE,					    //模拟io输入边沿唤醒复位
    PWR_WK_REASON_VDDIO_LVD,				    //vddio lvd唤醒复位
    PWR_WK_REASON_WDT,				    		//vddio lvd看门狗唤醒复位

    PWR_WK_REASON_FALLING_EDGE_0,			//p33 index0  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_1,			//p33 index1  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_2,			//p33 index2  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_3,			//p33 index3  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_4,     	    //p33 index4  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_5,          //p33 index5  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_6,          //p33 index6  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_7,          //p33 index7  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_8,          //p33 index8  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_9,          //p33 index9  io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_10,         //p33 index10 io下降沿唤醒复位
    PWR_WK_REASON_FALLING_EDGE_11,         //p33 index11 io下降沿唤醒复位

    PWR_WK_REASON_RISING_EDGE_0,			//p33 index0  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_1,			//p33 index1  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_2,			//p33 index2  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_3,			//p33 index3  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_4,     	    //p33 index4  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_5,          	//p33 index5  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_6,          	//p33 index6  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_7,          	//p33 index7  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_8,          	//p33 index8  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_9,          	//p33 index9  io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_10,         	//p33 index10 io上升沿唤醒复位
    PWR_WK_REASON_RISING_EDGE_11,         	//p33 index11 io上升沿唤醒复位

    PWR_ANA_WK_REASON_FALLING_EDGE_LDOIN, 		//LDO5V下降沿唤醒复位
    PWR_ANA_WK_REASON_FALLING_EDGE_VBATCH,      //VBATCH下降沿唤醒复位
    PWR_ANA_WK_REASON_FALLINIG_EDGE_VBATDT,     //vbatdt下降沿唤醒复位
    PWR_ANA_WK_REASON_FALLINIG_EDGE_CHARGEFULL, //charge full下降沿唤醒复位

    PWR_ANA_WK_REASON_RISING_EDGE_LDOIN,        //LDO5V上降沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_VBATCH,       //VBATCH上降沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_VBATDT,     	//vbatdet上升沿唤醒复位
    PWR_ANA_WK_REASON_RISING_EDGE_CHARGEFULL,   //chargefull上升沿唤醒复位

    PINR_PND1_WKUP,

    PWR_RTC_WK_REASON_ALM, 					    //RTC闹钟唤醒复位
    PWR_RTC_WK_REASON_256HZ, 					//RTC 256Hz时基唤醒复位
    PWR_RTC_WK_REASON_64HZ, 					//RTC 64Hz时基唤醒复位
    PWR_RTC_WK_REASON_2HZ, 						//RTC 2Hz时基唤醒复位
    PWR_RTC_WK_REASON_1HZ, 						//RTC 1Hz时基唤醒复位

    PWR_WKUP_REASON_RESERVE = 63,

};

//
//
//                    platform_data
//
//
//
//******************************************************************
//每个滤波参数不一样
#define MAX_WAKEUP_PORT     8  //最大同时支持数字io输入个数
#define MAX_WAKEUP_ANA_PORT 0  //最大同时支持模拟io输入个数

typedef enum {
    RISING_EDGE = 1,	//支持
    FALLING_EDGE,		//支持
    BOTH_EDGE,			//支持
} P33_IO_WKUP_EDGE;

typedef enum {
    PORT_FLT_DISABLE,	//支持
    PORT_FLT_16us,		//支持
    PORT_FLT_128us,		//支持
    PORT_FLT_1ms,		//支持
    PORT_FLT_4ms,		//不支持
    PORT_FLT_16ms,		//不支持
} P33_IO_WKUP_FLT;

struct _p33_io_wakeup_config {
    u32 gpio;      						//唤醒io
    enum gpio_mode pullup_down_mode;    //输入上拉模式，数字输入、上拉输入、下拉输入
    P33_IO_WKUP_FLT filter;				//滤波参数
    P33_IO_WKUP_EDGE edge; 				//唤醒边沿条件
    void (*callback)(P33_IO_WKUP_EDGE edge);
};

struct _p33_io_wakeup_param {
};

struct _wakeup_param {
    struct _p33_io_wakeup_param *p33_io_wakeup_param_p;

};

//
//
//                    power_wakeup api
//
//
//
//******************************************************************

bool is_wakeup_source(enum WAKEUP_REASON index);

bool is_ldo5v_wakeup();

/*
 *@brief  判断是否为指定gpio唤醒
 *@param  gpio序号
 *@retval 	0: 不是该唤醒
 *@retval 	RISING_EDGE: 上升沿唤醒
 *@retval 	RISING_EDGE: 下降沿沿唤醒
 *@retval 	BOTH_EDGE: 	 双边沿沿唤醒
 */
u32 is_gpio_wakeup(u32 gpio);

/*
 *@brief  初始化一个唤醒口
 1.优先和长按复位复用
 2.所有通道都用完之后，若长按复位没有开启，则占用长按复位通道
 */
void p33_io_wakeup_port_init(const struct _p33_io_wakeup_config *const config);
void p33_io_wakeup_port_uninit(u32 gpio);

void p33_io_wakeup_filter(u32 gpio, P33_IO_WKUP_FLT filter);
void p33_io_wakeup_enable(u32 gpio, u32 enable);
void p33_io_wakeup_set_callback(u32 gpio, void (*callback)(P33_IO_WKUP_EDGE edge));

#endif
