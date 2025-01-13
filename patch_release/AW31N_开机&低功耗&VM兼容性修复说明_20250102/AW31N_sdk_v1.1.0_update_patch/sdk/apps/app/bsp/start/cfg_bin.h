#ifndef __CFG_BIN_H__
#define __CFG_BIN_H__


#define SYSCFG_DEFAULT_BIN_PATH 	"/cfg_tool.bin"  //默认配置项文件

#define CFGBIN_BLE_NAME_OFFSET      (0x9)
#define CFGBIN_MAC_ADDR_OFFSET      (0x2a0)
#define CFGBIN_RF_POWER_OFFSET      (0x2aa)

#define RESERVED_MAC_ADDR_OFFSET    (256 - 64)
#define RESERVED_AUTH_CODE_OFFSET   (256 - 64 - 16)

#define AUTH_MAX_LEN                (2 + 2 + 128)   //crc:2 data_len:2 data:128


/*****************auth*******************/
typedef struct __auth_header {
    u16 crc16;
    u16 data_len;
    const u8 data_p[];
} __attribute__((packed)) auth_header_t;

void rsv_auth_analysis(void);
/*****************auth end*******************/



int cfg_bin_init(void);

#endif

