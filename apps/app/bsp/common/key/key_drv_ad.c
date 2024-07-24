#pragma bss_seg(".adkey.data.bss")
#pragma data_seg(".adkey.data")
#pragma const_seg(".adkey.text.const")
#pragma code_seg(".adkey.text")
#pragma str_literal_override(".adkey.text.const")

#include "key_drv_ad.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[key_ad]"
#include "log.h"

#if KEY_AD_EN

const uint16_t ad_key_table[] = {
    ADKEY1_8, ADKEY1_7, ADKEY1_6, ADKEY1_5, ADKEY1_4,
    ADKEY1_3, ADKEY1_2, ADKEY1_1, ADKEY1_0
};

static uint8_t ad_key_init_flag;

/*----------------------------------------------------------------------------*/
/**@brief   按键去抖函数，输出稳定键值
   @param   key：键值
   @return  稳定按键
   @note    uint8_t key_filter(uint8_t key)
*/
/*----------------------------------------------------------------------------*/
static uint8_t key_filter(uint8_t key)
{
    static uint8_t used_key = NO_KEY;
    static uint8_t old_key;
    static uint8_t key_counter;

    if (old_key != key) {
        key_counter = 0;
        old_key = key;
    } else {
        key_counter++;
        if (key_counter == KEY_BASE_CNT) {
            used_key = key;
        }
    }

    return used_key;
}


/*----------------------------------------------------------------------------*/
/**@brief   ad按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ad_key0_init(void)
*/
/*----------------------------------------------------------------------------*/
void ad_key_init(void)
{
    // printf("ad key init\n");

    if ((u32) - 1 == adc_io2ch(AD_KEY_IO)) {
        key_puts("ad key init fail!\n");
        return;
    }

#if EXTERN_R_UP
    gpio_set_mode(IO_PORT_SPILT(AD_KEY_IO), PORT_INPUT_FLOATING);
#else
    gpio_set_mode(IO_PORT_SPILT(AD_KEY_IO), PORT_INPUT_PULLUP_10K);
#endif
    gpio_set_function(IO_PORT_SPILT(AD_KEY_IO), PORT_FUNC_GPADC);

    adc_add_sample_ch(adc_io2ch(AD_KEY_IO));
    ad_key_init_flag = 1;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ad按键值
   @param   void
   @param   void
   @return  key_number
   @note    tuint8_t get_adkey_value(void)
*/
/*----------------------------------------------------------------------------*/
uint8_t get_adkey_value(void)
{
    if (ad_key_init_flag != 1) {
        return 0xff;
    }
    u32 key_value = adc_get_value(adc_io2ch(AD_KEY_IO));
    if (key_value > AD_NOKEY) {
        /* printf("key_value: %d\n", key_value); */
        return key_filter(NO_KEY);
    }

    uint8_t key_number = 0;
    for (; key_number < sizeof(ad_key_table) / sizeof(ad_key_table[0]); key_number++) {
        if (key_value < ad_key_table[key_number]) {
            return key_filter(key_number);
        }
    }
    return key_filter(NO_KEY);
}

const key_interface_t key_ad_info = {
    .key_type = KEY_TYPE_AD,
    .key_init = ad_key_init,
    .key_get_value = get_adkey_value,
};
#endif


