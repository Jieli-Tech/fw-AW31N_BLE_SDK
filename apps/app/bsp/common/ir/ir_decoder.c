#include "ir_decoder.h"
#include "gptimer.h"
#include "jiffies.h"

#define LOG_TAG_CONST       PERI
#define LOG_TAG             "[ir_decode]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#include "debug.h"

#define NEC_NBITS           32
#define NEC_TIME_DEVIATION  10 //允许10%偏差
#define NEC_UNIT_TIME       563
#define NEC_HEADER_TIME     ((16 + 8) * NEC_UNIT_TIME)
#define NEC_BIT_0_TIME      ((1 + 1) * NEC_UNIT_TIME)
#define NEC_BIT_1_TIME      ((3 + 1) * NEC_UNIT_TIME)
#define NEC_REPEAT_TIME     ((16 + 4) * NEC_UNIT_TIME)
#define NEC_TIMEOUT_MS      150 //150ms

enum nec_data_type {
    NEC_ERROR = 0,
    NEC_HEAD,
    NEC_BIT_1,
    NEC_BIT_0,
    NEC_REPEAT,
};
struct ir_decode_t {
    u32 jiffies; //记录当前系统时间
    u32 ir_data; //存储接收到的红外数据,4*8bit = (cmd_not + cmd +addr_not + addr)
    u8 capture_tid; //用于捕获功能的tiemr_id
    u8 bit_count; //有效bit个数
    u8 command_count; //有效命令个数
};
static struct ir_decode_t _ir_decode;
static struct ir_decode_t *ir_decode = &_ir_decode;
static u32 get_cur_jiffies()
{
    return jiffies;
}
static u32 jiffies_timeout_check()
{
    u32 jiffies_cur = get_cur_jiffies();
    int jiffies_diff = jiffies_cur - ir_decode->jiffies;
    ir_decode->jiffies = jiffies_cur;
    //判断是否溢出
    if (jiffies_diff < 0) {
        jiffies_diff += 0xffffffff;
    }
    //判断两次下降沿间隔时间是否超时
    if (jiffies_to_msecs(jiffies_diff) >= NEC_TIMEOUT_MS) {
        return 1;
    } else {
        return 0;
    }
}
static u32 is_time_within(u32 t, u32 center, u32 deviation)
{
    if (__builtin_abs(((int)t - (int)center)) < (center * deviation / 100)) {
        return 1;
    } else {
        return 0;
    }
}
static u32 ir_decoder_pulse_check(u32 us)
{
    u32 type;
    if (is_time_within(us, NEC_HEADER_TIME, NEC_TIME_DEVIATION)) {
        type = NEC_HEAD;
    } else if (is_time_within(us, NEC_BIT_0_TIME, NEC_TIME_DEVIATION)) {
        type = NEC_BIT_0;
    } else if (is_time_within(us, NEC_BIT_1_TIME, NEC_TIME_DEVIATION)) {
        type = NEC_BIT_1;
    } else if (is_time_within(us, NEC_REPEAT_TIME, NEC_TIME_DEVIATION)) {
        type = NEC_REPEAT;
    } else {
        type = NEC_ERROR;
    }
    log_debug("us = %d, type = %d\n", us, type);
    return type;
}
static void ir_decoder_bit_count_check()
{
    if (ir_decode->bit_count == NEC_NBITS) {
        ir_decode->command_count++;
        log_debug("ir_data:0x%08x, command_count:%d\n", ir_decode->ir_data, ir_decode->command_count);
    }
}

static void ir_decode_irq(u32 tid, void *arg)
{
    u32 us = gptimer_get_capture_cnt2us(tid);
    //超时或第一个边沿直接退出
    if (jiffies_timeout_check()) {
        return;
    }
    u32 type = ir_decoder_pulse_check(us);

    switch (type) {
    case NEC_HEAD:
        ir_decode->bit_count = 0;
        ir_decode->command_count = 0;
        break;
    case NEC_BIT_1:
        ir_decode->ir_data >>= 1;
        ir_decode->ir_data |= 0x80000000;
        ir_decode->bit_count++;
        ir_decoder_bit_count_check();
        break;
    case NEC_BIT_0:
        ir_decode->ir_data >>= 1;
        ir_decode->bit_count++;
        ir_decoder_bit_count_check();
        break;
    case NEC_REPEAT:
        ir_decoder_bit_count_check();
        break;
    default:
        break;
    }
}

