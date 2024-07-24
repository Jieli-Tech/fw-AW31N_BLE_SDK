#include "ir_encoder.h"
#include "gptimer.h"
#include "asm/power_interface.h"
#include "app_config.h"

#define LOG_TAG_CONST       PERI
#define LOG_TAG             "[ir_encode]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "debug.h"


#define IR_NEC_PULSE_UNIT   563
#define IR_NEC_BIT_1_H  1
#define IR_NEC_BIT_1_L  3
#define IR_NEC_BIT_0_H  1
#define IR_NEC_BIT_0_L  1
#define IR_NEC_HEAD_H   16
#define IR_NEC_HEAD_L   8
#define IR_NEC_END_H    1
#define IR_NEC_END_L    1
#define IR_NEC_END_REPEAT_L    75//196
#define IR_NEC_REPEAT_H 16
#define IR_NEC_REPEAT_L 4
#define IR_NEC_REPEAT_END_H 1
#define IR_NEC_REPEAT_END_L 175
#define IR_NEC_REPEAT   0x00FF00FF

enum state : u8 {
    IR_IDLE,
    IR_HEAD,
    IR_DATA,
    IR_END,
    IR_REPEAT,
    IR_REPEAT_END,
};

enum : u8 {
    DATA_BIT0 = 0,
    DATA_BIT1,
    HEAD_BIT,
    END_BIT,
    REPEAT_BIT,
    REPEAT_END_BIT,
};

struct ir_encode_info {
    u32 ir_data; //存储发送的红外数据,4*8bit = (cmd_not + cmd +addr_not + addr)
    enum state state; //状态机,记录当前发送处于哪个阶段
    u8 pwm_tid; //用于pwm功能的timer_id
    u8 timer_tid; //用于timer功能的timer_id
    u8 high_level_count; //高电平持续count个单位时间
    u8 low_level_count; //低电平持续count个单位时间
    u8 repeat_en; //重复码使能标志
    u8 pwm_high_ctrl: 1; //高电平使能标志位,避免重复开关
    u8 pwm_low_ctrl: 1; //低平使能标志位,避免重复开关
    u8 bit_count: 6; //已发送bit个数
};

#define IR_ENCODER_MALLOC_ENABLE   0
#if IR_ENCODER_MALLOC_ENABLE
#include "malloc.h"
#else
static struct ir_encode_info _ir_encode;
#endif
static struct ir_encode_info *ir_encode;

static u32 ir_encoder_malloc()
{
#if IR_ENCODER_MALLOC_ENABLE
    ir_encode = (struct ir_encode_info *)malloc(sizeof(struct ir_encode_info));
#else
    ir_encode = &_ir_encode;
#endif
    memset(ir_encode, 0, sizeof(struct ir_encode_info));
    return 0;
}
static u32 ir_encoder_free()
{
    memset(ir_encode, 0, sizeof(struct ir_encode_info));
#if IR_ENCODER_MALLOC_ENABLE
    free(ir_encode);
#else
    ir_encode = NULL;
#endif
    return 0;
}


static u32 ir_pwm_ctrl()
{
    if (ir_encode->pwm_high_ctrl) {
        gptimer_pwm_enable(ir_encode->pwm_tid);
        ir_encode->pwm_high_ctrl = 0;
        ir_encode->high_level_count--;
    } else if (ir_encode->high_level_count) {
        ir_encode->high_level_count--;
    } else if (ir_encode->pwm_low_ctrl) {
        gptimer_pwm_disable(ir_encode->pwm_tid);
        ir_encode->pwm_low_ctrl = 0;
        ir_encode->low_level_count--;
    } else if (ir_encode->low_level_count) {
        ir_encode->low_level_count--;
    }

    if (ir_encode->low_level_count) {
        return 0;
    } else {
        return 1;
    }
}

static void ir_encode_tx_bit(u32 bit_state)
{
    switch (bit_state) {
    case HEAD_BIT:
        ir_encode->high_level_count = IR_NEC_HEAD_H;
        ir_encode->low_level_count = IR_NEC_HEAD_L;
        break;
    case DATA_BIT0:
        ir_encode->high_level_count = IR_NEC_BIT_0_H;
        ir_encode->low_level_count = IR_NEC_BIT_0_L;
        break;
    case DATA_BIT1:
        ir_encode->high_level_count = IR_NEC_BIT_1_H;
        ir_encode->low_level_count = IR_NEC_BIT_1_L;
        break;
    case END_BIT:
        ir_encode->high_level_count = IR_NEC_END_H;
        if (ir_encode->repeat_en) {
            ir_encode->low_level_count = IR_NEC_END_REPEAT_L;
        } else {
            ir_encode->low_level_count = IR_NEC_END_L;
        }
        break;
    case REPEAT_BIT:
        ir_encode->high_level_count = IR_NEC_REPEAT_H;
        ir_encode->low_level_count = IR_NEC_REPEAT_L;
        break;
    case REPEAT_END_BIT:
        ir_encode->high_level_count = IR_NEC_REPEAT_END_H;
        ir_encode->low_level_count = IR_NEC_REPEAT_END_L;
        break;
    default :
        break;
    }
    ir_encode->pwm_high_ctrl = 1;
    ir_encode->pwm_low_ctrl = 1;
}

