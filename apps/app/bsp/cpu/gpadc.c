#include "gpadc.h"
#include "efuse.h"
#include "jiffies.h"
/* #include "timer.h" */

#define LOG_TAG_CONST       GPADC
#define LOG_TAG             "[GPADC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

static u8 adc_suspend_status;
static volatile u8 gpadc_lock = 0;
extern const u8 adc_vbat_ch_en;
extern const u8 adc_vtemp_ch_en;
extern const u8 adc_data_res; //adc 采样精度
#define ENABLE_PREEMPTIVE_MODE  1   //阻塞式采集使能
#define PMU_CH_SAMPLE_PERIOD    500 //PMU通道采样周期默认值 单位:ms
#define ADC_MAX_CH      8   //采集队列支持的最大通道数
#define ADC_FACTOR      16
#define ADC_FACTOR_OLD    ((u32)(ADC_FACTOR * 0.5))
#define ADC_FACTOR_NEW    (ADC_FACTOR - ADC_FACTOR_OLD)

struct adc_info_t { //adc采集队列信息结构体
    u32 jiffies;
    u32 ch;
    union {
        u32 value;
        u32 voltage;
    } v;
    u16 sample_period;
    u8 voltage_mode;
};
static struct adc_info_t adc_queue[ADC_MAX_CH + ENABLE_PREEMPTIVE_MODE];  //采集队列声明
static u8 cur_ch; //adc采集队列当前编号
static u16 adc_scan_timer_id = 0;

static u32 adc_ch2num(enum AD_CH ch)
{
    u32 i;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            break;
        }
    }
    return i;
}

static u32 adc_get_vbg_voltage()
{
    if (ADC_VBG_DATA_WIDTH == 0) {
        return ADC_VBG_CENTER;
    }
#if 0
    int data = efuse_get_gpadc_vbg_trim();
    int sign = (data >> ADC_VBG_DATA_WIDTH) & 0x01;
    data = data & ((1 << ADC_VBG_DATA_WIDTH) - 1);
    if (sign == 1) {
        return  ADC_VBG_CENTER - data * ADC_VBG_TRIM_STEP;
    } else {
        return  ADC_VBG_CENTER + data * ADC_VBG_TRIM_STEP;
    }
#endif
}

static u32 adc_value_update(enum AD_CH ch, u32 adc_value_old, u32 adc_value_new)
{
    adc_value_new *= ADC_FACTOR;
    return (adc_value_old * ADC_FACTOR_OLD + adc_value_new * ADC_FACTOR_NEW) / ADC_FACTOR;
}

static u32 adc_get_next_ch()
{
    if (cur_ch == ADC_MAX_CH) {
        cur_ch = 0;
    }
    for (int i = cur_ch; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch != -1) {
            if (adc_queue[i].sample_period) {
                if (time_before(adc_queue[i].jiffies, jiffies)) {
                    adc_queue[i].jiffies += adc_queue[i].sample_period;
                } else {
                    continue;
                }
            }

            return i;
        }
    }
    return ADC_MAX_CH;
}

