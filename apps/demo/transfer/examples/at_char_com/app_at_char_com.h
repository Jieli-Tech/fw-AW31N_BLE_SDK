#ifndef __APP_AT_CHAR_COM_H
#define __APP_AT_CHAR_COM_H

#if CONFIG_APP_AT_CHAR_COM

void at_set_low_power_mode(uint8_t enable);
uint8_t at_get_low_power_mode(void);
void atchar_set_soft_poweroff(void);
#endif
#endif