void ir_decoder_init(u32 gpio)
{
    log_info("%s()\n", __func__);
    memset(ir_decode, 0, sizeof(struct ir_decode_t));
    ir_decode->jiffies = get_cur_jiffies();

    const struct gptimer_config ir_capture_config = {
        .capture.filter = 0,//38000,
        .capture.max_period = 110 * 1000, //110ms
        .capture.port = (gpio / IO_GROUP_NUM),
        .capture.pin = BIT(gpio % IO_GROUP_NUM),
        .irq_cb = ir_decode_irq,
        .irq_priority = 3,
        .mode = GPTIMER_MODE_CAPTURE_EDGE_FALL,
        /* .mode = GPTIMER_MODE_CAPTURE_EDGE_RISE, */
    };
    ir_decode->capture_tid = gptimer_init(TIMERx, &ir_capture_config);
    log_info("ir_decode->capture_tid = %d\n", ir_decode->capture_tid);
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_INPUT_PULLUP_10K);//开上拉
    /* gpio_set_mode(IO_PORT_SPILT(gpio), PORT_INPUT_PULLDOWN_10K);//开下拉 */
    gptimer_start(ir_decode->capture_tid);
}

void ir_decoder_deinit()
{
    gptimer_deinit(ir_decode->capture_tid);
}

u32 ir_decoder_get_data(void)
{
    u32 data = -1;
    if (ir_decode->command_count) {
        ir_decode->command_count--;
        data = ir_decode->ir_data;
    }
    return data;
}
u32 ir_decoder_get_command_value(void)
{
    u32 data = ir_decoder_get_data();
    if (data == -1) {
        return -1;
    }
    u8 cmd = (data >> 16) & 0xff;
    u8 cmd_not = (data >> 24) & 0xff;
    if ((cmd ^ cmd_not) != 0xff) {
        cmd = -1;
    }
    return cmd;
}
u32 ir_decoder_get_command_value_uncheck(void)
{
    u32 data = ir_decoder_get_data();
    if (data == -1) {
        return -1;
    }
    u8 cmd = (data >> 16) & 0xff;
    return cmd;
}
u32 ir_decoder_get_address_value(void)
{
    u32 data = ir_decoder_get_data();
    if (data == -1) {
        return -1;
    }
    u8 addr = (data >> 0) & 0xff;
    u8 addr_not = (data >> 8) & 0xff;
    if ((addr ^ addr_not) != 0xff) {
        addr = (u8) - 1;
    }
    return addr;
}
u32 ir_decoder_get_address_value_uncheck(void)
{
    u32 data = ir_decoder_get_data();
    if (data == -1) {
        return -1;
    }
    u8 addr = (data >> 0) & 0xff;
    return addr;
}
void ir_decoder_dump()
{
    printf("header:%d, header_min:%d, header_max:%d\n",
           NEC_HEADER_TIME, (NEC_HEADER_TIME * (100 - NEC_TIME_DEVIATION) / 100), (NEC_HEADER_TIME * (100 + NEC_TIME_DEVIATION) / 100));
    printf("bit0:%d, bit0_min:%d, bit0_max:%d\n",
           NEC_BIT_0_TIME, (NEC_BIT_0_TIME * (100 - NEC_TIME_DEVIATION) / 100), (NEC_BIT_0_TIME * (100 + NEC_TIME_DEVIATION) / 100));
    printf("bit1:%d, bit1_min:%d, bit1_max:%d\n",
           NEC_BIT_1_TIME, (NEC_BIT_1_TIME * (100 - NEC_TIME_DEVIATION) / 100), (NEC_BIT_1_TIME * (100 + NEC_TIME_DEVIATION) / 100));
    printf("repeat:%d, repeat_min:%d, repeat_max:%d\n",
           NEC_REPEAT_TIME, (NEC_REPEAT_TIME * (100 - NEC_TIME_DEVIATION) / 100), (NEC_REPEAT_TIME * (100 + NEC_TIME_DEVIATION) / 100));
}
