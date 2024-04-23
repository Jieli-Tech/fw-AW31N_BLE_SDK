#include "key_drv_io.h"
#include "app_config.h"
#include "gpio.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[key_io]"
#include "log.h"


#if KEY_IO_EN
#define MARK_BIT_VALUE(b, v) 		do {if ((v & (~BIT(7))) < 7) b |= BIT(v & (~BIT(7)));} while(0)
static const struct iokey_platform_data *__this = NULL;

static void key_io_pull_down_input(uint8_t key_io)
{
    gpio_set_mode(IO_PORT_SPILT(key_io), PORT_INPUT_PULLDOWN_10K);
}

static void key_io_pull_up_input(uint8_t key_io)
{
    gpio_set_mode(IO_PORT_SPILT(key_io), PORT_INPUT_PULLUP_10K);
}

static void key_io_output_high(uint8_t key_io)
{

    gpio_set_mode(IO_PORT_SPILT(key_io), PORT_OUTPUT_HIGH);
}

static void key_io_output_low(uint8_t key_io)
{
    gpio_set_mode(IO_PORT_SPILT(key_io), PORT_OUTPUT_LOW);
}

static int get_io_key_value(uint8_t key_io)
{
    return gpio_read(key_io);
}

static void key_io_reset(void)
{
    int i;

    for (i = 0; i < __this->num; i++) {
        switch (__this->port[i].connect_way) {
        case ONE_PORT_TO_HIGH:
            key_io_pull_down_input(__this->port[i].key_type.one_io.port);
            break;

        case ONE_PORT_TO_LOW:
            key_io_pull_up_input(__this->port[i].key_type.one_io.port);
            break;

        case DOUBLE_PORT_TO_IO:
            break;

        default:
            ASSERT(0, "IO KEY CONNECT ERR!!!");
            break;
        }
    }
}

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
/**@brief   io按键初始化
   @param   void
   @param   void
   @return  void
   @note    void io_key_init(void)
 */
/*----------------------------------------------------------------------------*/
extern const struct iokey_platform_data iokey_data;
int iokey_init(const struct iokey_platform_data *iokey_data)
{
    __this = iokey_data;
    if (__this == NULL) {
        return -EINVAL;
    }
    if (!__this->enable) {
        return KEY_NOT_SUPPORT;
    }

    key_io_reset();

    return 0;

}

void _iokey_init(void)
{
    iokey_init(&iokey_data);
}

__attribute__((weak)) uint8_t iokey_filter_hook(uint8_t io_state)
{
    return 0;
}

#if MULT_KEY_ENABLE
extern const struct key_remap_data iokey_remap_data;
static uint8_t iokey_value_remap(uint8_t bit_mark)
{
    for (int i = 0; i < iokey_remap_data.remap_num; i++) {
        if (iokey_remap_data.table[i].bit_value == bit_mark) {
            return iokey_remap_data.table[i].remap_value;
        }
    }

    return NO_KEY;
}
#endif


/*----------------------------------------------------------------------------*/
/**@brief   获取IO按键电平值
  @param   void
  @param   void
   @return  key_num:io按键号
   @note    uint8_t get_iokey_value(void)
*/
/*----------------------------------------------------------------------------*/
uint8_t get_iokey_value(void)
{
    int i;

    uint8_t press_value = 0;
    uint8_t read_value = 0;
    uint8_t read_io;
    /* uint8_t write_io; */
    uint8_t connect_way;
    uint8_t ret_value = NO_KEY;
    uint8_t bit_mark = 0;

    if (!__this->enable) {
        printf("%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__);
        return NO_KEY;
    }

    //先扫描单IO接按键方式
    for (i = 0; i < __this->num; i++) {
        connect_way = __this->port[i].connect_way;

        if (connect_way == ONE_PORT_TO_HIGH) {
            press_value = 1;
        } else if (connect_way == ONE_PORT_TO_LOW) {
            press_value = 0;
        } else {
            continue;
        }

        read_io = __this->port[i].key_type.one_io.port;

        read_value = get_io_key_value(read_io);
        if (iokey_filter_hook(read_value)) {
            return NO_KEY;
        }
        if (read_value == press_value) {
            ret_value = __this->port[i].key_value;
#if MULT_KEY_ENABLE
            MARK_BIT_VALUE(bit_mark, ret_value);  //标记被按下的按键
#else
            /* goto _iokey_get_value_end; */
            return ret_value;
#endif
        }
    }

    //再扫描两个IO接按键方式, in_port: 上拉输入, out_port: 输出低
    for (i = 0; i < __this->num; i++) {
        connect_way = __this->port[i].connect_way;
        if (connect_way == DOUBLE_PORT_TO_IO) {//标准双io
            press_value = 0;
            read_io = __this->port[i].key_type.two_io.in_port;
            key_io_output_low(__this->port[i].key_type.two_io.out_port);  //输出低
            key_io_pull_up_input(read_io); 	//上拉
            read_value = get_io_key_value(read_io);
            key_io_reset(); //按键初始化为单IO检测状态
            if (read_value == press_value) {
                ret_value = __this->port[i].key_value;
#if MULT_KEY_ENABLE
                MARK_BIT_VALUE(bit_mark, ret_value);	//标记被按下的按键
#else
                /* goto _iokey_get_value_end; */
                return ret_value;
#endif
            }
        }
    }

#if MULT_KEY_ENABLE
    bit_mark = iokey_value_remap(bit_mark);  //组合按键重新映射按键值
    ret_value = (bit_mark != NO_KEY) ? bit_mark : ret_value;
#endif
    /* _iokey_get_value_end: */
    return ret_value;
}

const key_interface_t key_io_info = {
    .key_type = KEY_TYPE_IO,
    .key_init = _iokey_init,
    .key_get_value = get_iokey_value,
};

#endif/*KEY_IO_EN*/
