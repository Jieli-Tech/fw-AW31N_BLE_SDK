#include "gpadc_hw.h"
#include "gpadc.h"

#define LOG_TAG_CONST       GPADC
#define LOG_TAG             "[GPADC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

const u8 adc_data_res = 10; //adc 采样精度
const static u8 adc_data_res_table[1] = {10}; //采样精度
void adc_data_res_check()
{
    for (u32 i = 0; i < ARRAY_SIZE(adc_data_res_table); i++) {
        if (adc_data_res_table[i] == adc_data_res) {
            return;
        }
    }
    assert(0, "adc_data_res = %d set error\n", adc_data_res);
}
static void adc_wait_idle_timeout()
{
    u32 time_out_us = 1000;
    while (time_out_us--) {
        asm("csync");
        if ((JL_ADC->CON & BIT(GPADC_CON_ADC_EN)) == 0) {
            return;
        }
        udelay(1);
    }
}
void adc_wait_enter_idle() //等待adc进入空闲状态，才可进行阻塞式采集
{
    if (JL_ADC->CON & BIT(GPADC_CON_ADC_EN)) {
        adc_wait_idle_timeout();
        adc_close();
    }
}
static u32 adc_wait_pnd_timeout(u32 pnd)
{
    u32 time_out_us = 1000;
    while (time_out_us--) {
        asm("csync");
        if (JL_ADC->CON & BIT(pnd)) {
            return 1; //成功检测到pnd
        }
        udelay(1);
    }
    assert(0, "adc timeout %x", JL_ADC->CON);
    return 0; //失败
}
u32 adc_wait_pnd()   //cpu采集等待pnd
{
    u32 adc_res;
    if (adc_wait_pnd_timeout(GPADC_CON_DONE_PND)) {
        adc_res = JL_ADC->RES; //默认右对齐
    } else {
        adc_res = 0;
    }
    JL_ADC->CON |= BIT(GPADC_CON_CPND);//kistart
    return adc_res;
}
void adc_set_enter_idle()
{
    adc_close();
}
void adc_sample(enum AD_CH ch, u32 ie, u32 calibrate_en) //启动一次cpu模式的adc采样
{
    //没有校准功能,忽略 calibrate_en 参数
    u32 adc_con = 0;
    u8 adc_clk_div = adc_get_clk_div();
    SFR(adc_con, GPADC_CON_ADC_BAUD, GPADC_CON_ADC_BAUD_, adc_clk_div);
    SFR(adc_con, GPADC_CON_WAIT_TIME, GPADC_CON_WAIT_TIME_, 1);//启动延时控制，实际启动延时为此数值*8个ADC时钟
    adc_con |= BIT(GPADC_CON_ADC_EN) | BIT(GPADC_CON_ADC_AE) | BIT(GPADC_CON_ADC_CLKEN);
    adc_con |= BIT(GPADC_CON_CPND) | BIT(GPADC_CON_DONE_PND_CLR);
    if (ie) {
        adc_con |= BIT(GPADC_CON_DONE_PND_IE);
    }
    adc_ana_ch_sel(ch, &adc_con);
    JL_ADC->CON = adc_con;
    JL_ADC->CON |= BIT(GPADC_CON_CPND);//kistart
}
void adc_close()
{
    JL_ADC->CON = BIT(GPADC_CON_ADC_CLKEN);//clock_en
    JL_ADC->CON = BIT(GPADC_CON_ADC_CLKEN) | BIT(GPADC_CON_DONE_PND_CLR) | BIT(GPADC_CON_CPND);
    JL_ADC->CON = BIT(GPADC_CON_DONE_PND_CLR) | BIT(GPADC_CON_CPND);
    ADC_PMU_CH_CLOSE();
}
void adc_dump(void)
{
    printf("ADC_CON:0x%x\n", JL_ADC->CON);
    printf("ADC_RES:0x%x\n", JL_ADC->RES);
}
u32 adc_get_res()
{
    if (JL_ADC->CON & BIT(GPADC_CON_DONE_PND)) {
        return JL_ADC->RES;
    }
    return -1;
}
_INLINE_ u32 adc_idle_query()
{
    return JL_ADC->CON & BIT(GPADC_CON_ADC_EN);
}
_INLINE_ void adc_register_clear()
{
    JL_ADC->CON = 0;
}


