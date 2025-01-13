// *INDENT-OFF*
/* #include "app_config.h" */
#include  "maskrom_stubs.ld"
#include "irq_vec.h"
#include "board_config.h"
#include "asm/power/power_defined.h"

_BOOT_RAM_SIZE  = 0x24;
_BOOT_RAM_BEGIN = _MASK_EXPORT_MEM_BEGIN - _BOOT_RAM_SIZE;
_NOT_KEEP_RAM_SIZE = 0x3000;

UPDATA_SIZE     = 0x80;
UPDATA_BEG      = _MASK_EXPORT_MEM_BEGIN - UPDATA_SIZE;

//config
MEMORY
{
    // app_code: LENGTH = 32M-0x100, 超过32M运行代码需要使用长跳转;
    app_code(rx)        : ORIGIN = _SFC_MEMORY_START_ADDR + 0x100,  LENGTH = 32M-0x100
    irq_vec(rx)         : ORIGIN = _IRQ_MEM_ADDR,                   LENGTH = MAX_IRQ_ENTRY_NUM * 4
    ram0(rw)            : ORIGIN = _UBOOT_LOADER_RAM_START,         LENGTH = _BOOT_RAM_BEGIN - _UBOOT_LOADER_RAM_START
    boot_ram(rw)        : ORIGIN = _BOOT_RAM_BEGIN,                 LENGTH = _BOOT_RAM_SIZE

	unuse_mem(rw)		: ORIGIN = 0x40000000,						LENGTH = 0x40000000
}
ENTRY(_start)

