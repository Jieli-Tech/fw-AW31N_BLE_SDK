/***********************************Jieli tech************************************************
  File : new_vm.h
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2022-12-26
********************************************************************************************/
#ifndef __NEW_VM_H__
#define __NEW_VM_H__

#include "typedef.h"


/*************************************************
 * newi vm cache
 * */
typedef struct __nvm_entry {
    u16 id;//vm id
    u16 rw_cnt;//访问次数
    u32 offset;//位置偏移
} NVM_ENTRY;

typedef struct __nvm_cache {
    u16 rw_cnt;//记录使用cache记录最新的读写值，用于刷新记录项的rw_cnt值
    u16 number_entry;//记录个数
    NVM_ENTRY *entries;//记录ram入口
} NVM_CACHE;

u32 nvm_cache_cnt(NVM_ENTRY *entries, u32 len);
u32 nvm_write_cache(NVM_CACHE *cache, u16 id, u32 offset);
u32 nvm_read_cache(NVM_CACHE *cache, u16 id);
u32 nvm_clear_cache(NVM_CACHE *cache);


/****************************************************/

#define BIT_MAP         (32 * 16) //512
#define BIT_MAP_SIZE    (BIT_MAP / 8) //64

#define NVM_MAX_LEN     128   //merge buffer

#define NVM_BUFF_SIZE   (NVM_MAX_LEN + BIT_MAP_SIZE)//128+64


// #define NVM_MULTIPLE_READ

typedef struct __new_vm_obj {
    void *device;//open handle
    NVM_CACHE *cache;//缓存 id 记录机制
    u32 addr;//vm 起始地址
    u32 reserve : 7;
    u32 bool_block : 1; //是否使用下半区B区去存储
    u32 block_size : 24;//总vm空间大小
    u32 area_len;//实际存储的空间大小
    u32 w_offset;//记录区域已写入数据长度
    u16 pre_sec_a;//已可写入的多个sec
    u16 pre_sec_b;//已可写入的多个sec
// #ifdef NVM_MULTIPLE_READ
    u16 id;//缓存支持多次读的id
    u16 offset;//记录读的位置
// #endif
} NEW_VM_OBJ;


/*********************************************************
 * 库接口
 * ******/
u32 nvm_init(NEW_VM_OBJ *p_nvm, u32 addr, u32 size);
u32 nvm_format_another(NEW_VM_OBJ *p_nvm);
u32 nvm_read(NEW_VM_OBJ *p_nvm, u32 id, u8 *buf, u32 len);
u32 nvm_write(NEW_VM_OBJ *p_nvm, u32 id, u8 *buf, u32 len);
void nvm_pre_erasure_next(NEW_VM_OBJ *p_nvm, u16 using_next, u16 idle_next);

u32 nvm_format_another_ignore(NEW_VM_OBJ *p_nvm, u32 *ignore_map, u32 ignore_bits);
u32 nvm_get_half_addr(NEW_VM_OBJ *p_nvm);
u32 nvm_get_half_len(NEW_VM_OBJ *p_nvm);
u32 nvm_get_cur_date_len(NEW_VM_OBJ *p_nvm);

/**********************************************************
 * 提供给库的回调函数
 * *****/
void *nvm_buf_for_lib(NEW_VM_OBJ *p_nvm, u32 *p_len);

/**********************************************************
 * 应用接口
 ******/
u32 nvm_init_api(u32 addr, u32 size);
u32 nvm_format_anotheri_api(void);
u32 nvm_read_api(u32 id, u8 *buf, u32 len);
u32 nvm_write_api(u32 id, u8 *buf, u32 len);
void nvm_erasure_next_api(void);
bool nvm_op_is_busy_api(void);

#endif

