#include "gpadc.h"

//bd47
#define LOG_TAG_CONST       GPADC
#define LOG_TAG             "[GPADC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

const static u8 adc_ch_io_table[16] = {
    IO_PORTA_00,
    IO_PORTA_01,
    IO_PORTA_02,
    IO_PORTA_03,
    IO_PORTA_04,
    IO_PORTA_05,
    IO_PORTA_06,
    IO_PORTF_02,
    IO_PORTA_08,
    IO_PORTA_09,
    IO_PORTA_10,
    IO_PORTA_11,
    IO_PORT_FSPG,
    IO_PORT_DP,
    IO_PORT_DM,
};

u32 adc_io2ch(int gpio)
{
    for (u8 i = 0; i < ARRAY_SIZE(adc_ch_io_table); i++) {
        if (adc_ch_io_table[i] == gpio) {
            return (ADC_CH_TYPE_IO | i);
        }
    }
    log_error("add_adc_ch io error!!! change other io_port!!!\n");
    return 0xffffffff; //未找到支持ADC的IO
}

#define ADC_CLK_MAX     500*1000 //ADC时钟最高支持频率 默认8M, vbg+vbat+vtemp+io 时间在50us左右
static u8 adc_clk_div = 7; //adc时钟分频系数档位
extern const u32 lib_adc_clk_max;

static void clock_critical_enter(void)
{
}
void adc_adjust_div(void)
{
    const u8 adc_div_table[] = {1, 6, 12, 24, 48, 72, 96, 128};
    const u32 lsb_clk = clk_get("lsb");
    if (lsb_clk == 0) {
        return;
    }
    const u32 adc_clk_max = MIN(ADC_CLK_MAX, lib_adc_clk_max);
    for (int i = 0; i < ARRAY_SIZE(adc_div_table); i++) {
        if (lsb_clk / adc_div_table[i] <= adc_clk_max) {
            adc_clk_div = i;
            break;
        }
    }
}
LSB_CRITICAL_HANDLE_REG(gpadc, clock_critical_enter, adc_adjust_div);

_INLINE_ u8 adc_get_clk_div()
{
    return adc_clk_div;
}

void adc_pmu_vbg_enable()
{
    ADC_PMU_VBG_BUFFER_EN(1);
    udelay(50);
    ADC_PMU_VBG_TEST_EN(1);
}
void adc_pmu_vbg_disable()
{
    ADC_PMU_VBG_TEST_EN(0);
    udelay(50);
    ADC_PMU_VBG_BUFFER_EN(0);
}
static void adc_pmu_ch_select(u32 ch)
{
    u8 vbg_ch = ((ch & ADC_CH_MASK_PMU_VBG_CH_SEL) >> 8);
    u8 pmu_ch = (ch & ADC_CH_MASK_CH_SEL);
    /* ADC_PADC0_TOADC_EN(0); */
    if (pmu_ch == 0) {  //VBG通道
        ADC_PMU_VBG_TEST_EN(1);
        ADC_PMU_VBG_TEST_SEL(vbg_ch);
    }
    ADC_PMU_CHANNEL_ADC(pmu_ch);
    ADC_PMU_DET_OE(1);
    ADC_PMU_TOADC_EN(1);
}

void adc_ana_ch_sel(enum AD_CH ch, u32 *_adc_con)
{
    u32 adc_con  = *_adc_con;
    u32 adc_type_sel = ch & ADC_CH_MASK_TYPE_SEL;
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    SFR(adc_con, GPADC_CON_ADC_MUX_SEL, GPADC_CON_ADC_MUX_SEL_, 0b010);//cpu adc test sel en
    SFR(adc_con, GPADC_CON_ADC_ASEL, GPADC_CON_ADC_ASEL_, adc_type_sel >> 16);    //test sel
    switch (adc_type_sel) {
    case ADC_CH_TYPE_BT:
        break;
    case ADC_CH_TYPE_PMU:
        adc_pmu_ch_select(ch);
        break;
    case ADC_CH_TYPE_SYSPLL:
        break;
    case ADC_CH_TYPE_X32K:
        break;
    default:
        SFR(adc_con, GPADC_CON_ADC_MUX_SEL, GPADC_CON_ADC_MUX_SEL_, 0b001); //cpu adc io sel en
        SFR(adc_con, GPADC_CON_CH_SEL, GPADC_CON_CH_SEL_, adc_ch_sel);
        break;
    }
    *_adc_con = adc_con;
}

void adc_internal_signal_to_io(enum AD_CH ch, u16 gpio)
{
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_HIGHZ);

    adc_sample(ch, 0, 0);
    u32 io_ch = adc_io2ch(gpio);
    u16 adc_ch_sel = io_ch & ADC_CH_MASK_CH_SEL;
    SFR(JL_ADC->CON, GPADC_CON_CH_SEL, GPADC_CON_CH_SEL_, adc_ch_sel);
    SFR(JL_ADC->CON, GPADC_CON_ADC_MUX_SEL, GPADC_CON_ADC_MUX_SEL_, 0b111);
}

void adc_test_func()
{
    printf("vbg:%d\n", adc_get_value(AD_CH_PMU_VBG));
    printf("vbg_0:%d\n", adc_get_value_blocking(AD_CH_PMU_WBG04));
    printf("vbg_1:%d\n", adc_get_value_blocking(AD_CH_PMU_MBG08));
    printf("vbg_2:%d\n", adc_get_value_blocking(AD_CH_PMU_LVDBG));
    printf("vbg_3:%d\n", adc_get_value_blocking(AD_CH_PMU_MVBG08));
}


