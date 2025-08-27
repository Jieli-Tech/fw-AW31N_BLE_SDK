#include "msg.h"
#include "irq.h"
#include <stdarg.h>
#include "config.h"
#include "app_main.h"
#include "power_interface.h"

#define LOG_TAG             "[event_msg]"
#include "log.h"

// 全局消息数组池和标志数组
#define         POOL_SIZE           6 //数组池的最小临界值
NOT_KEEP_RAM
struct sys_event event_pool[POOL_SIZE];

// 用于跟踪池使用情况的位掩码
NOT_KEEP_RAM
uint16_t pool_usage = 0; // 每个位表示一个池条目的使用状态

// 初始化函数
void event_pool_init()
{
    // 清零 pool_usage 位掩码
    pool_usage = 0;

    // 使用 memset 清零 event_pool 数组
    memset(event_pool, 0, sizeof(event_pool));
}

// 分配函数
struct sys_event *event_pool_alloc()
{
    for (int i = 0; i < POOL_SIZE; i++) {
        if ((pool_usage & (1 << i)) == 0) { // 检查第i位是否为0
            pool_usage |= (1 << i); // 将第i位置1
            return &event_pool[i];
        }
    }
    ASSERT(0, "No free blocks in the pool\n");
    return NULL;
}

// 释放函数
void event_pool_free(struct sys_event *event_ptr)
{
    for (int i = 0; i < POOL_SIZE; i++) {
        if (&event_pool[i] == event_ptr) {
            pool_usage &= ~(1 << i); // 将第i位清零
            break;
        }
    }
}

// 检查是否可以进入低功耗状态
static u8 sys_event_lowpower_idle_query(void)
{
    u8 is_idle;
    if (pool_usage != 0) { // 如果消息池中有消息，不进入低功耗
        is_idle = 0;
    } else {
        is_idle = 1;
    }
    return is_idle;
}

REGISTER_LP_TARGET(sys_event_lowpower_target) = {
    .name = "sys_event_lowpwer_deal",
    .is_idle = sys_event_lowpower_idle_query,
};

static u8 sys_event_exit_sleep(void)
{
    event_pool_init();
    return 0;
}

DEEPSLEEP_TARGET_REGISTER(sys_event_sleep) = {
    .name   = "sys_event",
    .enter  = NULL,
    .exit   = sys_event_exit_sleep,
};
