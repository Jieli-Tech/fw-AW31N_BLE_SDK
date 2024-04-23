#include "key_drv_matrix.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[key_matrix]"
#include "log.h"


#if KEY_MATRIX_EN

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
/**@brief   matrix按键初始化
   @param   void
   @param   void
   @return  void
   @note    void matrix_key_init(void)
*/
/*----------------------------------------------------------------------------*/
void matrix_key_init(void)
{
#if 0
    for (i = 0; i < sizeof(matrix_key_row); i++) { //key 输出模式
        gpio_direction_output(matrix_key_row[i], !(KEY_DETECT_LEVEL));
    }
#else
    for (uint8_t i = 0; i < sizeof(matrix_key_row); i++) { //key 高阻模式
        gpio_set_mode(IO_PORT_SPILT(matrix_key_row[i]), PORT_HIGHZ);
        gpio_hw_set_dieh(IO_PORT_SPILT(matrix_key_row[i]), 0);
    }
#endif
    for (uint8_t i = 0; i < sizeof(matrix_key_col); i++) {
        gpio_hw_set_direction(IO_PORT_SPILT(matrix_key_col[i]), 1);
#if KEY_DETECT_LEVEL
        gpio_hw_set_pull_up(IO_PORT_SPILT(matrix_key_col[i]), 0);
        gpio_hw_set_pull_down(IO_PORT_SPILT(matrix_key_col[i]), 1);
#else
        gpio_hw_set_pull_up(IO_PORT_SPILT(matrix_key_col[i]), 1);
        gpio_hw_set_pull_down(IO_PORT_SPILT(matrix_key_col[i]), 0);
#endif
        gpio_hw_set_die(IO_PORT_SPILT(matrix_key_col[i]), 1);
    }
}
void matrix_key_suspend()
{
    matrix_key_init();
}
void matrix_key_release()
{
    for (uint8_t i = 0; i < sizeof(matrix_key_row); i++) {
        gpio_set_mode(IO_PORT_SPILT(matrix_key_row[i]), PORT_HIGHZ);
        gpio_hw_set_dieh(IO_PORT_SPILT(matrix_key_row[i]), 0);
    }
    for (uint8_t i = 0; i < sizeof(matrix_key_col); i++) {
        gpio_set_mode(IO_PORT_SPILT(matrix_key_col[i]), PORT_HIGHZ);
        gpio_hw_set_dieh(IO_PORT_SPILT(matrix_key_col[i]), 0);
    }
}


#if 0
uint8_t matrix_key_scan()//key 输出模式
{
    /*{{{*/
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t key_val = 0xff;
    for (i = 0; i < sizeof(matrix_key_row); i++) {
        gpio_write(matrix_key_row[i], KEY_DETECT_LEVEL);

        for (j = 0; j < sizeof(matrix_key_col); j++) {
            if (gpio_read(matrix_key_col[j]) == KEY_DETECT_LEVEL) {
                key_val = i * sizeof(matrix_key_col) + j;
                gpio_write(matrix_key_row[i], !(KEY_DETECT_LEVEL));
                printf("(%d %d,%d)", key_val, i, j);
                return key_val;
            }
        }
        gpio_write(matrix_key_row[i], !(KEY_DETECT_LEVEL));
    }
    printf("- ");
    return key_val;
}/*}}}*/
#else
uint8_t matrix_key_scan()//key 高阻模式
{
    local_irq_disable();
    uint8_t key_val = NO_KEY;
    for (uint8_t i = 0; i < sizeof(matrix_key_row); i++) {
        gpio_hw_direction_output(IO_PORT_SPILT(matrix_key_row[i]), KEY_DETECT_LEVEL);

        for (uint8_t j = 0; j < sizeof(matrix_key_col); j++) {
            if (gpio_read(matrix_key_col[j]) == KEY_DETECT_LEVEL) {
                matrix_key_row_value &= ~(1u << i);
                matrix_key_col_value &= ~(1u << j);
                key_val = i * sizeof(matrix_key_col) + j;
                /* key_puts("(%d %d,%d)",key_val,i,j); */

                /* gpio_set_direction(matrix_key_row[i],1); */
                /* local_irq_enable(); */
                /* return key_val; */
            }
        }
        gpio_hw_set_direction(IO_PORT_SPILT(matrix_key_row[i]), 1);
    }
    /* key_puts("- "); */
    local_irq_enable();
    return key_val;
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief   获取matrix按键电平值
   @param   void
   @param   void
   @return  key_num:matrix按键号
   @note    uint8_t _get_matrixkey_value(void)
*/
/*----------------------------------------------------------------------------*/
uint8_t get_matrixkey_value(void)
{
    //key_puts("_get_matrixkey_value\n");
    uint8_t key_num = matrix_key_scan();
    return key_filter(key_num);
}

void set_matrixkey_row_port_output(void)
{
    for (uint8_t i = 0; i < sizeof(matrix_key_row); i++) {
        gpio_set_mode(IO_PORT_SPILT(matrix_key_row[i]), PORT_OUTPUT_LOW);
    }
}

const key_interface_t key_matrix_info = {
    .key_type = KEY_TYPE_MATRIX,
    .key_init = matrix_key_init,
    .key_get_value = get_matrixkey_value,
};

#endif/*KEY_MATRIX_EN*/
