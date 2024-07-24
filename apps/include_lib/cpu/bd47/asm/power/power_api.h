#ifndef __POWER_API_H__
#define __POWER_API_H__

#include "power_defined.h"

#define AT_VOLATILE_RAM_POWER        	AT(.power_driver.data)
#define AT_VOLATILE_RAM_BSS_POWER      	AT(.power_driver.data.bss)
#define AT_VOLATILE_RAM_CODE_POWER      AT(.power_driver.text.cache.L1)

#define AT_VOLATILE_RAM_LOWPOWER        AT(.power_driver.data.overlay)
#define AT_VOLATILE_RAM_BSS_LOWPOWER    AT(.power_driver.data.bss.overlay)
#define AT_VOLATILE_RAM_CODE_LOWPOWER	AT(.power_driver.text.cache.L1.overlay)

#define AT_VOLATILE_RAM_LOWPOWER_OSC        AT(.power_driver_osc.data.overlay)
#define AT_VOLATILE_RAM_BSS_LOWPOWER_OSC    AT(.power_driver_osc.data.bss.overlay)
#define AT_VOLATILE_RAM_CODE_LOWPOWER_OSC   AT(.power_driver_osc.text.cache.L1.overlay)

//
//
//                    platform_data
//
//
//
//******************************************************************

//osc_type
enum LOWPOWER_OSC_TYPE {
    OSC_TYPE_LRC,
    OSC_TYPE_BT_OSC,
    OSC_TYPE_NULL,
};

struct _power_param {
    //sleep
    u32 btosc_hz;			//蓝牙晶振频率(默认使用24M)
    u32 osc_delay_us;		//低功耗晶振起振延时，为预留配置。
    u32 t1;					//低功耗参数，预留配置
    u32 t2;					//低功耗参数，预留配置
    u32 t3;					//低功耗参数，预留配置
    u32 t4;					//低功耗参数，预留配置

    //power
    //vddiom\vddiow在进出低功耗时使用 VDDIO_KEEP_TYPE 配置
    u8 vddiom_lev;			//vddiom，系统工作时使用vddiom ldo(使用enum VDDIOM_VOL配置)
    u8 vddiow_lev;			//vddiow，系统低功耗时使用vddiow ldo(使用enum VDDIOW_VOL配置)

    //sleep
    u8 config;				//低功耗使能，蓝牙&&系统空闲可进入低功耗(使用LOWPOWER_CONFIG配置)
    u8 osc_type;			//低功耗晶振类型(使用enum LOWPOWER_OSC_TYPE配置)
    u8 lptmr_flow;			//低功耗参数由用户配置
};

struct _power_pdata {
    struct _power_param *power_param_p;
    struct _wakeup_param *wakeup_param_p;
};

//
//
//                    power_api
//
//
//
//******************************************************************

enum VDDIO_KEEP_TYPE {
    VDDIO_KEEP_TYPE_NULL,			//vddiow使用配置值
    VDDIO_KEEP_TYPE_NORMAL,		    //vddiow使用配置值使用vddiom挡位，即vddiom_lev
    VDDIO_KEEP_TYPE_TRIM,			//vddiow使用trim值
    VDDIO_KEEP_TYPE_PG,             //保持vddiom不关闭
};


#include "power_manage.h"

void power_early_init(u32 arg);

void power_later_init(u32 arg);

void power_init(struct _power_pdata *pdata);

enum PCONTROL_CMD {
    PCONTROL_POWER_DRIVER_RESERVE = 0,
    PCONTROL_P_PUTBYTE,						//串口调试函数

    //*****************************************************
    /* power
     */
    PCONTROL_POWER_MODE = 0x100,
    PCONTROL_DCVDD_CAP_SW,					//0：DCVDD上没有外挂电容	1：DCVDD上有外挂电容
    PCONTROL_FLASH_PG_VDDIO,				//0：FLASH电源引脚使用IO 	1：FLASH电源引脚没有使用IO
    PCONTROL_RTC_CLK,						//RTC_CLK类型，配置开机、关机晶振流程
    PCONTROL_POWER_SUPPLY,					//供电方式，0：IOVDD供电，1：VPWR供电

    //*****************************************************
    /* sleep
     */
    PCONTROL_PD_VDDIO_KEEP,					//pdown vddio切换流程(使用enum VDDIO_KEEP_TYPE配置)
    PCONTROL_PD_WDVDD_LEV,					//pdown wvdd挡位
    PCONTROL_PD_DVDD_LEV,					//pdown dvdd挡位
    PCONTROL_PD_KEEP_LPCTMU,				//pdown 触摸是否保持 0：不保持 1：保持

    //*****************************************************
    /* soff
     */
    PCONTROL_SF_KEEP_LRC,					//soff lrc是否保持 0：不保持 1：保持
    PCONTROL_SF_VDDIO_KEEP,					//soff vddio切换流程(使用enum VDDIO_KEEP_TYPE配置)
    PCONTROL_SF_KEEP_NVDD,					//soff nvdd是否保持 0：不保持 1：保持
    PCONTROL_SF_KEEP_PVDD,					//soff pvdd是否保持 0：不保持 1：保持

    //*****************************************************
    /* 以下配置为对应子模块的预留配置
     */
    PCONTROL_PHW_RESERVE = 0x100,		   //使用enum POWER_MODE配置
    PCONTROL_P33_RESERVE = 0x200,		   //使用PCONTROL_P33_CMD配置
    PCONTROL_P11_RESERVE = 0x300,		   //使用PCONTROL_P11_CMD配置
    PCONTROL_LP_FLOW_IC_RESERVE = 0x400,   //使用PCONTROL_IC_CMD配置
};

u32 power_control(enum PCONTROL_CMD cmd, u32 arg);

void lowpower_init();

void lowpower_uninit();

//
//
//                  lowpower
//
//
//
//******************************************************************
void low_power_sys_request(void *priv);

s32 low_power_trace_drift(u32 usec);

void low_power_reset_osc_type(u8 type);

u8 low_power_get_default_osc_type(void);

u8 low_power_get_osc_type(void);

void lowpower_dump();


//
//
//                    soff
//
//
//
//******************************************************************
#include "pmu_flag.h"

void power_set_soft_poweroff(void);

void mask_softflag_config(struct app_soft_flag_t *softflag);


#endif
