#ifndef __POWER_LIB_API_H__
#define __POWER_LIB_API_H__

//-------------------------------------------------------------------
/*lptmr
 */
bool __lp_timer_is_running();
void __lp_timer_dump();
bool __lp_timer_set_time(u32 usec, u32 usec1);
bool __lp_timer_kick_start(u32 type, u32 enable);
bool __lp_timer_reset_time(u32 usec);
bool __lp_timer_reset_timeout(u32 usec);
u32 __lp_timer_get_time();
u32 __lp_timer_get_pass_time();
u32 __lp_timer_get_period();
void __lp_timer_enable();
void __lp_timer_disable();
u32 lp_timeout_get();
u32 lp_timeout_set(u32 value);
u32 Lp_timeout_check(u8 mode, u32 feq_us);
void Lp_timeout_dump();
//phw_dev
u32 __lp_timer_init();
u32 __lp_timer_sleep_post();

//-------------------------------------------------------------------
/*lrc
 */
u32 __get_lrc_hz();
u32 get_lrc_hz();
//phw_dev
u32 __hw_lrc_init();
u32 __lrc_already();
u32 __lrc_sleep_prepare();
u32 __lrc_sleep_post();

//-------------------------------------------------------------------
/*rch
 */
u32 get_rch_hz();
//phw_dev
void cap_rch_init();
u32 cap_rch_already();
u32 cap_rch_sleep_prepare();
u32 cap_rch_sleep_post();

//-------------------------------------------------------------------
/*lp_flow_ic
 */
enum PWR_LAT {
    DVDDLS_LAT,
    PVDDLS_LAT,
};

void LS_ENTER_PDOWN();
void LS_EXIT_PDOWN();
void LS_ENTER_SOFF_POFF(enum PWR_LAT pwr_lat);
void LS_EXIT_ROM();
void LS_EXIT_SOFF_POFF(enum PWR_LAT pwr_lat);
void Lp_flow_soff_down_ic();
void Lp_flow_pdown_down_ic();
void Lp_flow_poff_down_ic();
void Lp_flow_poff_up_ic();

//-------------------------------------------------------------------
/*power_clock
 */
void low_power_clock_init();
void low_power_clock_uninit();
u32 get_hw_reserve_time_p();
u32 get_hw_reserve_time_r();

//-------------------------------------------------------------------
/*power_clock_ic
 */
void xosc_resume_dump();
u8 is_lowpower_anc_active();
bool is_btosc_keep();
void power_clock_pdown_enter(void);
void power_clock_pdown_exit();
void Power_clock_soff_enter();

//-------------------------------------------------------------------
/*sfc
 */
void Power_flash_poweroff();
void Power_flash_poweron();
void Power_cache_idle();
void Power_cache_run();


//----------------------------------------------------------
/*basic_funt
 */
u32 __tcnt_us(u32 x);
u32 __tcnt_us_no_carry(u32 x);
u32 __tus_cnt(u32 x);
u32 __tus_cnt_carry(u32 x);
#define udelay_lrc	lrc_tmr1_udelay
void lrc_tmr1_udelay(u32 usec);
void power_test(u32 cnt);
u32 Gpio_to_imap(u32 gpio);
u32 Imap_to_gpio(u32 imap);
u32 Wsig_to_gpio(u32 wsig);
void Gpio_to_wsig_dump(u32 gpio);
u32 Gpio_to_wsig(u32 gpio);
u32 measure_init();
u32 measure_sample(u32 start);
u32 measure_close();

//---------------------------------------------------------
/*port
 */
//phw_dev
void *__port_init(u32 arg);

//---------------------------------------------------------
/*reset
 */
//phw_dev
void *__reset_source_record(u32 arg);
void reset_source_dump();

//---------------------------------------------------------
/*pmu_wakeup
 */
//phw_dev
void *__wakeup_source_record(u32 arg);
void __pmu_wakeup_init(struct _phw_dev *dev, u32 arg);

void p33_io_wakeup_dump();
void wakeup_source_dump();

//----------------------------------------------------------
/*p11
 */
void p11_key_patch();
//phw_dev
u32 __p11_ioctl(struct _phw_dev *dev, u32 cmd, u32 arg);
u32 __p11_init();
void *__p11_early_init(u32 arg);

//---------------------------------------------------------
/*p33
 */
//phw_dev
u32 __p33_ioctl(struct _phw_dev *dev, u32 cmd, u32 arg);

//---------------------------------------------------------
/*poff
 */

u32 __lp_flow_deepsleep_enter();
u32 __lp_flow_deepsleep_exit();
void rom_gpio_enter_deep_sleep();
void rom_gpio_exit_deep_sleep();
void clock_enter_deep_sleep();
void clock_exit_deep_sleep();
void mmu_exit_deepsleep();

void __system_enter_deepsleep(void);

#endif
