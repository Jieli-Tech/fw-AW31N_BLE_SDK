/*********************************************************************************************
    *   Filename        : ble_dg_central.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  :

    *   Copyright:(c)JIELI  2023-2035  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "app_config.h"
#include "app_action.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "le_common.h"
#include "bt_include/btstack_event.h"
#include "le_gatt_common.h"
#include "usb/usb_phy.h"
#include "usb/usb_std_class_def.h"
#include "sys_timer.h"
#include "ble_dg_central.h"
#include "app_dongle.h"
#include "app_power_mg.h"
#include "usb_suspend_resume.h"
#include "my_malloc.h"

#include "if_decoder_ctrl.h"
#include "ima_stream_decode_api.h"
#include "sbc_stddec_api.h"

#if (CONFIG_APP_DONGLE)

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[USB_ADO]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf
#define put_buf           printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#define put_buf(...)
#endif

/*
   1、config 配置说明：CMD_SET_GOON_CALLBACK  //配置可读编码数据量回调函数
   使用方式 ima_test->dec_confing(ima_work_buf,CMD_SET_GOON_CALLBACK,&goon_dec_callback);
   其中goon_dec_callback 为Goon_DEC_CallBack。
   若配置了，每次读数之前回调，返回可读数据量。若返回0 则run 返回 MAD_ERROR_STREAM_NODATA;
   若没有配置，读不到数返回  MAD_ERROR_FILE_END;


   2、config 配置说明：CMD_SET_PRE_IDX   //配置每包预测值与步长表索引,在每包解码前配置
   使用方式 ima_test->dec_confing(ima_work_buf,CMD_SET_PRE_IDX,&prevalue_index_info)
   prevalue_index_info 是ima_dec_stream_pre_idx变量
   其中 prevalue_index_info.valpred = head[0] << 8 | head[1];
   prevalue_index_info.index = head[2];
   head是 unsigned char 类型
   注意：在配置之前需要暂停解码，配置之后才能开始解码（保证解码当前包开始，是使用的头中的valpred与index）


   3、struct if_decoder_io  说明： lib_ima_stream_decode.a
   typedef struct ifdecoder_io{
   void *priv;               //私有结构，句柄
   int (*input)(void *priv u32 addr, void *buf,int len,u8 type);  // 输入回调函数，addr 文件位置（数据流传输不需要用），buf 读入地址，len 读入字节数。返回输入的字节数
   int(*check_buf)(void *priv,u32 addr,void *buf);                    //检查输入buf回调函数
   u32(*output)(void *priv,void *data,int len);                         //输出回调，返回已经输出的字节数
   u32(*get_lslen)(void *priv);                                                  //获取文件长度
   u32(*store_rev_data）(void *priv,u32 addr,int len)            //缓存数据回调函数
   };
 */

//解码读数buf大小,单位字节数。若是有头编码数据建议大小为一包编码数据大小
const int ima_dec_stream_max_input = 128;
//解码输出的样点数buf大小，单位为样点数。大小设置不要超过 ima_dec_stream_max_input * 2 且ima_dec_stream_max_input * 2的整除数
const int ima_dec_stream_max_output = 32;
// 例如 一包的编码数据为128(不包含头)，则ima_dec_stream_max_input 设置为128  ima_dec_stream_max_output可设置为32 64 128  256

static struct if_decoder_io  my_dec_io;
void dg_usb_audio_adpcm_init(void)
{
#if 0
    audio_decoder_ops *ima_test = get_ima_adpcm_stream_ops(); // 获取解码句柄
    int bufsize = ima_test->need_dcbuf_size();   //获取buf大小
    u8 *ima_work_buf = malloc(bufsize);           //申请运行buf
    ima_test->open(ima_work_buf, &my_dec_io, 0); //打开解码器
    //其中my_dec_io  是 IF_DECODER_IO 变量    input 读数函数，读的是编码数据，不包含头
    ima_test->format_check(ima_work_buf); //格式检查，由于无格式数据流，返回必是0 可不调用
    while (1) {
        int ret = ima_test->run(ima_work_buf, 0); //解码运行
    }
#endif
}


#endif


