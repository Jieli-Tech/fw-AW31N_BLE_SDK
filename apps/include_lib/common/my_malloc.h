#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include "typedef.h"


#define MALLOC_INFO_PRINTF     0

#define configUSE_MALLOC_FAILED_HOOK    0

#ifndef portPOINTER_SIZE_TYPE
#define portPOINTER_SIZE_TYPE u32
#endif

#define portBYTE_ALIGNMENT      4   //对齐规则


#if portBYTE_ALIGNMENT == 32
#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
#define portBYTE_ALIGNMENT_MASK ( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
#define portBYTE_ALIGNMENT_MASK ( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
#define portBYTE_ALIGNMENT_MASK ( 0x0000 )
#endif

#define vTaskSuspendAll()
#define traceMALLOC(...)
#define configASSERT       ASSERT

extern void xTaskResumeAll(void);


#define pdFALSE   0
#define pdTRUE    1

extern const char MM_ASSERT;


typedef enum _mm_type {
    MM_NONE = 0,
    MM_VFS,
    MM_SYDFS,
    MM_SYDFF,
    MM_NORFS,
    MM_NORFF,
    MM_FATFS,
    MM_FATFF,
    MM_FAT_TMP,
    MM_SRC,
    MM_MIO,
    MM_SWIN_BUF,
    MM_VFSCAN_BUF,
    MM_SCAN_BUF,
    MM_FF_APIS_BUF,
    MM_EQ,
    MM_CFG,
    MM_SYS_TMR,
} mm_type;


#ifndef traceFREE
#define traceFREE(pvAddress, uSize)
#endif

extern const u8 _free_start[];
extern const u8 _free_end[];

#define SYS_HEAP_MALLOC_SIZE    ((u32)&_free_end[0] - (u32)&_free_start[0])

void vPortInit(void *pAddr, uint32_t xLen);
// void *pvPortMalloc( size_t xWantedSize );

void *pvPortMalloc(size_t xWantedSize, mm_type type);
void vPortFree(void *pv);
void *my_malloc(u32 size, mm_type xType);
void *my_free(void *pv);
int my_get_free_size(void);
size_t xPortGetFreeHeapSize(void);
// void *my_malloc(u32 size);
void my_malloc_init(void);

#if MALLOC_INFO_PRINTF
void mem_printf(void);
void mem_malloc_info_printf(u32 addr, u32 size, mm_type xType);
void mem_free_info_printf(u32 addr);
#endif

//libs malloc api
extern const u8 _nk_ram_malloc_start[];
extern const u8 _nk_ram_malloc_end[];
extern const u8 _nv_ram_malloc_start[];
extern const u8 _nv_ram_malloc_end[];
extern const u8 _nk_ram_remain_start[];
extern const u8 nk_ram_end[];
extern const u8 _nv_ram_remain_start[];
extern const u8 nv_ram_end[];


#define   NK_RAM_MALLOC_START_ADDR   (void*)(_nk_ram_malloc_start)
#define   NK_RAM_MALLOC_SIZE         (int)(_nk_ram_malloc_end - _nk_ram_malloc_start)

#define   NV_RAM_MALLOC_START_ADDR   (void*)(_nv_ram_malloc_start)
#define   NV_RAM_MALLOC_SIZE         (int)(_nv_ram_malloc_end - _nv_ram_malloc_start)

#define   NK_RAM_REMAIN_SIZE         (int)(nk_ram_end - _nk_ram_remain_start)
#define   NV_RAM_REMAIN_SIZE         (int)(nv_ram_end - _nv_ram_remain_start)


void *__bt_malloc(int size);
void __bt_free(void *p);
void btctler_nv_memory_apply(void);

int __bt_nk_get_free_size(void);
int __bt_get_free_size(void);


#endif

