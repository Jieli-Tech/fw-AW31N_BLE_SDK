#ifndef IMA_DECODE_API_H
#define IMA_DECODE_API_H

#include "audio_typedef.h"
#include "if_decoder_ctrl.h"

/*
   config 配置说明：CMD_SET_GOON_CALLBACK  //配置可读编码数据量回调函数
   使用方式 ima_test->dec_confing(ima_work_buf,CMD_SET_GOON_CALLBACK,&goon_dec_callback);
   其中goon_dec_callback 为Goon_DEC_CallBack。
   若配置了，每次读数之前回调，返回可读数据量。若返回0 则run 返回 MAD_ERROR_STREAM_NODATA;
   若没有配置，读不到数返回  MAD_ERROR_FILE_END;

   config 配置说明：CMD_SET_PRE_IDX   //配置每包预测值与步长表索引,在每包解码前配置
   使用方式 ima_test->dec_confing(ima_work_buf,CMD_SET_PRE_IDX,&prevalue_index_info)
   prevalue_index_info 是ima_dec_stream_pre_idx变量
   其中 prevalue_index_info.valpred = head[0] << 8 | head[1];
   prevalue_index_info.index = head[2];
   head是 unsigned char 类型
   注意：在配置之前需要暂停解码，配置之后才能开始解码（保证解码当前包开始，是使用的头中的valpred与index）
 */

#define CMD_SET_GOON_CALLBACK 0x95
#define CMD_SET_PRE_IDX		  0x96

// sdk 定义两个常量、两个常量的大小与buf大小有关，值越大，buf越大
// 例如 一包的编码数据为128(不包含头)，则ima_dec_stream_max_input 设置为128  ima_dec_stream_max_output可设置为32 64 128  256

extern const int ima_dec_stream_max_input; //解码读数buf大小,单位字节数。若是有头编码数据建议大小为一包编码数据大小
extern const int ima_dec_stream_max_output;  //解码输出的样点数buf大小，单位为样点数。大小设置不要超过 ima_dec_stream_max_input * 2 且ima_dec_stream_max_input * 2的整除数


typedef struct _Goon_DEC_CallBack_ {
    void *priv;
    int (*callback)(void *priv);
} Goon_DEC_CallBack;

typedef struct _ima_dec_stream_pre_idx {
    short valpred;
    char index;
} ima_dec_stream_pre_idx;

extern audio_decoder_ops *get_ima_adpcm_stream_ops();

/*
    调用说明:
    audio_decoder_ops *ima_test = get_ima_adpcm_stream_ops(); // 获取解码句柄
    bufsize = ima_test->need_dcbuf_size();   //获取buf大小
    ima_work_buf = malloc(bufsize)           //申请运行buf
    ima_test->open(ima_work_buf,&my_dec_io,0);//打开解码器
    其中my_dec_io  是 IF_DECODER_IO 变量    input 读数函数，读的是编码数据，不包含头
    ima_test->format_check(ima_work_buf); //格式检查，由于无格式数据流，返回必是0 可不调用
    while(1)
    {
       int ret = ima_test->run(ima_work_buf,0); //解码运行
    }
 */

#endif