static void ir_encode_irq(u32 tid, void *arg)
{
    u32 bit_state_change = ir_pwm_ctrl();
    //检查当前bit是否发送完成，切换下一个状态
    if (bit_state_change == 0) {
        return;
    }
    switch (ir_encode->state) {
    case IR_IDLE:
        ir_encode->state = IR_HEAD;
        break;
    case IR_HEAD:
        ir_encode_tx_bit(HEAD_BIT);
        ir_encode->state = IR_DATA;
        ir_encode->bit_count = 0;
        break;
    case IR_DATA:
        u32 bit_level = ir_encode->ir_data & BIT(0);
        ir_encode_tx_bit(bit_level);
        ir_encode->ir_data >>= 1;
        ir_encode->bit_count++;
        if (ir_encode->bit_count == 32) {
            ir_encode->state = IR_END;
        }
        break;
    case IR_END:
        ir_encode_tx_bit(END_BIT);
        ir_encode->state = IR_REPEAT;
        break;
    case IR_REPEAT:
        if (ir_encode->repeat_en) {
            ir_encode_tx_bit(REPEAT_BIT);
            ir_encode->state = IR_REPEAT_END;
        } else {
            gptimer_pause(ir_encode->timer_tid);
            ir_encode->state = IR_IDLE;
        }
        break;
    case IR_REPEAT_END:
        ir_encode_tx_bit(REPEAT_END_BIT);
        ir_encode->state = IR_REPEAT;
        break;
    default:
        break;
    }

}

void ir_encoder_init(u32 gpio, u32 freq, u32 duty)
{
    log_info("func:%s(), line:%d\n", __func__, __LINE__);
    ir_encoder_malloc();

    const struct gptimer_config pwm_config = {
        .pwm.freq = freq, //设置频率
        .pwm.port = (gpio / IO_GROUP_NUM), //设置输出IO
        .pwm.pin = BIT(gpio % IO_GROUP_NUM), //设置输出IO
        .pwm.duty = duty, //设置占空比
        .mode = GPTIMER_MODE_PWM, //设置工作模式
    };
#ifdef TCFG_IR_ENCODER_TIMER_TID
    ir_encode->pwm_tid = (u8)gptimer_init(TCFG_IR_ENCODER_TIMER_TID, &pwm_config); //用户配置固定timer
#else
    ir_encode->pwm_tid = (u8)gptimer_init(TIMERx, &pwm_config); //内部分配空闲timer
#endif
    log_info("pwm_config tid = %d\n", ir_encode->pwm_tid);
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);
    gptimer_start(ir_encode->pwm_tid);
    gptimer_pwm_disable(ir_encode->pwm_tid);

    const struct gptimer_config timer_config = {
        .timer.period_us = IR_NEC_PULSE_UNIT, //设置定时周期
        .irq_cb = ir_encode_irq, //设置中断回调函数
        .irq_priority = 3, //设置中断优先级
        .mode = GPTIMER_MODE_TIMER, //设置工作模式
    };
#ifdef TCFG_IR_ENCODER_PWM_TID
    ir_encode->timer_tid = (u8)gptimer_init(TCFG_IR_ENCODER_PWM_TID, &timer_config); //用户配置固定timer
#else
    ir_encode->timer_tid = (u8)gptimer_init(TIMERx, &timer_config); //内部分配空闲timer
#endif
    log_info("timer_config tid = %d\n", ir_encode->timer_tid);
}

void ir_encoder_deinit()
{
    log_info("func:%s(), line:%d\n", __func__, __LINE__);
    gptimer_deinit(ir_encode->pwm_tid);
    gptimer_deinit(ir_encode->timer_tid);
    ir_encode->state = IR_IDLE;
    ir_encoder_free();
}

u32 ir_encoder_tx(u8 ir_addr, u8 ir_cmd, u8 repeat_en)
{
    if (ir_encode->state != IR_IDLE) {
        return -1;
    }

    u32 cmd_not = (~ir_cmd & 0xFF) << 24;
    u32 addr_not = (~ir_addr & 0xFF) << 8;

    ir_encode->ir_data = cmd_not | (ir_cmd << 16) | addr_not | ir_addr;

    ir_encode->repeat_en = repeat_en;
    log_info("ir_data 0x%08x repeat_en : %d\n", ir_encode->ir_data, ir_encode->repeat_en);

    gptimer_start(ir_encode->timer_tid);

    return 0;
}

// 匹配海信电视红外发数控制
u32 ir_encoder_tx_hx(u8 ir_addr, u8 ir_cmd, u8 repeat_en)
{
    if (ir_encode->state != IR_IDLE) {
        return -1;
    }

    u32 cmd_not = (~ir_cmd & 0xFF) << 24;

    ir_encode->ir_data = cmd_not | (ir_cmd << 16) | (0xbf << 8) | ir_addr;

    ir_encode->repeat_en = repeat_en;
    log_info("ir_data 0x%08x repeat_en : %d\n", ir_encode->ir_data, ir_encode->repeat_en);

    gptimer_start(ir_encode->timer_tid);

    return 0;
}

void ir_encoder_repeat_stop()
{
    ir_encode->repeat_en = 0;
}

static u8 ir_encoder_status_get()
{
    if (ir_encode->state == IR_IDLE) {
        return 1;
    } else {
        return 0;
    }
}
REGISTER_LP_TARGET(ir_encoder_lp_target) = {
    .name = "ir_encoder",
    .is_idle = ir_encoder_status_get,
};
