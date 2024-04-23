#ifndef __PMU_FLAG_H__
#define __PMU_FLAG_H__

//efuse flag
struct efuse_page0_config_t {
    /* 0    */  u32 en_act: 1;
    /* 1    */  u32 lvd_rst_en: 1;
    /* 4-2  */  u32 lvd_level: 3;
    /* 8-5  */  u32 iovdd_level: 4;
    /* 9    */  u32 mclr_en : 1;
    /* 10   */  u32 fast_pu_dis: 1;
    /* 11   */  u32 pinr_io_enable: 1;
    /* 13-12*/  u32 res: 2;
    /* 14   */  u32 sfc_fast_boot_dis : 1;
    /* 15   */  u32 trim_act: 1;
    /* 19-16*/  u32 lvd_vbg_lel: 4;
    /* 23-20*/  u32 mvbg_lel: 4;
    /* 27-24*/  u32 wvbg0_lel: 4;
    /* 31-28*/  u32 wvbg1_lel: 4;
};
struct efuse_page0_t {
    union {
        struct efuse_page0_config_t cfg;
        u32 value;
    } u;
    u8 efuse_read_finish;
};

struct efuse_page1_config_t {
    /* 23-0 */  u32 chip_key: 24;
    /* 24   */  u32 cp_pass: 1;
    /* 25   */  u32 ft_pass: 1;
    /* 27:26*/  u32 wvdd_level_trim: 2;
    /* 31-28*/  u32 res: 4;
};
struct efuse_page1_t {
    union {
        struct efuse_page1_config_t cfg;
        u32 value;
    } u;
};


//p33 soft flag
enum soft_flag_io_stage {
    SOFTFLAG_HIGH_RESISTANCE,
    SOFTFLAG_PU,
    SOFTFLAG_PD,

    SOFTFLAG_OUT0,
    SOFTFLAG_OUT0_HD0,
    SOFTFLAG_OUT0_HD,
    SOFTFLAG_OUT0_HD0_HD,

    SOFTFLAG_OUT1,
    SOFTFLAG_OUT1_HD0,
    SOFTFLAG_OUT1_HD,
    SOFTFLAG_OUT1_HD0_HD,

    SOFTFLAG_PU100K,
    SOFTFLAG_PU1M,
    SOFTFLAG_PD100K,
    SOFTFLAG_PD1M,
};

struct app_soft_flag_t {
    u8 sfc_fast_boot;
    u8 sfc_flash_stable_delay_sel;
    u8 flash_stable_delay_sel;
    u8 usbdp;
    u8 usbdm;
    u8 pa5;
    u8 pa6;
};

//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//
#endif
