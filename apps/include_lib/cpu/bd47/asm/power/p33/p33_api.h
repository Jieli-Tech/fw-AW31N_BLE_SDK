#ifndef __P33_API_H__
#define __P33_API_H__


//
//
//					vol
//
//
//
/****************************************************************/

enum DVDD_VOL {
    DVDD_VOL_084V = 0,
    DVDD_VOL_087V,
    DVDD_VOL_090V,
    DVDD_VOL_093V,
    DVDD_VOL_096V,
    DVDD_VOL_099V,
    DVDD_VOL_102V,
    DVDD_VOL_105V,
    DVDD_VOL_108V,
    DVDD_VOL_111V,
    DVDD_VOL_114V,
    DVDD_VOL_117V,
    DVDD_VOL_120V,
    DVDD_VOL_123V,
    DVDD_VOL_126V,
    DVDD_VOL_129V,
};
/*
enum DVDD2_VOL {
};

enum RVDD_VOL {
};

enum RVDD2_VOL {
};

enum BTVDD_VOL {
};
*/
enum DCVDD_VOL {
    DCVDD_VOL_1000V = 0,
    DCVDD_VOL_1050V,
    DCVDD_VOL_1100V,
    DCVDD_VOL_1150V,
    DCVDD_VOL_1200V,
    DCVDD_VOL_1250V,
    DCVDD_VOL_1300V,
    DCVDD_VOL_1350V,
    DCVDD_VOL_1400V,
    DCVDD_VOL_1450V,
    DCVDD_VOL_1500V,
    DCVDD_VOL_1550V,
    DCVDD_VOL_1600V,
};

enum VDDIOM_VOL {
    VDDIOM_VOL_21V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_23V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_25V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_27V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_29V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_31V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_33V,
    VDDIOM_VOL_34V,
    VDDIOM_VOL_35V,
    VDDIOM_VOL_36V,
};

enum VDDIOW_VOL {
    VDDIOW_VOL_21V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_23V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_25V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_27V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_29V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_31V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_33V,
    VDDIOW_VOL_34V,
    VDDIOW_VOL_35V,
    VDDIOW_VOL_36V,
};

enum WVDD_VOL {
    WVDD_VOL_050V = 0,
    WVDD_VOL_055V,
    WVDD_VOL_060V,
    WVDD_VOL_065V,
    WVDD_VOL_070V,
    WVDD_VOL_075V,
    WVDD_VOL_080V,
    WVDD_VOL_085V,
    WVDD_VOL_090V,
    WVDD_VOL_095V,
    WVDD_VOL_100V,
    WVDD_VOL_105V,
    WVDD_VOL_110V,
    WVDD_VOL_115V,
    WVDD_VOL_120V,
    WVDD_VOL_125V,
};
/*
enum PVDD_VOL {
};

void pvdd_config(u32 lev, u32 low_lev, u32 output);

void pvdd_output(u32 output);
*/


void dvdd_vol_sel(enum DVDD_VOL vol);
enum DVDD_VOL get_dvdd_vol_sel();
//void dvdd2_vol_sel(enum DVDD2_VOL vol);
//enum DVDD2_VOL get_dvdd2_vol_sel();

//void rvdd_vol_sel(enum RVDD_VOL vol);
//enum RVDD_VOL get_rvdd_vol_sel();
//void rvdd2_vol_sel(enum RVDD2_VOL vol);
//enum RVDD2_VOL get_rvdd2_vol_sel();

void dcvdd_vol_sel(enum DCVDD_VOL vol);
enum DCVDD_VOL get_dcvdd_vol_sel();

//void btvdd_vol_sel(enum BTVDD_VOL vol);
//enum BTVDD_VOL get_btvdd_vol_sel();
//
//void pvdd_config(u32 lev, u32 low_lev, u32 output);
//void pvdd_output(u32 output);

void vddiom_vol_sel(enum VDDIOM_VOL vol);
enum VDDIOM_VOL get_vddiom_vol_sel();
void vddiow_vol_sel(enum VDDIOW_VOL vol);
enum VDDIOW_VOL get_vddiow_vol_sel();
u32 get_vddiom_vol();

//
//
//					lvd
//
//
//
/****************************************************************/
typedef enum {
    LVD_RESET_MODE,		//复位模式
    LVD_EXCEPTION_MODE, //异常模式，进入异常中断
    LVD_WAKEUP_MODE,     //唤醒模式，进入唤醒中断，callback参数为回调函数
} LVD_MODE;

typedef enum {
    VLVD_SEL_18V = 0,
    VLVD_SEL_19V,
    VLVD_SEL_20V,
    VLVD_SEL_21V,
    VLVD_SEL_22V,
    VLVD_SEL_23V,
    VLVD_SEL_24V,
    VLVD_SEL_25V,
} LVD_VOL;

void lvd_en(u8 en);
void lvd_config(LVD_VOL vol, u8 expin_en, LVD_MODE mode, void (*callback));

//
//
//                    pinr
//
//
//
//******************************************************************
void gpio_longpress_pin0_reset_config(u32 pin, u32 level, u32 time, u32 release, enum gpio_mode pullup_down_mode);
void gpio_longpress_pin1_reset_config(u32 pin, u32 level, u32 time, u32 release);



//
//
//                    dcdc
//
//
//
//******************************************************************
enum POWER_MODE {
    //LDO模式
    PWR_LDO15,
    //DCDC模式
    PWR_DCDC15,
};

enum POWER_DCDC_TYPE {
    PWR_DCDC12 = 2,
    PWR_DCDC18_DCDC12 = 6,
    PWR_DCDC18_DCDC12_DCDC09 = 7,
};

enum {
    DCDC09 = 1,
    DCDC12 = 2,
    DCDC18 = 4,
};

void power_set_dcdc_type(enum POWER_DCDC_TYPE type);
void power_set_mode(enum POWER_MODE mode);


//
//
//				mclr
//
//
//
/****************************************************************/
void p33_mclr_sw(u8 en);




#endif
