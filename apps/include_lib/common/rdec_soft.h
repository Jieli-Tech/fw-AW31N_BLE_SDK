#ifndef __RDEC_SOFT_H__
#define __RDEC_SOFT_H__

#include "typedef.h"

#define RDEC_MAX_NUM                1 //软件rdec最大支持数量
#define RDEC_ADC_DEVIATION_BASE     30 //电压值小于 (vddio * 30%) 时,认为是低电平
#define RDEC_ADC_DEVIATION          5 //允许偏差 ±5%
#define RDEC_EXT_R1                 10000 //编码器外挂电阻阻值
#define RDEC_EXT_R2                 4700 //编码器外挂电阻阻值
#define RDEC_ADC_MAX_VALUE         ((1023 * (RDEC_EXT_R2) * (100+RDEC_ADC_DEVIATION)) / ((RDEC_EXT_R1+RDEC_EXT_R2) * 100 ))
#define RDEC_ADC_MIN_VALUE         ((1023 * (RDEC_EXT_R2) * (100-RDEC_ADC_DEVIATION)) / ((RDEC_EXT_R1+RDEC_EXT_R2) * 100 ))


typedef enum : u8 {
    RDEC_PHASE_1, //半码
    RDEC_PHASE_2, //全码
    RDEC_PHASE_2_ADC, //全码 adc单线采集 rdec_a配置IO, rdec_b配置-1
} rdec_mode;
enum rdec_soft_err : u8 {
    RDEC_SOFT_ERR_INIT_FAIL = 0xFF,
};
struct rdec_soft_config {
    u32 rdec_a; //编码器A相IO口
    u32 rdec_b; //编码器B相IO口
    u16 filter_us; //软件滤波参数,单位:us
    rdec_mode mode; //编码器工作模式
    const u8 tid; //使用的timer_id, 必须手动分配固定timer_id,不能使用内部自动分配
};

/**@brief 旋转编码器(软件方式)功能初始化
  * @param[in]  *cfg    rdec_soft_config定义的结构体指针
  * @return     tid:初始化成功分配的id号  -1:初始化失败
  */
u32 rdec_soft_init(const struct rdec_soft_config *cfg);

/**@brief 旋转编码器(软件方式)功能释放
  * @param[in]  id  初始化成功返回的id号
  * @return     无
  */
void rdec_soft_deinit(u32 id);

void rdec_soft_start(u32 id);
void rdec_soft_pause(u32 id);
void rdec_soft_resume(u32 id);

/**@brief 获取计数值
  * @param[in]  id  初始化成功返回的id号
  * @return     value   当前累计的计数值,带方向
  */
int rdec_soft_get_value(u32 id);

#endif


