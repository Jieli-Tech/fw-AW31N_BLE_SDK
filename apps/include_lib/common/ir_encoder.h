#ifndef  __IR_ENCODER_H__
#define  __IR_ENCODER_H__

#include "cpu.h"

/**@brief 红外编码功能初始化
  * @param[in]  gpio    发送引脚
  * @param[in]  freq    红外载波频率,NEC格式为38KHz
  * @param[in]  duty    红外载波占空比,满量程为10000
  * @return     无
  */
void ir_encoder_init(u32 gpio, u32 freq, u32 duty); //gpio:发送脚, freq:载波频率, duty:占空比,满量程10000

/**@brief 红外编码功能释放
  * @param[in]  无
  * @return     无
  */
void ir_encoder_deinit();

/**@brief 启动一次发送
  * @param[in]  ir_addr 红外地址码
  * @param[in]  ir_cmd  红外命令码
  * @param[in]  repeat_en  重复码发送使能
  * @return     0:启动发送成功  非0:发送失败
  */
u32 ir_encoder_tx(u8 ir_addr, u8 ir_cmd, u8 repeat_en); //addr:地址码, cmd:命令码, repeat_en:重复码发送使能

u32 ir_encoder_tx_hx(u8 ir_addr, u8 ir_cmd, u8 repeat_en);
/**@brief 停止发送重复码
  * @param[in]  无
  * @return     无
  */
void ir_encoder_repeat_stop();

#endif