static u32 gpadc_battery_voltage_process(u32 value);
static u32 adc_cpu_mode_process(u32 adc_value)    //adc_isr 中断中，cpu模式的公共处理函数
{
    if (adc_queue[cur_ch].ch == AD_CH_PMU_VBAT) {
        gpadc_battery_voltage_process(adc_value);
        if (AD_CH_PMU_VBAT == AD_CH_IOVDD) {
            adc_value = (0XFFFF >> (16 - adc_data_res));
        }
    }
    if (adc_queue[cur_ch].voltage_mode == AD_MODE_VOLTAGE) {
        u32 vbg_value = adc_queue[0].v.value;
        u32 voltage = adc_value_to_voltage(vbg_value, adc_value);
        adc_queue[cur_ch].v.voltage = adc_value_update(adc_queue[cur_ch].ch, adc_queue[cur_ch].v.voltage, voltage);
        /* printf("%d ad[%x]: %d %d", cur_ch, adc_queue[cur_ch].ch, adc_value, adc_queue[cur_ch].v.voltage); */
    } else  if (adc_queue[cur_ch].ch == AD_CH_LDOREF) {
        /* adc_queue[0].v.value = adc_value_update(AD_CH_LDOREF, adc_queue[0].v.value, adc_value); */
        if (adc_value == 0) {
            log_error("adc_cpu_mode_process vbg = 0\n");
        } else {
            adc_queue[0].v.value = adc_value;
        }
    } else {
        adc_queue[cur_ch].v.value = adc_value;
    }
    /* printf("%d ad[%x]: %d %d", cur_ch, adc_queue[cur_ch].ch, adc_value, adc_queue[cur_ch].v.voltage); */
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        adc_close();
        return 1;
    }
    return 0;
}
___interrupt
static void adc_irq()   //中断函数
{
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        adc_close();
        return;
    }

    u32 adc_value = adc_get_res();
    if (adc_value == -1) {
        return;
    }
    if (adc_cpu_mode_process(adc_value)) {
        return;
    }
    cur_ch++;
    if (cur_ch == ADC_MAX_CH) {
        adc_close();
        return;
    }
    cur_ch = adc_get_next_ch();
    if (cur_ch == ADC_MAX_CH) {
        adc_close();
    } else {
        if (adc_queue[cur_ch].ch == AD_CH_IOVDD) {
            adc_sample(AD_CH_LDOREF, 1, 0);
        } else {
            adc_sample(adc_queue[cur_ch].ch, 1, 0);
        }
        /* adc_sample(adc_queue[cur_ch].ch, 1, 0); */
    }
}
void adc_scan(void *priv)
{
    if (adc_suspend_status == 1) {
        return;
    }
    gpadc_spin_lock(&gpadc_lock);

    if (adc_queue[ADC_MAX_CH].ch != -1) {
        gpadc_spin_unlock(&gpadc_lock);
        return;
    }
    if (adc_idle_query()) {
        gpadc_spin_unlock(&gpadc_lock);
        return;
    }
    cur_ch = adc_get_next_ch();
    if (cur_ch == ADC_MAX_CH) {
        gpadc_spin_unlock(&gpadc_lock);
        return;
    }
    adc_sample(adc_queue[cur_ch].ch, 1, 1);

    gpadc_spin_unlock(&gpadc_lock);
}
static void adc_hw_init(void)
{
    adc_suspend();
    adc_data_res_check();
    adc_register_clear();
    adc_pmu_vbg_enable();

    memset(adc_queue, 0xff, sizeof(adc_queue));
    adc_adjust_div();

    adc_add_sample_ch(AD_CH_LDOREF);
    adc_set_voltage_mode(AD_CH_LDOREF, AD_MODE_DEFAULT);
    if (adc_vbat_ch_en || adc_vtemp_ch_en) {
        adc_set_sample_period(AD_CH_LDOREF, PMU_CH_SAMPLE_PERIOD);
    } else {
        adc_set_sample_period(AD_CH_LDOREF, -1);
    }

    adc_add_sample_ch(AD_CH_PMU_VBAT);
    adc_set_voltage_mode(AD_CH_PMU_VBAT, AD_MODE_VOLTAGE);
    if (adc_vbat_ch_en) {
        adc_set_sample_period(AD_CH_PMU_VBAT, PMU_CH_SAMPLE_PERIOD);
    } else {
        adc_set_sample_period(AD_CH_PMU_VBAT, -1);
    }

    adc_add_sample_ch(AD_CH_PMU_VTEMP);
    adc_set_voltage_mode(AD_CH_PMU_VTEMP, AD_MODE_VOLTAGE);
    if (adc_vtemp_ch_en) {
        adc_set_sample_period(AD_CH_PMU_VTEMP, PMU_CH_SAMPLE_PERIOD);
    } else {
        adc_set_sample_period(AD_CH_PMU_VTEMP, -1);
    }

    adc_queue[0].v.value = adc_get_value_blocking_filter(AD_CH_LDOREF, 16);
    printf("LDOREF = %d\n", adc_get_value(AD_CH_LDOREF));

    adc_queue[1].v.voltage = adc_get_voltage_blocking(AD_CH_PMU_VBAT) * ADC_FACTOR;
    printf("vbat = %d mv\n", adc_get_voltage(AD_CH_PMU_VBAT)*AD_CH_PMU_VBAT_DIV);

    adc_queue[2].v.voltage = adc_get_voltage_blocking(AD_CH_PMU_VTEMP) * ADC_FACTOR;
    printf("vtemp = %d mv\n", adc_get_voltage(AD_CH_PMU_VTEMP));

    gpadc_battery_init();

    adc_resume();
    /* request_irq(IRQ_GPADC_IDX, 3, adc_irq, 0);//注册中断函数 */
    /*  */
    /* usr_timer_add(NULL, adc_scan, 5, 0); */

    /* adc_add_sample_ch(adc_io2ch(IO_PORTB_00)); */
    /* void gpadc_test(void *priv); */
    /* usr_timer_add(NULL, gpadc_test, 1000, 0); */
    /* local_irq_disable(); */
    /* adc_suspend(); */
    /* adc_internal_signal_to_io(AD_CH_PMU_VBG, IO_PORTC_03); */
    /* adc_resume(); */
    /* while (1) { */
    /*     wdt_clear(); */
    /*     udelay(1000*1000); */
    /*     #<{(| gpadc_test(NULL); |)}># */
    /* } */
}
static void gpadc_test(void *priv)
{
    extern void adc_test_func();
    adc_test_func();
    printf("vbat_value:%d\n", adc_get_value_blocking(AD_CH_PMU_VBAT)*AD_CH_PMU_VBAT_DIV);
    printf("vbat:%dmv\n", adc_get_voltage(AD_CH_PMU_VBAT)*AD_CH_PMU_VBAT_DIV);
    printf("vbat:%dmv\n", adc_get_voltage_blocking(AD_CH_PMU_VBAT)*AD_CH_PMU_VBAT_DIV);
    printf("battery:%dmv\n", gpadc_battery_get_voltage());
}
void adc_suspend()
{
    if (adc_suspend_status == 0) {
        unrequest_irq(IRQ_GPADC_IDX);
        adc_suspend_status = 1;
    }
    adc_close();
}
void adc_resume()
{
    if (adc_suspend_status == 1) {
        request_irq(IRQ_GPADC_IDX, 3, adc_irq, 0);//注册中断函数
        adc_suspend_status = 0;
    }
}
u32 adc_get_value(enum AD_CH ch)
{
    for (int i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            return adc_queue[i].v.value;
        }
    }
    return 1;
}
u32 adc_get_voltage(enum AD_CH ch)
{
    for (int i = 0; i < ADC_MAX_CH; i++) {
        if (ch == adc_queue[i].ch && adc_queue[i].voltage_mode == AD_MODE_VOLTAGE) {
            if (ch == AD_CH_PMU_VBAT) {
            }
            return adc_queue[i].v.voltage / ADC_FACTOR;
        }
    }

    u32 adc_vbg = adc_get_value(AD_CH_PMU_VBG);
    u32 adc_value;
    if (ch == AD_CH_IOVDD) {
        adc_value = (0XFFFF >> (16 - adc_data_res));
    } else {
        adc_value = adc_get_value(ch);
    }
    return adc_value_to_voltage(adc_vbg, adc_value);
}
u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_value)
{
    u32 tmp = adc_get_vbg_voltage();
    if (adc_vbg == 0) {
        log_error("func:%s(), line:%d, adc_vbg = 0\n", __func__, __LINE__);
        adc_vbg = adc_get_value(AD_CH_LDOREF);
    }
    /* if (adc_vbg == 0) { */
    /*     adc_vbg = 1; //防止除0异常 */
    /*     assert_d(0, "func:%s(), line:%d, adc_vbg is 0\n", __func__, __LINE__); */
    /* } */
    u32 adc_voltage = adc_value * tmp / adc_vbg;
    return adc_voltage;
}
u32 adc_get_value_blocking(enum AD_CH ch)
{
    if (ch == AD_CH_IOVDD) {
        adc_queue[ADC_MAX_CH].v.value = (0XFFFF >> (16 - adc_data_res));
        return adc_queue[ADC_MAX_CH].v.value;
    }
    u32 time_out_us = 10 * 1000;
    while (time_out_us--) {
        asm("csync");
        if (adc_queue[ADC_MAX_CH].ch == -1) {
            break;
        }
        udelay(1);
    }
    gpadc_spin_lock(&gpadc_lock);
    adc_queue[ADC_MAX_CH].ch = ch;
    gpadc_spin_unlock(&gpadc_lock);

    adc_wait_enter_idle();

    local_irq_disable();

    adc_sample(adc_queue[ADC_MAX_CH].ch, 0, 1);

    adc_queue[ADC_MAX_CH].v.value = adc_wait_pnd();
    /* u16 value = adc_wait_pnd(); */
    /* if (ch == AD_CH_PMU_VBAT) { */
    /*     value *= AD_CH_PMU_VBAT_DIV; */
    /* } */
    /* adc_queue[ADC_MAX_CH].v.value = value; */

    /* adc_queue[ADC_MAX_CH].ch = -1; */

    adc_set_enter_idle();

    local_irq_enable();

    adc_queue[ADC_MAX_CH].ch = -1;

    return adc_queue[ADC_MAX_CH].v.value;
}
u32 adc_get_value_blocking_filter_dma(enum AD_CH ch, u32 *buffer, u32 sample_times)
{
    if (ch == AD_CH_IOVDD) {
        adc_queue[ADC_MAX_CH].v.value = (0XFFFF >> (16 - adc_data_res));
        return adc_queue[ADC_MAX_CH].v.value;
    }
    u32 time_out_us = 10 * 1000;
    while (time_out_us--) {
        asm("csync");
        if (adc_queue[ADC_MAX_CH].ch == -1) {
            break;
        }
        udelay(1);
    }
    gpadc_spin_lock(&gpadc_lock);
    adc_queue[ADC_MAX_CH].ch = ch;
    gpadc_spin_unlock(&gpadc_lock);

    adc_wait_enter_idle();

    local_irq_disable();

    adc_sample(adc_queue[ADC_MAX_CH].ch, 0, 1);

    u32 min_value = -1;
    u32 max_value = 0;
    u32 sum_value = 0;

    for (int i = 0; i < sample_times; i++) {
        u32 v = adc_wait_pnd();
        if (v > max_value) {
            max_value = v;
        }
        if (v < min_value) {
            min_value = v;
        }

        sum_value += v;
        if (buffer) {
            buffer[i] = v;
        }
    }

    sum_value -= max_value;
    sum_value -= min_value;

    adc_queue[ADC_MAX_CH].v.value = sum_value;

    /* adc_queue[ADC_MAX_CH].ch = -1; */

    adc_set_enter_idle();

    local_irq_enable();

    adc_queue[ADC_MAX_CH].ch = -1;

    return adc_queue[ADC_MAX_CH].v.value;
}
u32 adc_get_value_blocking_filter(enum AD_CH ch, u32 sample_times)
{
    u32 ch_adc_value = 0;
    u32 ch_min_value = -1;
    u32 ch_max_value = 0;

    if (sample_times <= 2) {
        sample_times = 3;
    }
    if (ch == AD_CH_IOVDD) {
        ch_adc_value = (0XFFFF >> (16 - adc_data_res));
        return ch_adc_value;
    }

    for (int i = 0; i < sample_times; i++) {

        u32 adc_value = adc_get_value_blocking(ch);

        if (adc_value > ch_max_value) {
            ch_max_value = adc_value;
        }
        if (adc_value < ch_min_value) {
            ch_min_value = adc_value;
        }
        ch_adc_value += adc_value;
    }

    ch_adc_value -= ch_max_value;
    ch_adc_value -= ch_min_value;

    ch_adc_value /= sample_times - 2;

    return ch_adc_value;
}
u32 adc_get_voltage_blocking(enum AD_CH ch)
{
    u32 vbg_adc_value = adc_get_value_blocking(AD_CH_LDOREF);
    u32 ch_adc_value = adc_get_value_blocking(ch);
    return adc_value_to_voltage(vbg_adc_value, ch_adc_value);
}
u32 adc_get_voltage_blocking_filter(enum AD_CH ch, u32 sample_times)
{
    if (sample_times <= 2) {
        sample_times = 3;
    }

    u32 min_value = -1;
    u32 max_value = 0;
    u32 sum_value = 0;

    for (int i = 0; i < sample_times; i++) {

        u32 v = adc_get_voltage_blocking(ch);
        if (v > max_value) {
            max_value = v;
        }
        if (v < min_value) {
            min_value = v;
        }

        sum_value += v;

    }

    sum_value -= max_value;
    sum_value -= min_value;

    sum_value /= sample_times - 2;
    return sum_value;
}
u32 adc_add_sample_ch(enum AD_CH ch)
{
    u32 adc_type_sel = ch & ADC_CH_MASK_TYPE_SEL;
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    log_info("type = %x,ch = %x\n", adc_type_sel, adc_ch_sel);
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            break;
        } else if (adc_queue[i].ch == -1) {
            adc_queue[i].ch = ch;
            adc_queue[i].jiffies = 0;
            adc_queue[i].v.value = (u16) - 1;
            adc_queue[i].sample_period = 0;
            adc_queue[i].voltage_mode = AD_MODE_DEFAULT;
            break;
        }
    }
    return i;
}
u32 adc_delete_ch(enum AD_CH ch)
{
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            adc_queue[i].ch = -1;
            break;
        }
    }
    return i;
}
u32 adc_set_sample_period(enum AD_CH ch, u32 ms)
{
    u32 i;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            adc_queue[i].sample_period = msecs_to_jiffies(ms);
            adc_queue[i].jiffies = msecs_to_jiffies(ms) + jiffies;
            break;
        }
    }
    return i;
}
u32 adc_set_voltage_mode(enum AD_CH ch, enum AD_MODE mode)
{
    u32 num = adc_ch2num(ch);
    adc_queue[num].voltage_mode = mode;
    return num;
}
void adc_init(void)
{
    adc_close();
    adc_hw_init();
}
void adc_refresh()
{
    for (u8 ch = 0; ch < ADC_MAX_CH; ch++) {
        if (adc_queue[ch].voltage_mode == AD_MODE_VOLTAGE) {
            u32 voltage = adc_get_voltage_blocking(adc_queue[ch].ch) * ADC_FACTOR;
            adc_queue[ch].v.voltage = voltage;
            log_info("ch:%d, voltage:%dmv\n", ch, voltage);
        } else if (adc_queue[ch].ch == AD_CH_LDOREF) {
            u32 adc_value = adc_get_value_blocking_filter(AD_CH_LDOREF, 12);
            adc_queue[0].v.value = adc_value;
            log_info("LDOREF = %d\n", adc_value);
        } else {
            u32 adc_value = adc_get_value_blocking_filter(AD_CH_LDOREF, 3);
            adc_queue[ch].v.value = adc_value;
            log_info("ch:%d, value:%d\n", ch, adc_value);
        }
    }
}
void adc_update_vbg_value_restart(u8 cur_miovdd_level, u8 new_miovdd_level)
{
    //该接口有bug, 需提供“获取x档位对应的iovdd电压” 的接口
    printf("cur = %d, new = %d\n", cur_miovdd_level, new_miovdd_level);
    u32 cur_miovdd_vol = 2100 + cur_miovdd_level * 100;
    u32 new_miovdd_vol = 2100 + new_miovdd_level * 100;
    adc_queue[0].v.value = adc_queue[0].v.value * cur_miovdd_vol / new_miovdd_vol;
}

