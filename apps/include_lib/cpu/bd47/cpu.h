
#ifndef ASM_CPU_H
#define ASM_CPU_H


#include "bd47.h"
#include "typedef.h"
#include "csfr.h"
#include <stdarg.h>
#include "assert_d.h"

#ifndef __ASSEMBLY__
#define _G_va_list __gnuc_va_list
typedef _G_va_list va_list;

typedef unsigned char   		u8, bool, BOOL, uint8_t;
typedef char            		s8;
typedef signed char             int8_t;
typedef unsigned short  		u16;
typedef signed short    		s16;
typedef unsigned int    		u32;
typedef signed int      		s32;
typedef unsigned long long 		u64;
typedef unsigned long long int	uint64_t;
typedef u32						FOURCC;
typedef long long               s64;
typedef unsigned long long      u64;


#endif



#ifndef BIG_ENDIAN
#define BIG_ENDIAN 			0x3021
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 		0x4576
#endif
#define CPU_ENDIAN 			LITTLE_ENDIAN

#define CPU_CORE_NUM     1


#ifndef __ASSEMBLY__

#if CPU_CORE_NUM > 1
static inline int current_cpu_id()
{
    unsigned id;
    asm volatile("%0 = cnum" : "=r"(id) ::);
    return id ;
}
#else
static inline int current_cpu_id()
{
    return 0;
}
#endif

static inline int cpu_in_irq()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return flag & 0xff;
}

static inline int cpu_irq_disabled()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return (flag & 0x300) != 0x300;
}



static inline u32 rand32()
{
    return JL_RAND->R64L;
}


static inline void cpu_reset(void)
{
    void p33_soft_reset(void);
    p33_soft_reset();
}


#include "irq.h"

#ifndef EINVAL
#define EINVAL      22  /* Invalid argument */
#endif



extern void __local_irq_disable();
extern void __local_irq_enable();
extern void __local_irq_disable_hook(void);
extern void __local_irq_enable_hook(void);

extern void chip_reset();
extern int printf(const char *format, ...);
extern const int config_asser;

#define IRQ_HOOK_ENALBE    0

#if IRQ_HOOK_ENALBE
#define local_irq_disable  __local_irq_disable_hook
#define local_irq_enable   __local_irq_enable_hook
#else
#define local_irq_disable  __local_irq_disable
#define local_irq_enable   __local_irq_enable
#endif



#define	CPU_SR_ALLOC() 	\

#define __asm_csync() \
    do { \
		asm volatile("csync;"); \
    } while (0)



#define CPU_CRITICAL_ENTER()  local_irq_disable()

#define CPU_CRITICAL_EXIT()  local_irq_enable()

/*
#define ASSERT(a,...)   \
    do { \
        if(!(a)){ \
            if(config_asser){\
                printf("file:%s, line:%d", __FILE__, __LINE__); \
                printf("ASSERT-FAILD: "#a" "__VA_ARGS__); \
                local_irq_disable();\
                while (1);\
            }\
            else{\
                chip_reset(); \
            }\
        } \
    }while(0);

*/
#define arch_atomic_read(v)  \
    ({ \
     __asm_csync(); \
     (*(volatile int *)&(v)->counter); \
     })

#endif //__ASSEMBLY__


#endif

