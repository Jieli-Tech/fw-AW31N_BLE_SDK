#ifndef  __BOOT_MODE_H__
#define  __BOOT_MODE_H__

#include "typedef.h"


struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 lvd_en: 1;
    u8 sfc_fast_boot: 1;
    u8 flash_power_keep: 1;
    u8 skip_flash_reset: 1;
    u8 sfc_flash_stable_delay_sel: 1;   //0: 0.5mS; 1: 1mS
    u8 flash_stable_delay_sel: 2;       //0: 0mS;  1: 2mS;  2:4mS; 3:6mS
};

struct soft_flag1_t {
    u8 usbdp: 4;
    u8 usbdm: 4;
};

struct soft_flag2_t {
    u8 res: 2;
    u8 soft_flag_power_enable: 1;
    u8 btxosc_init_ext_clk_enable: 1;
    u8 disable_uart_upgrade: 1;
    u8 uart_key_port_pull_down: 1;
    u8 flash_spi_baud: 2;
};

struct soft_flag3_t {
    u8 pa5: 4;
    u8 pa6: 4;
};

struct soft_flag7_4_t {
    u32 chip_key : 32;
};
struct soft_flag11_8_t {
    u32 pll_lrc_nr : 12;
    u32 res : 20;
};

struct boot_soft_flag_t {
    u8 soff_wkup;
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
    union {
        struct soft_flag1_t boot_ctrl;
        u8 value;
    } flag1;
    union {
        struct soft_flag2_t boot_ctrl;
        u8 value;
    } flag2;
    union {
        struct soft_flag3_t boot_ctrl;
        u8 value;
    } flag3;
    union {
        struct soft_flag7_4_t boot_ctrl;
        u32 value;
    } flag7_4;
    union {
        struct soft_flag11_8_t boot_ctrl;
        u32 value;
    } flag11_8;
};



#endif