u32 adc_check_vbat_lowpower()
{
    return 0;
}

_WEAK_
u16 efuse_get_vtemp(void)
{
    return 0;
}

s32 gpadc_get_ntc_temperature()
{
    const u8 Vbit = 3;//mv
    const u16 Vcent = 1440;//mv
    const float Factor = -3.57f;//mv/°C

    const u8 efuse_dtemp_vol_record = efuse_get_vtemp();

    const u8 E_t25 = efuse_dtemp_vol_record;
    const u8 E_offset = E_t25 & 0x3f;
    const u8 E_sig = E_t25 & BIT(6);
    const u32 V_offset = E_offset * Vbit;
    const s32 V_t25 = E_sig ? (Vcent - V_offset) : (Vcent + V_offset);

    const u32 V_tnow = adc_get_voltage(ADC_CH_PMU_VTEMP);

    const s32 Tnow = 25 - (((s32)V_t25 - (s32)V_tnow) / Factor);

    return Tnow;
}

static u8 gpadc_idle_query(void)
{
    if (adc_idle_query()) {
        return 0; //不可以进入休眠
    }
    return 1; //可以进入休眠
}

REGISTER_LP_TARGET(gpadc_lp_target) = {
    .name = "gpadc",
    .is_idle = gpadc_idle_query,
};









