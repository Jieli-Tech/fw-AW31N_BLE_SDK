#include "board_config.h"
[EXTRA_CFG_PARAM]
NEW_FLASH_FS = YES;
CHIP_NAME = AW31N; //8
ENTRY = 0x4000100; //程序入口地址
PID = AW31N; //长度16byte,示例：芯片封装_应用方向_方案名称
VID = 0.01;
RESERVED_OPT = 0;

/* #DOWNLOAD_MODEL=SERIAL; */
DOWNLOAD_MODEL = usb;
SERIAL_DEVICE_NAME = JlVirtualJtagSerial;
SERIAL_BARD_RATE = 1000000;
SERIAL_CMD_OPT = 2;
SERIAL_CMD_RATE = 100;// [n * 10000]
SERIAL_CMD_RES = 0;
SERIAL_INIT_BAUD_RATE = 9600;
LOADER_BAUD_RATE = 1000000;
LOADER_ASK_BAUD_RATE = 1000000;
BEFORE_LOADER_WAIT_TIME = 150;
SERIAL_SEND_KEY = yes;

/* #####################################################    UBOOT配置项，请勿随意调整顺序    ################################################## */
[SYS_CFG_PARAM]
/* #data_width[0 1 2 3 4] 3的时候uboot自动识别2或者4线 */
/* #clk [0-255] */
/* #mode: */
/* #	  0 RD_OUTPUT,		 1 cmd 		 1 addr  */
/* #	  1 RD_I/O,   		 1 cmd 		 x addr */
/* #	  2 RD_I/O_CONTINUE] no_send_cmd x add */
/* #port: */
/* #	  0  优先选A端口  CS:PD3  CLK:PD0  D0:PD1  D1:PD2  D2:PB7  D3:PD5 */
/* #	  1  优先选B端口  CS:PA13 CLK:PD0  D0:PD1  D1:PA14 D2:PA15 D3:PD5 */
SPI = 2_3_0_0;
/* #OSC=btosc; */
/* #OSC_FREQ=12MHz; #[24MHz 12MHz] */
/* #SYS_CLK=24MHz;	#[48MHz 24MHz] */
#if TCFG_CLOCK_OSC_1PIN_EN
OSC_1PIN = 1; //单脚晶振
#endif
UTTX = PA03; //uboot串口tx
UTBD = 1000000; //uboot串口波特率
UTRX = PA00;
// 串口升级[PB00 PB05 PA05]
/* #RESET=PB01_00_0;	//port口_长按时间_有效电平（长按时间有00、01、02、04、08三个值可选，单位为秒，当长按时间为00时，则关闭长按复位功能。） */
/*  */
/* # 外部FLASH 硬件连接配置 */
/* #EX_FLASH=PC03_1A_NULL;	//CS_pin / spi (0/1/2) /port(A/B) / power_io */
/* #EX_FLASH_IO=2_PC01_PC02_PC04_PC05_PC00;	//data_width / CLK_pin / DO_pin / DI_pin / D2_pin / D3_pin   当data_width为4的时候，D2_pin和D3_pin才有效 */
/*  */
/* #0:disable */
/* #1:PA9 PA10  */
/* #2:USB */
/* #3:PB1 PB2 */
/* #4:PB6 PB7 */
/*  */
/* #sdtap=2; */

[FW_ADDITIONAL]
#if UPDATE_V2_EN
FILE_LIST = (file = ota.bin: type = 100)
#endif
            /* ############################################################################################################################################# */
            /*  */
            /*  */
            /*  */
            /* ########flash空间使用配置区域############################################### */
            /* #PDCTNAME:    产品名，对应此代码，用于标识产品，升级时可以选择匹配产品名 */
            /* #BOOT_FIRST:  1=代码更新后，提示APP是第一次启动；0=代码更新后，不提示 */
            /* #UPVR_CTL：   0：不允许高版本升级低版本   1：允许高版本升级低版本 */
            /* #XXXX_ADR:    区域起始地址	AUTO：由工具自动分配起始地址 */
            /* #XXXX_LEN:    区域长度		CODE_LEN：代码长度 */
            /* #XXXX_OPT:    区域操作属性 */
            /* # */
            /* # */
            /* # */
            /* #操作符说明  OPT: */
            /* #	0:  下载代码时擦除指定区域 */
            /* #	1:  下载代码时不操作指定区域 */
            /* #	2:  下载代码时给指定区域加上保护 */
            /* ############################################################################ */
            [RESERVED_CONFIG]
            BTIF_ADR = AUTO;
BTIF_LEN = 0x1000;
BTIF_OPT = 1;

/* #WTIF_ADR=BEGIN_END; */
/* #WTIF_LEN=0x1000; */
/* #WTIF_OPT=1; */
/*  */
/* #PRCT_ADR=AUTO; */
/* #PRCT_LEN=8K; */
/* #PRCT_OPT=2; */

VM_ADR = AUTO;
VM_LEN = 8K;
VM_OPT = 1;

EEPROM_ADR = AUTO;
EEPROM_LEN = 8K;
EEPROM_OPT = 1;

[BURNER_CONFIG]
SIZE = 32;

