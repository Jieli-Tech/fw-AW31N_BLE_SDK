#ifndef __KEY_DRV_MATRIX_H__
#define __KEY_DRV_MATRIX_H__

#include "gpio.h"
#include "key.h"
#include "app_config.h"

#if KEY_MATRIX_EN

#define KEY_DETECT_LEVEL 0 //1:检测高电平; 0:检测低电平

static u32 matrix_key_row_value = 0xffffffff;
static u32 matrix_key_col_value = 0xffffffff;

static uint8_t matrix_key_row[] = { //normal io
    MATRIX_KEY_ROW1,
    MATRIX_KEY_ROW2,
    MATRIX_KEY_ROW3
};
static uint8_t matrix_key_col[] = { //normal io or reuse lcd seg io
    MATRIX_KEY_ROL1,
    MATRIX_KEY_ROL2,
    MATRIX_KEY_ROL3
};


extern const key_interface_t key_matrix_info;
void matrix_key_init(void);
void matrix_key_suspend();
void matrix_key_release();
uint8_t get_matrixkey_value(void);

#endif
#endif/*__KEY_DRV_MATRIX_H__*/
