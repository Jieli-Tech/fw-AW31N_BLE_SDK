#ifndef __POWER_HW_H__
#define __POWER_HW_H__

#define CLK_SFR_NUM		30

#define MMU_SFR_NUM	 	3
#define MPU_SFR_NUM		(8*4)
#define SFC_SFR_NUM		(15)
#define ICFG_SFR_NUM	(32)
#define CORE_SFR_NUM	(20)

#define SDK_GPIO_NUM	(15)
#define ROM_GPIO_NUM	(40)

struct poff_sfr_bk_t {
    u32 *clk_sfr;
#if PCONFIG_MMU_ENABLE
    u32 *mmu_sfr;
#endif
    u32 *mpu_sfr;
    u32 *sfc_sfr;
    u32(*icfgx_sfr)[ICFG_SFR_NUM];
    u32(*core_sfr)[CORE_SFR_NUM];

    u32 *imap_sfr;
    u32 *omap_sfr;
    u32 *iomc_sfr;

    u16 *sdk_gpio_sfr;
    u16 *rom_gpio_sfr;
};

struct clk_regs_t {
    u32 sys_sel_bk;
    u32 sys_div_bk;
    u32 sfc_sel_bk;
    u32 lsb_sel_bk;

    u32 pll_keep;
#if PCONFIG_PDOWN_PLL_FLOWING
    u32 pll0_bk;
    u32 pll1_bk;
#endif

#if CONFIG_BTOSC_ENABLE
    u32 btosc_backup;
    u32 btosc_keep;
    u32 pmu_oe;
#endif
};

struct _phw_hdl {

    //sleep
    u32 osc_hz;
    u32 btosc_hz;
    u32 osc_delay_us;
    u32 xosc_resume_us;
    u32 *sleep_regs;
    struct clk_regs_t *clk_regs;

#if CONFIG_POFF_ENABLE
    struct poff_sfr_bk_t *poff_sfr_bk;
#endif

    //power
    u8 mode;
    u8 dcdc_type;
    u8 vddiow_lev;
    u8 vddiom_lev;
    u8 dcvdd_cap_en;
    u8 flash_pg_vddio;
#if CONFIG_RTC_ENABLE
    u8 rtc_clk;
#endif
    u8 power_supply;

    //sleep
    u8 pd_vddio_keep;
    u8 pd_wdvdd_lev;

#if PCONFIG_LPCTMU_ENABLE
    u8 pd_keep_lpctmu;
#endif

    u8 osc_type;
    u8 last_osc_type;
    u8 default_osc_type;

#if (CONFIG_P11_CPU_ENABLE==0)
    u8 kstb0;
    u8 kstb1;
    u8 kstb2;
    u8 kstb3;
    u8 kstb4;
    u8 kstb5;
    u8 kstb6;
#endif
    u8 fatal_error;

    //soff
    u8 sf_keep_lrc;
    u8 sf_vddio_keep;

#if CONFIG_P11_ENABLE
    u8 sf_keep_pvdd;
    u8 sf_keep_nvdd;

#if PCONFIG_LPCTMU_ENABLE
    u8 sf_keep_lpctmu;
#endif

    u8 sf_keep_lpnfc;
#endif
};

extern struct _phw_hdl phw_hdl;
extern struct _phw_dev phw_dev;


#endif
