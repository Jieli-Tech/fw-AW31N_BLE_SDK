#ifndef  __GPADC_V12_H__
#define  __GPADC_V12_H__
//适用于 bd47

//ADC_CON reg
#define GPADC_CON_DONE_PND      31
#define GPADC_CON_DONE_PND_CLR  30
#define GPADC_CON_DONE_PND_IE   29
// #define GPADC_CON_RESERVED      28 //bit28~bit24
#define GPADC_CON_ADC_MUX_SEL   21
#define GPADC_CON_ADC_MUX_SEL_  3
#define GPADC_CON_ADC_ASEL      18
#define GPADC_CON_ADC_ASEL_     3
#define GPADC_CON_ADC_CLKEN     17
#define GPADC_CON_ADC_ISEL      16
#define GPADC_CON_WAIT_TIME     12
#define GPADC_CON_WAIT_TIME_    4
#define GPADC_CON_CH_SEL        8
#define GPADC_CON_CH_SEL_       4
#define GPADC_CON_PND       7
#define GPADC_CON_CPND      6
#define GPADC_CON_ADC_IE    5
#define GPADC_CON_ADC_EN    4
#define GPADC_CON_ADC_AE    3
#define GPADC_CON_ADC_BAUD  0
#define GPADC_CON_ADC_BAUD_ 3



#endif


