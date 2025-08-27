#include "rdec_soft.h"
#include "spinlock.h"
#include "gpio.h"
/* #include "gpadc.h" */
#include "gpadc.h"
#include "gptimer.h"
#include "clock.h"

#define LOG_TAG_CONST       PERI
#define LOG_TAG             "[rdec_soft]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "debug.h"

#ifndef NO_CONFIG_PORT
#define NO_CONFIG_PORT  -1
#endif

struct rdec_soft_info {
    struct rdec_soft_config cfg;
    s8 cnt; //编码器计数值,带方向
    s8 cnt_last; //编码器计数值,带方向
    u8 phase_level; //AB相电平
    //bit7,bit6:无效信息
    //bit5,bit4:上上次读取到的AB相电平
    //bit3,bit2:上次读取到的AB相电平
    //bit1,bit0:最近一次读取到的AB相电平
    s8 dir; //旋转方向,用户不需要关注
}; //16Byte

/* #define _rs_info(x) (_rs_info_##x) */
#define RDEC_SOFT_MALLOC_ENABLE   0
#if RDEC_SOFT_MALLOC_ENABLE
#include "malloc.h"
#else
static struct rdec_soft_info _rs_info[RDEC_MAX_NUM];
#endif
static struct rdec_soft_info *g_rs_info[RDEC_MAX_NUM];
static u32 rdec_soft_malloc()
{
    u8 id = RDEC_SOFT_ERR_INIT_FAIL;
    for (u8 i = 0; i < RDEC_MAX_NUM; i++) { //分配g_rs_info id号
        if (g_rs_info[i] == NULL) {
            id = i;
            break;
        }
    }
    assert(id != RDEC_SOFT_ERR_INIT_FAIL, "func:%s, line:%d, rdec_soft malloc fail. need increase RDEC_MAX_NUM\n", __func__, __LINE__);

#if RDEC_SOFT_MALLOC_ENABLE
    g_rs_info[id] = (struct rdec_soft_config *)zalloc(sizeof(struct rdec_soft_config));
#else
    g_rs_info[id] = &_rs_info[id];
    assert(g_rs_info[id] != NULL, "func:%s(), line:%d, id:%  malloc fail!\n", __func__, __LINE__, id);
    memset(g_rs_info[id], 0, sizeof(struct rdec_soft_config));
#endif
    log_info("func:%s(),line:%d, rdec_soft:%d init success\n", __func__, __LINE__, id);
    return id;
}
static u32 rdec_soft_free(const u32 id)
{
    assert(g_rs_info[id], "%s()\n", __func__);
#if RDEC_SOFT_MALLOC_ENABLE
    free(g_rs_info[id]);
#else
    g_rs_info[id] = NULL;
#endif
    return 0;
}

static void rdec_soft_cfg_dump(struct rdec_soft_info *cfg)
{
    printf("func:%s, line:%d\n", __func__, __LINE__);
    printf("cfg->rdec.rdec_a: 0x%x\n", cfg->cfg.rdec_a);
    printf("cfg->rdec.rdec_b: 0x%x\n", cfg->cfg.rdec_b);
    printf("cfg->rdec.mode: %d\n", cfg->cfg.mode);
    printf("cfg->cfg.filter_us: %dus\n", cfg->cfg.filter_us);
}

static void phase_direction_check(struct rdec_soft_info *g_rs_info)
{
    s8 direction = 0;
    u8 data = g_rs_info->phase_level & 0b111111;
    switch (data) {
    case 0b001011:
    case 0b110100:
        direction = 1;
        g_rs_info->dir = direction;
        break;
    case 0b000111:
    case 0b111000:
        direction = -1;
        g_rs_info->dir = direction;
        break;
    default:
        break;
    }
    g_rs_info->cnt += direction;
    if (direction != 0) {
        gptimer_set_work_mode(g_rs_info->cfg.tid, GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE);
    }

}
static void phase_level_check(struct rdec_soft_info *g_rs_info, u8 stage)
{
    const u8 phase_level_last = g_rs_info->phase_level & 0b11;
    u8 phase_a_level;
    if (stage) {
        phase_a_level = phase_level_last & BIT(1);
    } else {
        phase_a_level = gpio_read(g_rs_info->cfg.rdec_a) << 1;
    }
    u8 phase_b_level = gpio_read(g_rs_info->cfg.rdec_b);
    u8 phase_level_cur = phase_a_level | phase_b_level;
    if (phase_level_cur == phase_level_last) {
        return;
    }

    g_rs_info->phase_level <<= 2;
    g_rs_info->phase_level |= phase_level_cur;

    if (phase_a_level != phase_b_level << 1) {
        return;
    }

    phase_direction_check(g_rs_info);
    switch (g_rs_info->cfg.mode) {
    case RDEC_PHASE_1:
        break;

    case RDEC_PHASE_2:
        //
        if (phase_level_cur == 0b11) {
            if (g_rs_info->cnt % 2) {
                g_rs_info->cnt -= g_rs_info->dir;
            } else {
            }
        }
        break;
    default:
        break;
    }

}

