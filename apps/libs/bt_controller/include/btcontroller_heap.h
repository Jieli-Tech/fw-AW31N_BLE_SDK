/*********************************************************************************************
    *   Filename        : btcontroller_heap.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-11-29 16:10

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/

#ifndef __BTCONTROLLER_HEAP_H
#define __BTCONTROLLER_HEAP_H

#if defined __cplusplus
extern "C" {
#endif

#include "typedef.h"

// #define BTCTLER_CLASSIC_STATIC_MEMORY_SIZE      (3*1024)
// #define BTCTLER_CLASSIC_DYNAMIC_MEMORY_SIZE     (512)

#define  BT_MEM_CLASSIC     BIT(0)
#define  BT_MEM_LE          BIT(1)
#define  BT_MEM_COMMON      BIT(2)


    enum {
        BT_CLASS_MEM_STATIC = 0,
        BT_CLASS_MEM_DYNAMIC,
    };

    void *bt_classic_malloc(u8 type, int size);

    void bt_classic_free(void *p);

    void btctler_memory_apply(u8 mode);

    void btctler_memory_release(u8 mode);

    void btctler_nv_memory_apply(void);

    void btctler_nv_memory_release(void);

    void *btctler_nv_memory_malloc(int size);

    void btctler_nv_memory_free(void *p);

    void *__bt_malloc(int size);

    void __bt_free(void *p);

    void *__bt_nk_malloc(int size);

    void __bt_nk_free(void *p);

#if defined __cplusplus
}
#endif

#endif // __btctler_HEAP_H
