/*********************************************************************************************
    *   Filename        : lib_system_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 15:22

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/

#include "app_config.h"
#include "includes.h"


///打印是否时间打印信息
const int config_printf_time         = 1;

///打印printf\log\puts等接口控制关闭,代码优化
#ifdef CONFIG_DEBUG_ENABLE
const int config_debug_log_enable  = 1;
#else
const int config_debug_log_enable  = 0;
#endif

///异常中断，asser打印开启
#ifdef CONFIG_RELEASE_ENABLE
const char MM_ASSERT           = 0;//内存管理malloc内部断言
const int config_asser         = 0;
#else
const char MM_ASSERT           = 1;//内存管理malloc内部断言
const int config_asser         = 1;
#endif

const int config_system_info   = 0;

//TODO
#if 0
//开启异常处理模块的lite打印版本(为了省代码空间,下面的控制变量必须逐级开启增加打印)
//异常打印模块使能
const u8 config_exception_enable = 1;
//1.仅报显示触发单元的寄存器，报错后可以去到对应的异常单元取消对应的使能进行查看或者查看对应的emu文档
const u8 config_exception_unit_lite_lite_enable  = 1;
//1.仅报触发单元的id信息，报错后可以去到对应的异常单元取消对应的使能进行查看或者查看对应的emu文档
const u8 config_exception_unit_lite_enable       = 1;
//1.不报dev信息，报错后可以去到对应的异常单元取消对应的使能进行查看
const u8 config_exception_dev_lite_enable        = 1;
/* #else */
/* //异常打印模块使能 */
/* const u8 config_exception_enable = 0; */
/* //1.仅报显示触发单元的寄存器，报错后可以去到对应的异常单元取消对应的使能进行查看或者查看对应的emu文档 */
/* const u8 config_exception_unit_lite_lite_enable  = 0; */
/* //1.仅报触发单元的id信息，报错后可以去到对应的异常单元取消对应的使能进行查看或者查看对应的emu文档 */
/* const u8 config_exception_unit_lite_enable       = 0; */
/* //1.不报dev信息，报错后可以去到对应的异常单元取消对应的使能进行查看 */
/* const u8 config_exception_dev_lite_enable        = 0; */
/* #endif */
#endif

//================================================//
//                  SDFILE 精简使能               //
//================================================//
const int SDFILE_VFS_REDUCE_ENABLE = 1;

//================================================//
//                  dev使用异步读使能             //
//================================================//
#ifdef TCFG_DEVICE_BULK_READ_ASYNC_ENABLE
const int device_bulk_read_async_enable = 1;
#else
const int device_bulk_read_async_enable = 0;
#endif

//================================================//
//                  UI 							  //
//================================================//
const int ENABLE_LUA_VIRTUAL_MACHINE = 0;

//================================================//
//        不可屏蔽中断使能配置(UNMASK_IRQ)        //
//================================================//
const int CONFIG_CPU_UNMASK_IRQ_ENABLE = 0;

//================================================//
//0:使用timer0 delay; 1:使用mpwm3 delay,释放timer0//
//               only supprt: bd19	              //
//================================================//
const int set_to_close_timer0_delay = 0;

/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
/* const char log_tag_const_v_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_i_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_d_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_w_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */
/* const char log_tag_const_e_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */

const char log_tag_const_v_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

//FreeRTOS
const char log_tag_const_v_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_KTASK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_KTASK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_KTASK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_KTASK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_KTASK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

