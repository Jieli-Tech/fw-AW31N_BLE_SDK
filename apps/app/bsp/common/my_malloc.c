

#pragma bss_seg(".malloc.data.bss")
#pragma data_seg(".malloc.data")
#pragma const_seg(".malloc.text.const")
#pragma code_seg(".malloc.text")
#pragma str_literal_override(".malloc.text.const")

#include "app_config.h"
#include "my_malloc.h"

#define LOG_TAG_CONST       HEAP
#define LOG_TAG             "[malloc]"
#include "log.h"

/* void *pvPortMalloc( size_t xWantedSize ) */
/* u32 example(u32 dat) */
/* { */
/* if (dat & 0x80) { */
/* dat += 1; */
/* } else { */
/* dat <<= 1; */
/* } */
/* return dat; */
/* } */

const u16 configHEAP_BEST_SIZE = 100;
void mem_printf(void);

void *my_malloc(u32 size, mm_type xType)
{
    u32 rets;
    __asm__ volatile("%0 = rets" : "=r"(rets));

#if MY_MALLOC_SELECT
    void *res;
    /* return pvPortMalloc(size); */
    res =  pvPortMalloc(size, xType);
    if (NULL != res) {
        log_info(" malloc res 0x%x, size 0x%x,type 0x%x\n", (u32)res, size, xType);
        memset(res, 0, size);
#if MALLOC_INFO_PRINTF
        mem_malloc_info_printf((u32)res, size, xType);
        mem_printf();
#endif
    } else {
#if MALLOC_INFO_PRINTF
        mem_printf();
#endif
        ASSERT(0, "my_malloc null(free:0x%x), need 0x%x,type 0x%x\n,rets %08x\n", my_get_free_size(), size, xType, rets);
    }
    return res;
#else
    return __bt_malloc(size);
#endif
}

void *my_free(void *pv)
{
#if MY_MALLOC_SELECT
    if (NULL != pv) {
        vPortFree(pv);
#if MALLOC_INFO_PRINTF
        mem_free_info_printf((u32)pv);
#endif
    }
#else
    __bt_free(pv);
#endif
    return NULL;
}

int my_get_free_size(void)
{
#if MY_MALLOC_SELECT
    return xPortGetFreeHeapSize();
#else
    return __bt_get_free_size();
#endif
}

void my_malloc_init(void)
{
#if MY_MALLOC_SELECT
    u32 len = (u32)&_free_end[0] - (u32)&_free_start[0];
    log_info(" HEAP----: 0x%x; 0x%x\n", (u32)&_free_end[0], (u32)&_free_start[0]);
    ASSERT(len >= 512, "my_malloc limit >=512");
    memset((void *)&_free_start[0], 0, len);
    vPortInit((void *)&_free_start[0], len);
#else
#endif

#if TCFG_USER_BLE_ENABLE
    btctler_nv_memory_apply();//BT malloc放到my_malloc初始化
#endif
}

void xTaskResumeAll(void)
{
    ;
}


#if MALLOC_INFO_PRINTF

#define MAX_MALLOC_NUM  10
struct _mem_table {
    u32 addr;
    u32 size;
    u8  type;
    u8  active;
} _GNU_PACKED_;
static struct _mem_table mem_table[MAX_MALLOC_NUM] = {0};
static u8 mem_cnt = 0;

void mem_printf(void)
{
    log_info("addr\t\tsize\ttype\tactive\tremain");
    for (int i = 0; i < mem_cnt; i++) {
        log_info("0x%x\t0x%x\t%d\t%d\t0x%x", mem_table[i].addr, mem_table[i].size, mem_table[i].type, mem_table[i].active, ((u32)&_free_end[0] - mem_table[i].addr - mem_table[i].size));
    }
    log_char('\n');
}

void mem_malloc_info_printf(u32 addr, u32 size, mm_type xType)
{
    for (int i = 0; i < mem_cnt; i++) {
        if (addr == mem_table[i].addr) {
            mem_table[i].active += 1;
            return;
        }
    }
    mem_table[mem_cnt].addr = addr;
    mem_table[mem_cnt].size = size;
    mem_table[mem_cnt].type = xType;
    mem_table[mem_cnt].active += 1;
    mem_cnt++;
    if (mem_cnt > MAX_MALLOC_NUM) {
        log_info("%s() %d\n", __func__, __LINE__);
        while (1);
    }
}

void mem_free_info_printf(u32 addr)
{
    for (int i = 0; i < mem_cnt; i++) {
        if (addr == mem_table[i].addr) {
            mem_table[i].active -= 1;
        }
    }
}
#endif