static void rdec_soft_irq(u32 tid, void *private_data)
{
    assert(private_data != NULL, "func:%s, line:%d, private_data is NULL.\n", __func__, __LINE__);
    struct rdec_soft_info *rs_info = (struct rdec_soft_info *)private_data;
    u8 timer_mode = gptimer_get_work_mode(tid);
    switch (timer_mode) {
    case GPTIMER_MODE_CAPTURE_EDGE_RISE:
    case GPTIMER_MODE_CAPTURE_EDGE_FALL:
    case GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE:
        phase_level_check(rs_info, 1);
        gptimer_set_timer_period(tid, rs_info->cfg.filter_us);
        gptimer_set_work_mode(tid, GPTIMER_MODE_TIMER);
        break;
    case GPTIMER_MODE_TIMER:
        phase_level_check(rs_info, 0);
        break;
    default:
        break;
    }
}


u32 rdec_soft_init(const struct rdec_soft_config *cfg)
{
    log_info("func:%s, line:%d\n", __func__, __LINE__);

    const struct gptimer_config rdec_capture_config = {
        .capture.filter = 0,
        .capture.port = (cfg->rdec_a / IO_GROUP_NUM),
        .capture.pin = BIT(cfg->rdec_a % IO_GROUP_NUM),
        .irq_cb = rdec_soft_irq,
        .irq_priority = 1,
        .mode = GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE,
        .private_data = NULL,
    };
    u32 tid = gptimer_init(cfg->tid, &rdec_capture_config);
    log_info("rdec_soft capture_tid = %d\n", tid);

    u8 id = rdec_soft_malloc();
    gptimer_set_private_data(tid, g_rs_info[id]);
    memcpy(&g_rs_info[id]->cfg, cfg, sizeof(struct rdec_soft_config));
    rdec_soft_cfg_dump(g_rs_info[id]);

    if (cfg->rdec_a != NO_CONFIG_PORT) {
        if (cfg->mode == RDEC_PHASE_2_ADC) {
            gpio_set_mode(IO_PORT_SPILT(cfg->rdec_a), PORT_INPUT_FLOATING);
        } else {
            gpio_set_mode(IO_PORT_SPILT(cfg->rdec_a), PORT_INPUT_PULLUP_10K);
        }
    }
    if (cfg->rdec_b != NO_CONFIG_PORT) {
        gpio_set_mode(IO_PORT_SPILT(cfg->rdec_b), PORT_INPUT_PULLUP_10K);
    }
    return id;
}

void rdec_soft_deinit(u32 id)
{
    log_info("func:%s, line:%d\n", __func__, __LINE__);
    assert(g_rs_info[id] != NULL, "func:%s, line:%d, rx_info[%d] is NULL.\n", __func__, __LINE__, id);
    gptimer_deinit(g_rs_info[id]->cfg.tid);
    gpio_set_mode(IO_PORT_SPILT(g_rs_info[id]->cfg.rdec_a), PORT_HIGHZ);
    gpio_set_mode(IO_PORT_SPILT(g_rs_info[id]->cfg.rdec_b), PORT_HIGHZ);
    memset(g_rs_info[id], 0, sizeof(struct rdec_soft_info));
    rdec_soft_free(id);
}

void rdec_soft_start(u32 id)
{
    log_info("func:%s, line:%d\n", __func__, __LINE__);
    assert(g_rs_info[id] != NULL, "func:%s, line:%d, rx_info[%d] is NULL.\n", __func__, __LINE__, id);
    u8 mode = g_rs_info[id]->cfg.mode;
    if ((mode == RDEC_PHASE_1) || (mode == RDEC_PHASE_2)) {
        phase_level_check(g_rs_info[id], 0);
        gptimer_set_work_mode(g_rs_info[id]->cfg.tid, GPTIMER_MODE_CAPTURE_EDGE_ANYEDGE);
    } else if (mode == RDEC_PHASE_2_ADC) {
        gptimer_set_work_mode(g_rs_info[id]->cfg.tid, GPTIMER_MODE_CAPTURE_EDGE_FALL);
    } else {
        log_info("rdec_cfg_mode error!\n");
    }
}
void rdec_soft_pause(u32 id)
{
    log_info("func:%s, line:%d\n", __func__, __LINE__);
    assert(g_rs_info[id] != NULL, "func:%s, line:%d, rx_info[%d] is NULL.\n", __func__, __LINE__, id);
    gptimer_pause(g_rs_info[id]->cfg.tid);
}

void rdec_soft_resume(u32 id)
{
    rdec_soft_start(id);
}

int rdec_soft_get_value(u32 id)
{
    assert(g_rs_info[id] != NULL, "func:%s, line:%d, rx_info[%d] is NULL.\n", __func__, __LINE__, id);
    log_debug("cnt:%d, cnt_last:%d\n", g_rs_info[id]->cnt, g_rs_info[id]->cnt_last);
    s16 cnt_diff = ((s16)g_rs_info[id]->cnt - (s16)g_rs_info[id]->cnt_last);
    s8 cnt_last = g_rs_info[id]->cnt_last;
    g_rs_info[id]->cnt_last = g_rs_info[id]->cnt;
    if (cnt_diff > 127) {
        cnt_diff = cnt_diff - 0x100;
    } else if (cnt_diff < -128) {
        cnt_diff = cnt_diff + 0x100;
    }
    log_debug("cnt_diff:%d\n", cnt_diff);
    if (g_rs_info[id]->cfg.mode == RDEC_PHASE_2) {
        if (cnt_diff % 2) {
            g_rs_info[id]->cnt_last = cnt_last;
            cnt_diff = 0;
        } else {
            cnt_diff /= 2;
        }
    }
    return (int)cnt_diff;
}