SECTIONS
{

	. = ORIGIN(boot_ram);
	.boot_data ALIGN(4):
	{
		*(.boot_info)
	} > boot_ram

    /* L1 memory sections */
    . = ORIGIN(ram0);
	.debug_data ALIGN(4):
	{
        PROVIDE(debug_buf_start = .);
        *(.debug.data.bss)
        *(.debug.data)
        . = (. + 3) / 4 * 4 ;
    } > ram0

    /*12k---低功耗poweroff下可掉电ram,start*/
    .nk_bss (NOLOAD) : SUBALIGN(4)
    {
        nk_ram_start = .;
      *(.not_keep_ram)
      /* *(.rcsp_not_keep_ram) */
        . = ALIGN(4);
		*(.sstack_magic);
        . = ALIGN(4);
        _cpu0_sstack_begin = .;
        PROVIDE(cpu0_sstack_begin = .);
        KEEP(*(.sstack))
        . = ALIGN(4);
        _cpu0_sstack_end = .;
        PROVIDE(cpu0_sstack_end = .);
        KEEP(*(.sstack_top))
        . = ALIGN(4);
		*(.sstack_magic0);
        . = ALIGN(4);

        PROVIDE(_nk_ram_malloc_start = .);
        KEEP(*(.sec_bt_nk_ram))
        PROVIDE(_nk_ram_malloc_end = .);

        PROVIDE(_nk_ram_remain_start = .);
#if TCFG_LOWPOWER_LOWPOWER_SEL
        . = _NOT_KEEP_RAM_SIZE + ORIGIN(ram0);
#endif
        PROVIDE(nk_ram_end = .);
    } > ram0


    /*16k---低功耗pretention ram,poweroff不掉电,start*/
    .data ALIGN(4):
    {
        PROVIDE(data_buf_start = .);
        *(.data_magic)
        *(.data)
        *(.*.data)
        *(.common)

#if (TCFG_LOWPOWER_LOWPOWER_SEL == 0)
            *(.power_driver_osc.data.overlay)
#endif
		_bt_lib_data_start = .;
        #include "bt_include/btstack_lib_data.ld"
        #include "bt_controller_include/btctler_lib_data.ld"
		_bt_lib_data_end = .;

//        *(.btram.data.overlay)

		cache_Lx_code_text_begin = .;

		*muldi3*.o(.text .rodata*)
		*udivdi3*.o(.text .rodata*)
		*udivmoddi4.o(.text .rodata*)

		*(.audio_isr_text)
        *(.*.text.cache.L1)
        *(.*.text.cache.L2)
        *(.*.text.cache.L3)
        *(.log_ut_text)
        . = (. + 3) / 4 * 4 ;

    } > ram0

   	OVERLAY : AT(0x200000) SUBALIGN(4)
    {
		.lowpower_overlay
		{
        	PROVIDE(power_driver_data_start = .);
#if TCFG_LOWPOWER_LOWPOWER_SEL
        	PROVIDE(power_driver_osc_data_start = .);
            *(.power_driver_osc.data.overlay)
            *(.power_driver_osc.text.cache.L1.overlay)
            PROVIDE(power_driver_osc_data_end = .);
#endif
            *(.power_driver.data.overlay)
            *(.power_driver.text.cache.L1.overlay)
            PROVIDE(power_driver_data_end = .);
		}

		.update_overlay
		{
        	PROVIDE(update_data_start = .);
            *(.update.data.overlay)
        	update_data_overlay_end = .;
            *(.update.data.local_val.overlay) // 大的局部变量共用该段(避免爆栈)
            PROVIDE(update_data_end = .);
		}
	} > ram0

	cache_Lx_code_text_end = .;

    .bss (NOLOAD) : SUBALIGN(4)
    {
        PROVIDE(bss_buf_start = .);
        . = ALIGN(4);
		*(.stack_magic);
        . = ALIGN(4);
        _cpu0_ustack_begin = .;
        PROVIDE(cpu0_ustack_begin = .);
        KEEP(*(.ustack))
        . = ALIGN(4);
        _cpu0_ustack_end = .;
        PROVIDE(cpu0_ustack_end = .);
        KEEP(*(.ustack_top))
        . = ALIGN(4);
		*(.stack_magic0);
		. = ALIGN(4);
		_system_data_begin = .;
        *(.bss)
        *(.*.data.bss)
        *(.non_volatile_ram)

#if (TCFG_LOWPOWER_LOWPOWER_SEL == 0)
            *(.power_driver_osc.data.bss.overlay)
#endif
		. = ALIGN(4);
		_bt_lib_bss_start = .;
        #include "bt_include/btstack_lib_bss.ld"
        #include "bt_controller_include/btctler_lib_bss.ld"
		_bt_lib_bss_end = .;

        _system_data_end = .;
    } > ram0

	OVERLAY : AT(0x210000) SUBALIGN(4)
	{
		.lowpower_bss_overlay
		{
		    PROVIDE(power_driver_data_bss_start = .);
#if (TCFG_LOWPOWER_LOWPOWER_SEL)
            *(.power_driver_osc.data.bss.overlay)
#endif
            *(.power_driver.data.bss.overlay)
            PROVIDE(power_driver_data_bss_end = .);
		}

		.update_bss_overlay
		{
        	PROVIDE(update_bss_start = .);
            *(.update.bss.overlay)
            PROVIDE(update_bss_end = .);
		}

	} > ram0

    .nv_ram_malloc ALIGN(4)://for bt
    {
        PROVIDE(_nv_ram_malloc_start = .);
        KEEP(*(.sec_bt_nv_ram))
        PROVIDE(_nv_ram_malloc_end = .);
    } > ram0

     .heap_buf ALIGN(4)://for systerm timer
    {
        PROVIDE(_free_start = .);
        KEEP(*(.sec_sys_heap))
        PROVIDE(_nv_ram_remain_start = .);
        . = LENGTH(ram0) + ORIGIN(ram0);
        PROVIDE(_free_end = .);
    } > ram0

     PROVIDE(nv_ram_end = .);
     _ram_end = .;

    . = ORIGIN(app_code);
    .app_code ALIGN(32):
    {

        app_code_text_begin = .;
        KEEP(*(.chip_entry_text))
        *(.start.text)
        *(.*.text.const)
        *(.*.text)
        *(.version)
        *(.debug)
        *(.debug.text.const)
        *(.debug.text)
        *(.debug.string)
        *(.text)
#if (TCFG_LOWPOWER_LOWPOWER_SEL == 0)
            *(.power_driver_osc.text.cache.L1.overlay)
#endif
        . = ALIGN(4);
        #include "bt_include/btstack_lib_text.ld"
		. = ALIGN(4);
        #include "bt_controller_include/btctler_lib_text.ld"

        _SPI_CODE_START = .;
        *(.spi_code)
        . = ALIGN(4);
        _SPI_CODE_END = .;
        *(.rodata*)
        *(.ins)
        app_code_text_end = . ;

        . = ALIGN(4);
        vfs_ops_begin = .;
        KEEP(*(.vfs_operations))
        vfs_ops_end = .;

        . = ALIGN(4);
        app_main_begin = .;
        KEEP(*(.app_main))
        app_main_end = .;

        . = ALIGN(4);
        //mouse sensor dev begin
        OMSensor_dev_begin = .;
        KEEP(*(.omsensor_dev))
        OMSensor_dev_end = .;

        . = ALIGN(4);
        PROVIDE(device_node_begin = .);
        KEEP(*(.device))
        PROVIDE(device_node_end = .);

        . = ALIGN(4);
        hsb_critical_handler_begin = .;
        KEEP(*(.hsb_critical_txt))
        hsb_critical_handler_end = .;

        . = ALIGN(4);
        lsb_critical_handler_begin = .;
        KEEP(*(.lsb_critical_txt))
        lsb_critical_handler_end = .;

        . = ALIGN(4);
        loop_detect_handler_begin = .;
        KEEP(*(.loop_detect_region))
        loop_detect_handler_end = .;

#if TCFG_LOWPOWER_LOWPOWER_SEL
		. = ALIGN(4);
		lp_target_begin = .;
	    PROVIDE(lp_target_begin = .);
	    KEEP(*(.lp_target))
		lp_target_end = .;
	    PROVIDE(lp_target_end = .);

		. = ALIGN(4);
	    lp_request_begin = .;
	    PROVIDE(lp_request_begin = .);
	    KEEP(*(.lp_request))
	    lp_request_end = .;
	    PROVIDE(lp_request_end = .);

#endif

#if (TCFG_LOWPOWER_LOWPOWER_SEL == DEEP_SLEEP_EN)
		. = ALIGN(4);
        deepsleep_target_begin = .;
        PROVIDE(deepsleep_target_begin = .);
        KEEP(*(.deepsleep_target))
        deepsleep_target_end = .;
        PROVIDE(deepsleep_target_end = .);
#endif

#if TCFG_LOWPOWER_SOFF
  		platform_uninitcall_begin = .;
  		KEEP(*(.platform.uninitcall))
  		platform_uninitcall_end = .;
#endif

        . = ALIGN(32);
        /* . = LENGTH(app_code) - SIZEOF(.data); */
        text_end = .;
	} >app_code


    . = ORIGIN(unuse_mem);
    .unuse_mem ALIGN(32):
	{
#if (TCFG_LOWPOWER_LOWPOWER_SEL == 0)
	    *(.lp_target)
	    *(.lp_request)
#endif

#if (TCFG_LOWPOWER_LOWPOWER_SEL != DEEP_SLEEP_EN)
        *(.deepsleep_target)
#endif

#if (TCFG_LOWPOWER_SOFF == 0)
  	*(.platform.uninitcall)
#endif
	} >unuse_mem
}

