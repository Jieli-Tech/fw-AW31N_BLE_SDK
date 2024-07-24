#ifndef __IR_DECODER_H__
#define __IR_DECODER_H__

#include "cpu.h"

/**@brief 红外解码功能初始化
  * @param[in]  gpio    接收引脚
  * @return     无
  */
void ir_decoder_init(u32 gpio);

/**@brief 红外解码功能释放
  * @param[in]  无
  * @return     无
  */
void ir_decoder_deinit();

/**@brief 获取红外原始数据
  * @param[in]  无
  * @return     红外原始数据,4*8bit = (命令反码 + 命令码 + 地址反码 +地址码)
  */
u32 ir_decoder_get_data(void);

/**@brief 获取红外命令码,内部校验
  * @param[in]  无
  * @return     红外命令码 cmd
  */
u32 ir_decoder_get_command_value(void);

/**@brief 获取红外命令码,内部不校验
  * @param[in]  无
  * @return     红外命令码 cmd
  */
u32 ir_decoder_get_command_value_uncheck(void);

/**@brief 获取红外地址码,内部校验
  * @param[in]  无
  * @return     红外地址码 addr
  */
u32 ir_decoder_get_address_value(void);

/**@brief 获取红外地址码,内部不校验
  * @param[in]  无
  * @return     红外地址码 addr
  */
u32 ir_decoder_get_address_value_uncheck(void);

#endif

