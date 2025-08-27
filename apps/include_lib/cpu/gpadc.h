#ifndef  __GPADC_H__
#define  __GPADC_H__

#include "gpadc_hw.h"

enum AD_MODE {
    AD_MODE_DEFAULT, //普通模式,存储在队列中为原始值
    AD_MODE_VOLTAGE, //电压模式,存储在队列中为电压值
};
enum BATTERY_MODE {
    MEAN_FILTERING_MODE = 0, //均值滤波
    WEIGHTING_MODE = 1, //加权求值
};

//以下为用户常用api
/**@brief 获取指定通道原始值,从队列中获取
  * @param[in]  ch      指定通道
  * @return     value   原始值,范围根据采样精度变化
  */
u32 adc_get_value(enum AD_CH ch);

/**@brief 获取指定通道电压值,从队列中获取
  * @param[in]  ch      指定通道
  * @return     voltage 电压值, 单位:mv
  */
u32 adc_get_voltage(enum AD_CH ch);

/**@brief 将原始值换算为电压值
  * @param[in]  adc_vbg     VBG原始值
  * @param[in]  adc_value   需要换算的原始值
  * @return     voltage     换算得到的电压值, 单位:mv
  */
u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_value);

/**@brief 阻塞式采集指定通道原始值,采集一次
  * @param[in]  ch      指定通道
  * @return     value   原始值,范围根据采样精度变化
  */
u32 adc_get_value_blocking(enum AD_CH ch);

/**@brief 阻塞式采集指定通道原始值,采集 sample_times次, 求平均值
  * @param[in]  ch              指定通道
  * @param[in]  sample_times    采样次数
  * @return     value           原始值,范围根据采样精度变化
  */
u32 adc_get_value_blocking_filter(enum AD_CH ch, u32 sample_times);

/**@brief 阻塞式dma模式采集指定通道原始值,采集 sample_times次,
  * @param[in]  ch              指定通道
  * @param[in]  buffer          dma缓冲区域
  * @param[in]  sample_times    采样次数
  * @return     value           sample_times次采样，去掉最大，最小值之后的ad值累加
  */
u32 adc_get_value_blocking_filter_dma(enum AD_CH ch, u32 *buffer, u32 sample_times);

/**@brief 阻塞式采集指定通道电压值,采集一次
  * @param[in]  ch      指定通道
  * @return     voltage 电压值, 单位:mv
  */
u32 adc_get_voltage_blocking(enum AD_CH ch);

/**@brief 阻塞式采集指定通道电压值,采集 sample_times次, 求平均值
  * @param[in]  ch              指定通道
  * @param[in]  sample_times    采样次数
  * @return     voltage 电压值, 单位:mv
  */
u32 adc_get_voltage_blocking_filter(enum AD_CH ch, u32 sample_times);

/**@brief 将IO通道转换为合法的AD通道 如:传入IO_PORTA_00, 返回AD_CH_IO_PA0
  * @param[in]  gpio    IO通道
  * @return     ch      合法的AD通道
  */
u32 adc_io2ch(int gpio);

/**@brief 添加指定通道到队采样队列中
  * @param[in]  ch  指定通道
  * @return     num 通道在队列中的编号
  */
u32 adc_add_sample_ch(enum AD_CH ch);

/**@brief 从队采样队列中删除指定通道
  * @param[in]  ch  指定通道
  * @return     num 通道在队列中的编号
  */
u32 adc_delete_ch(enum AD_CH ch);

/**@brief 设置指定通道的采样周期
  * @param[in]  ch  指定通道
  * @param[in]  ms  采样周期,单位:ms
  * @return     num 通道在队列中的编号
  */
u32 adc_set_sample_period(enum AD_CH ch, u32 ms);

/**@brief 设置指定通道的采样模式,初始化时设置,采样中设置需调用adc_reset刷新队列数据
  * @param[in]  ch      指定通道
  * @param[in]  mode    采样模式, 1:电压模式,队列中存储为电压值,单位:mv; 0:普通模式,队列中存储为原始值
  * @return     num     通道在队列中的编号
  */
u32 adc_set_voltage_mode(enum AD_CH ch, enum AD_MODE mode);
//以上为用户常用api


/**@brief ADC模块初始化
  * @param[in]  无
  * @return     无
  */
void adc_init(void);

/**@brief 触发一次adc队列采集,定时调用
  * @param[in]  无
  * @return     无
  */
void adc_scan(void *priv);

/**@brief ADC模块复位,将已注册的通道全部刷新一次
  * @param[in]  无
  * @return     无
  */
void adc_refresh(void);

/**@brief 打印ADC相关所有信息
  * @param[in]  无
  * @return     无
  */
void adc_dump(void);

/**@brief 获取电池电压
  * @param[in]  无
  * @return     电池电压值,单位:mv   注意:底层已做换算,不需要乘以倍数
  */
u32 gpadc_battery_get_voltage();

/**@brief 电池电压fifo 刷新
  * @param[in]  无
  * @return     无
  */
void gpadc_battery_refresh();

/**@brief 电池电压采集初始化
  * @param[in]  无
  * @return     0
  */
int gpadc_battery_init();






/**
 * @brief gpadc_get_ntc_temperature
 *
 * @return 芯片内置的ntc温度
 */
s32 gpadc_get_ntc_temperature();



















//兼容老版本芯片
u32 adc_check_vbat_lowpower();
void adc_update_vbg_value_restart(u8 cur_miovdd_level, u8 new_miovdd_level);


//gpadc_hw.c 实现
void adc_pmu_vbg_enable();
void adc_pmu_vbg_disable();
void adc_ana_ch_sel(enum AD_CH ch, u32 *_adc_con);
void adc_internal_signal_to_io(enum AD_CH analog_ch, u16 gpio);
// void adc_pmu_ch_select(u32 ch);
// void adc_audio_ch_select(u32 ch_sel);
void adc_adjust_div(void);
_INLINE_ u8 adc_get_clk_div();

//gpadc_hw_vxx.c 实现
void adc_close();
void adc_sample(enum AD_CH ch, u32 ie, u32 calibrate_en);
void adc_wait_enter_idle();
u32 adc_wait_pnd();
void adc_set_enter_idle();
u32 adc_get_res();
_INLINE_ u32 adc_idle_query();
_INLINE_ void adc_register_clear();
void adc_data_res_check();
void adc_suspend();
void adc_resume();


// CPU-----版本   对应表
// br27----gpadc_hw_v20
// br28----gpadc_hw_v10
// br29----gpadc_hw_v10
// br35----gpadc_hw_v40
// br36----gpadc_hw_v10
// br50----gpadc_hw_v10
// br52----gpadc_hw_v10
// br56----gpadc_hw_v11
// bd47----gpadc_hw_v12



#include "spinlock.h"
#if CPU_CORE_NUM > 1

#define gpadc_spin_lock(lock) \
	do { \
		q32DSP_testset(lock);\
	}while(0)

#define gpadc_spin_unlock(lock) \
	do{ \
		q32DSP_testclr(lock) ;\
	}while(0)

#else

#define gpadc_spin_lock(lock)

#define gpadc_spin_unlock(lock)

#endif


#endif

