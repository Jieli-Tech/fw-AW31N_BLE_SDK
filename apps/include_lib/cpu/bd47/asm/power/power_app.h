#ifndef __POWER_APP_H__
#define __POWER_APP_H__

void power_early_flowing();
int power_later_flowing();

void board_power_init();

void extern_dcdc_switch(u32 sw);

void extern_dcdc_en(u32 en, u32 delay_us);

void ldo_en(u32 en);

#endif
