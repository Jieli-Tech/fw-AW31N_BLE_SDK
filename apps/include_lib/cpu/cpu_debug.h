#ifndef _CPU_DEBUG_H_
#define _CPU_DEBUG_H_
#include "typedef.h"

void emu_init();

//实时检测当前位置栈的深度
void stack_inquire(void);

//系统消耗栈初始化
bool stack_debug_free_check_init(void);
//查询已运行系统消耗栈的情况
void stack_debug_free_check_info(void);

//debug,检查程序调用位置是否中断模式
bool cpu_run_is_irq_mode(void);

void sdk_cpu_debug_loop_call(void);

void sdk_cpu_debug_main_init(void);

#endif