#include "update/code_v2/update.ld"

//================== Section Info Export ====================//
nk_bss_begin     = ADDR(.nk_bss);
nk_bss_size      = SIZEOF(.nk_bss);

nv_ram_begin     = nk_bss_begin + nk_bss_size;

bss_begin       = ADDR(.bss);
bss_size        = SIZEOF(.bss);

/*除堆栈外的bss区*/
bss_size1       = _system_data_end - _system_data_begin;
bss_begin1      = _system_data_begin;

text_begin      = ADDR(.app_code);
text_size       = SIZEOF(.app_code);
text_end        = text_begin + text_size;

data_addr  = ADDR(.data) ;
data_begin = text_end ;
data_size =  SIZEOF(.data) + SIZEOF(.debug_data);

bt_bss         = _bt_lib_bss_end - _bt_lib_bss_start;
bt_data        = _bt_lib_data_end - _bt_lib_data_start;

nk_ram_size      = nk_ram_end - nk_ram_start;
nv_ram_size      = nv_ram_end - nv_ram_begin;
head_buf_size    = _free_end - _free_start;

nk_ram_malloc_size    = _nk_ram_malloc_end -_nk_ram_malloc_start;
nk_ram_remain    = nk_ram_end - _nk_ram_remain_start;

nv_ram_malloc_size    = _nv_ram_malloc_end -_nv_ram_malloc_start;
nv_ram_remain    = nv_ram_end - _nv_ram_remain_start;

//================== power overlay Section Info Export ====================//
lowpower_overlay_addr       = ADDR(.lowpower_overlay);
lowpower_overlay_begin      = data_begin + data_size;
lowpower_overlay_size       = SIZEOF(.lowpower_overlay);
lowpower_overlay_bss_size   = SIZEOF(.lowpower_bss_overlay);

update_overlay_addr       = ADDR(.update_overlay);
update_overlay_begin      = lowpower_overlay_begin + lowpower_overlay_size;
update_overlay_size       = SIZEOF(.update_overlay);

update_overlay_bss_addr       = ADDR(.update_bss_overlay);
update_overlay_bss_size   = SIZEOF(.update_bss_overlay);
#if TCFG_LOWPOWER_LOWPOWER_SEL
update_overlay_size         = SIZEOF(.update_overlay);
update_overlay_bss_size     = SIZEOF(.update_bss_overlay);
#endif

//========================================================================//
rcsp_update_sstack_begin = update_data_overlay_end;
rcsp_update_sstack_end = lowpower_overlay_size + lowpower_overlay_addr;