#define BATTERY_VOLTAGE_BUF_SIZE   20
#define BATTERY_DATA_BUF_MID    10
static u32 battery_voltage_buf[BATTERY_VOLTAGE_BUF_SIZE];
#define BATTERY_VALUE_BUF_SIZE   8
static u32 battery_value_buf[BATTERY_VALUE_BUF_SIZE];
extern const u8 gpadc_battery_mode;
extern const u16 gpadc_battery_trim_vddiom_voltage; //电池trim 使用的vddio电压
extern const u16 gpadc_battery_trim_voltage; //电池trim 使用的vbat电压
static struct battery_data_fifo {
    u32 voltage;
    u8 voltage_offset;
    u8 value_offset;
    u32 *voltage_buf;
    u32 *value_buf;
    u32 sum_value;
} battery_fifo;

_WEAK_
u32 get_vddiom_voltage()
{
    return get_vddiom_vol();
}

_WEAK_
u32 efuse_get_vbat_3700()
{
    u32 vddiom_vol = gpadc_battery_trim_vddiom_voltage;
    u32 adc_value_max = BIT(adc_data_res) - 1;
    u32 voltage = gpadc_battery_trim_voltage / AD_CH_PMU_VBAT_DIV;

    return 8 * voltage * adc_value_max / vddiom_vol;
}
static void sort_ascending(u32 arr[], u32 n) //插入排序
{
    int i, j, key;
    for (i = 1; i < n; i++) {        // 从第二个元素开始遍历 <button class="citation-flag" data-index="7">
        key = arr[i];                // 保存当前待插入的元素 <button class="citation-flag" data-index="9">
        j = i - 1;

        // 将比key大的元素向后移动，找到插入位置 <button class="citation-flag" data-index="1">
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;            // 插入到正确位置 <button class="citation-flag" data-index="10">
    }
}
static u32 gpadc_battery_get_average(u32 *data, u32 size, u32 mid)
{
    u32 sum_v = 0;
    u32 start = (size - mid) / 2;
    u32 end = start + mid;
    for (u8 i = start; i < end; i++) {
        sum_v += data[i];
    }
    return sum_v / mid;
}
static u32 gpadc_battery_value_to_voltage(u32 value)
{
    /* int sum_v = adc_get_value_blocking_filter_dma(AD_CH_PMU_VBAT, NULL, BATTERY_SAMPLE_TIMES); */
    int sum_v = 0;
    if (value == 0) {
        for (u8 i = 0; i < BATTERY_VALUE_BUF_SIZE; i++) {
            battery_fifo.value_buf[i] = adc_get_value_blocking(AD_CH_PMU_VBAT);
            sum_v += battery_fifo.value_buf[i];
        }
    } else {
        u32 *p = &battery_fifo.value_buf[battery_fifo.value_offset];
        sum_v = battery_fifo.sum_value + value - *p;
        *p = value;
        battery_fifo.value_offset++;
        if (battery_fifo.value_offset >= BATTERY_VALUE_BUF_SIZE) {
            battery_fifo.value_offset = 0;
        }
    }
    battery_fifo.sum_value = sum_v;

    int K = efuse_get_vbat_3700();
    int Dg = get_vddiom_voltage() - gpadc_battery_trim_vddiom_voltage;
    int V;
    if (K == 0) {
        V = adc_value_to_voltage(adc_get_value(AD_CH_PMU_VBG), sum_v);
        V = V * AD_CH_PMU_VBAT_DIV / BATTERY_VALUE_BUF_SIZE;
    } else {
        V = gpadc_battery_trim_voltage * sum_v / K + (Dg * sum_v / (BIT(adc_data_res) * (8 / AD_CH_PMU_VBAT_DIV)));
    }
    return (u32)V;
}
int gpadc_battery_init()
{
    if (gpadc_battery_mode != MEAN_FILTERING_MODE) {
        return 0;
    }
    log_info("func:%s(), line:%d\n", __func__, __LINE__);

    memset(battery_voltage_buf, 0, sizeof(battery_voltage_buf));
    battery_fifo.voltage = 0;
    battery_fifo.sum_value = 0;
    battery_fifo.voltage_buf = battery_voltage_buf;
    battery_fifo.voltage_offset = 0;
    battery_fifo.value_buf = battery_value_buf;
    battery_fifo.value_offset = 0;

    /* for (u8 i = 0; i < BATTERY_VALUE_BUF_SIZE; i++) { */
    /*     battery_fifo.value_buf[i] = adc_get_value_blocking(AD_CH_PMU_VBAT); */
    /*     battery_fifo.sum_value += battery_fifo.value_buf[i]; */
    /* } */
    for (u8 i = 0; i < BATTERY_VOLTAGE_BUF_SIZE; i++) {
        battery_fifo.voltage_buf[i] = gpadc_battery_value_to_voltage(0);
        log_info("gpadc_battery_init voltage_buf[%d]:%dmv\n", i, battery_fifo.voltage_buf[i]);
    }
    sort_ascending(battery_fifo.voltage_buf, BATTERY_VOLTAGE_BUF_SIZE);
    battery_fifo.voltage = gpadc_battery_get_average(battery_fifo.voltage_buf, BATTERY_VOLTAGE_BUF_SIZE, BATTERY_DATA_BUF_MID);
    log_info("gpadc_battery_init voltage:%dmv\n", battery_fifo.voltage);

    /* usr_timer_add(NULL, gpadc_battery_callback, 100, 0); */
    /* adc_internal_signal_to_io(AD_CH_PMU_VBAT, 16+2); //将内部通道信号，接到IO口上，输出 */
    /* local_irq_disable(); */
    /* extern void wdt_clear(); */
    /* wdt_clear(); */
    return 0;
}
static u32 gpadc_battery_voltage_process(u32 value)
{
    if (gpadc_battery_mode == MEAN_FILTERING_MODE) {
        u32 voltage = gpadc_battery_value_to_voltage(value);
        battery_fifo.voltage_buf[battery_fifo.voltage_offset] = voltage;
        battery_fifo.voltage_offset++;
        if (battery_fifo.voltage_offset >= BATTERY_VOLTAGE_BUF_SIZE) {
            battery_fifo.voltage_offset = 0;
        }
        u32 buffer[BATTERY_VOLTAGE_BUF_SIZE];
        memcpy(buffer, battery_fifo.voltage_buf, sizeof(buffer));
        /* quickSortDescending(buffer, 0, BATTERY_VOLTAGE_BUF_SIZE-1); */
        sort_ascending(buffer, BATTERY_VOLTAGE_BUF_SIZE);
        battery_fifo.voltage = gpadc_battery_get_average(buffer, BATTERY_VOLTAGE_BUF_SIZE, BATTERY_DATA_BUF_MID);
        /* log_info("vbat_voltage:%dmv\n", battery_fifo.voltage); */
        /* printf("#gpadc %d, %d, %d, %d, %d\n",  */
        /*         adc_get_value(AD_CH_PMU_VBG), */
        /*         value, */
        /*         voltage, */
        /*         battery_fifo.voltage, */
        /*         adc_get_voltage(AD_CH_PMU_VBAT)*AD_CH_PMU_VBAT_DIV); */
    }
    return 0;
}
u32 gpadc_battery_get_voltage()
{
    if (gpadc_battery_mode == MEAN_FILTERING_MODE) {
        return battery_fifo.voltage;
    } else if (gpadc_battery_mode == WEIGHTING_MODE) {
        return adc_get_voltage(AD_CH_PMU_VBAT) * AD_CH_PMU_VBAT_DIV;
    } else {
        return 0;
    }
}
void gpadc_battery_refresh()
{
    if (gpadc_battery_mode == MEAN_FILTERING_MODE) {
        for (u8 i = 0; i < BATTERY_VOLTAGE_BUF_SIZE; i++) {
            battery_fifo.voltage_buf[i] = gpadc_battery_value_to_voltage(0);
            printf("i:%d, %d\n", i, battery_fifo.voltage_buf[i]);
        }
    } else if (gpadc_battery_mode == WEIGHTING_MODE) {
        adc_refresh();
    }
}

