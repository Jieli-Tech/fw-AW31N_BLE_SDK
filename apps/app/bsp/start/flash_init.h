#ifndef __FLASH_INIT_H__
#define __FLASH_INIT_H__


int flash_info_init(void);
void flash_system_init(void);
void flash_code_set_unprotect(void);
void flash_code_set_protect(void);
#endif
