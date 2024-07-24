
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := q32s-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/q32s-lib
SYS_INC_DIR := C:/JL/pi32/q32s-include
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := ..\..\..\..\..\tools\utils\fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := ../../../../../apps/app/post_build/bd47/download.bat
RUN_POST_SCRIPT := ..\..\..\..\..\apps\app\post_build\bd47\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/q32s/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := ../../../../../apps/app/post_build/bd47/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := ../../../../../apps/app/post_build/bd47/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs
# 工程路径前缀
ROOT_PREFIX := ../../../../..

# 编译参数设置
CFLAGS := \
	-flto \
	-target q32s \
	-integrated-as \
	-fno-builtin \
	-mllvm -inline-threshold=5 \
	-Oz \
	-integrated-as \
	-g \
	-O0 \
	-flto \
	-Os \
	-Wcast-align \
	-fallow-pointer-null \
	-Wincompatible-pointer-types \
	-Wundef \
	-fprefer-gnu-section \
	-Wframe-larger-than=256 \
	-Wreturn-type \
	-Wimplicit-function-declaration \
	-Werror=implicit-function-declaration \
	-Werror=return-type \
	-Werror=undef \
	-fms-extensions \
	-Wno-format \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-D__FPGA=0 \
	-DCONFIG_CPU_BD47=1 \
	-DAPP_BT_BLE=1 \
	-DSUPPORT_MS_EXTENSIONS \
	-DSUPPORT_RESERVATION_RAM \
	-DAPP_CASE_TRANSFER \
	-DSDK_VERSION_CFG_DEFINE=0x110003 \
	-DSDK_VERSION_DATE_DEFINE=20240717 \
	-DCONFIG_RELEASE_ENABLE \
	-DHAS_VFS_EN=1 \
	-DNOFLOAT \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-I../../../../../apps/include_lib/fs/sydf \
	-I../../../../../apps/app/bsp/common/bt_common \
	-I../../../../../apps/include_lib/update \
	-I../../../../../apps/include_lib/update/code_v2 \
	-I../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update \
	-I../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_hid \
	-I../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp \
	-I../../../../../apps/app/bsp/common/third_party_profile/common \
	-I../../../../../apps/include_lib/bt_include/third_party/rcsp \
	-I../../../../../apps/examples/transfer/include \
	-I../../../../../apps/app/bsp \
	-I../../../../../apps/app/bsp/start \
	-I../../../../../apps/app/bsp/common/interphone \
	-I../../../../../apps/app/bsp/common/key \
	-I../../../../../apps/app/bsp/common/usb \
	-I../../../../../apps/app/bsp/common/power_manage \
	-I../../../../../apps/app/bsp/common/led \
	-I../../../../../apps/app/bsp/common/common_uart \
	-I../../../../../apps/app/bsp/common/mouse_sensor \
	-I../../../../../apps/app/bsp/common/code_switch \
	-I../../../../../apps/app/bsp/cpu/bd47 \
	-I../../../../../apps/include_lib/common \
	-I../../../../../apps/include_lib/cpu/bd47 \
	-I../../../../../apps/include_lib/liba/ARCH/q32s/header \
	-I../../../../../apps/include_lib/bt_controller_include \
	-I../../../../../apps/include_lib/bt_controller_include/adapter/include/common \
	-I../../../../../apps/include_lib/bt_controller_include/port/bd47 \
	-I../../../../../apps/include_lib/bt_include \
	-I../../../../../apps/include_lib/bt_include/third_party/common \
	-I../../../../../apps/include_lib/config \
	-I../../../../../apps/include_lib/dev_mg \
	-I../../../../../apps/app/bsp/common/third_party_profile/jieli \
	-I../../../../../apps/app/bsp/common/third_party_profile/jieli/gatt_common \
	-I../../../../../apps/app/bsp/common/temp_trim \
	-I../../../../../apps/demo/transfer \
	-I../../../../../apps/demo/transfer/include \
	-I../../../../../apps/app/bsp/common/usb/device \
	-I../../../../../apps/app/bsp/common/usb/host \
	-I../../../../../apps/include_lib/device/usb \
	-I../../../../../apps/include_lib/device/usb/device \
	-I../../../../../apps/include_lib/device/usb/host \
	-I../../../../../apps/demo \
	-I../../../../../apps/include_lib \
	-I../../../../../apps/app/post_build/bd47 \
	-I../../../../../apps/include_lib/audio \
	-I../../../../../apps/include_lib/audio_eq \
	-I../../../../../apps/include_lib/cpu \
	-I../../../../../apps/maske_include \
	-I../../../../../apps/app/bsp/lib \
	-I../../../../../apps/app/bsp/cpu/bd47/entry \
	-I../../../../../apps/app/bsp/common \
	-I../../../../../apps/app/bsp/device \
	-I../../../../../apps/app/bsp/common/usb/usr \
	-I../../../../../apps/include_lib/msg \
	-I../../../../../apps/include_lib/fs \
	-I../../../../../apps/include_lib/decoder \
	-I../../../../../apps/include_lib/decoder/list \
	-I../../../../../apps/include_lib/sound_mge \
	-I../../../../../apps/include_lib/device \
	-I../../../../../apps/include_lib/device/sdmmc \
	-I../../../../../apps/include_lib/device/norflash \
	-I../../../../../apps/demo/transfer/board/bd47 \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	../../../../../apps/app/bsp/common/code_switch/code_switch.c \
	../../../../../apps/app/bsp/common/common_uart/common_uart_control.c \
	../../../../../apps/app/bsp/common/config/lib_power_config.c \
	../../../../../apps/app/bsp/common/fs/sydf/sydf_resource.c \
	../../../../../apps/app/bsp/common/fs/vfs.c \
	../../../../../apps/app/bsp/common/fs/vfs_fat.c \
	../../../../../apps/app/bsp/common/fs/vfs_resource.c \
	../../../../../apps/app/bsp/common/ir/ir_decoder.c \
	../../../../../apps/app/bsp/common/ir/ir_encoder.c \
	../../../../../apps/app/bsp/common/key/key.c \
	../../../../../apps/app/bsp/common/key/key_drv_ad.c \
	../../../../../apps/app/bsp/common/key/key_drv_io.c \
	../../../../../apps/app/bsp/common/key/key_drv_matrix.c \
	../../../../../apps/app/bsp/common/led/led_control.c \
	../../../../../apps/app/bsp/common/ledc/ledc.c \
	../../../../../apps/app/bsp/common/ledc/ledc_demo.c \
	../../../../../apps/app/bsp/common/mouse_sensor/OMSensor_manage.c \
	../../../../../apps/app/bsp/common/mouse_sensor/hal3205/hal3205.c \
	../../../../../apps/app/bsp/common/mouse_sensor/hal3212/hal3212.c \
	../../../../../apps/app/bsp/common/msg/msg.c \
	../../../../../apps/app/bsp/common/msg/sys_event.c \
	../../../../../apps/app/bsp/common/my_malloc.c \
	../../../../../apps/app/bsp/common/power_manage/app_power_mg.c \
	../../../../../apps/app/bsp/common/rdec_soft/rdec_soft.c \
	../../../../../apps/app/bsp/common/rtc/rtc_demo.c \
	../../../../../apps/app/bsp/common/sys_memory/sys_memory.c \
	../../../../../apps/app/bsp/common/tab_read.c \
	../../../../../apps/app/bsp/common/temp_trim/adc_dtemp_alog.c \
	../../../../../apps/app/bsp/common/temp_trim/dtemp_pll_trim.c \
	../../../../../apps/app/bsp/common/third_party_profile/common/custom_cfg.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_bluetooth.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_hid/rcsp_hid_inter.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_msg.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update/rcsp_ch_loader_download.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update/rcsp_user_update.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_client.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_common.c \
	../../../../../apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_server.c \
	../../../../../apps/app/bsp/common/update/testbox_uart_update.c \
	../../../../../apps/app/bsp/common/update/testbox_update.c \
	../../../../../apps/app/bsp/common/update/update.c \
	../../../../../apps/app/bsp/common/usb/device/cdc.c \
	../../../../../apps/app/bsp/common/usb/device/custom_hid.c \
	../../../../../apps/app/bsp/common/usb/device/descriptor.c \
	../../../../../apps/app/bsp/common/usb/device/hid_keyboard.c \
	../../../../../apps/app/bsp/common/usb/device/hid_media.c \
	../../../../../apps/app/bsp/common/usb/device/hid_mouse.c \
	../../../../../apps/app/bsp/common/usb/device/msd.c \
	../../../../../apps/app/bsp/common/usb/device/msd_upgrade.c \
	../../../../../apps/app/bsp/common/usb/device/task_pc.c \
	../../../../../apps/app/bsp/common/usb/device/uac1.c \
	../../../../../apps/app/bsp/common/usb/device/usb_device.c \
	../../../../../apps/app/bsp/common/usb/device/usb_device_config.c \
	../../../../../apps/app/bsp/common/usb/device/usb_suspend_resume.c \
	../../../../../apps/app/bsp/common/usb/device/user_setup.c \
	../../../../../apps/app/bsp/common/vm/nvm_api.c \
	../../../../../apps/app/bsp/common/vm/vm_api.c \
	../../../../../apps/app/bsp/cpu/bd47/adc_api.c \
	../../../../../apps/app/bsp/cpu/bd47/cpu_debug.c \
	../../../../../apps/app/bsp/cpu/bd47/device.c \
	../../../../../apps/app/bsp/cpu/bd47/led/led_api.c \
	../../../../../apps/app/bsp/cpu/bd47/led/led_api_test.c \
	../../../../../apps/app/bsp/cpu/bd47/led/pwm_led.c \
	../../../../../apps/app/bsp/cpu/bd47/my_print.c \
	../../../../../apps/app/bsp/cpu/bd47/power/extern_dcdc/extern_dcdc.c \
	../../../../../apps/app/bsp/cpu/bd47/power/power_app.c \
	../../../../../apps/app/bsp/cpu/bd47/power/power_config.c \
	../../../../../apps/app/bsp/cpu/bd47/power_api.c \
	../../../../../apps/app/bsp/cpu/bd47/tick_timer_sys.c \
	../../../../../apps/app/bsp/cpu/bd47/uart.c \
	../../../../../apps/app/bsp/device/device_app.c \
	../../../../../apps/app/bsp/device/device_list.c \
	../../../../../apps/app/bsp/start/bd47/main.c \
	../../../../../apps/app/bsp/start/flash_init.c \
	../../../../../apps/app/bsp/start/init.c \
	../../../../../apps/demo/transfer/app_main.c \
	../../../../../apps/demo/transfer/board/bd47/board_aw318n_dongle.c \
	../../../../../apps/demo/transfer/board/bd47/board_aw31n_demo.c \
	../../../../../apps/demo/transfer/config/lib_btctrler_config.c \
	../../../../../apps/demo/transfer/config/lib_btstack_config.c \
	../../../../../apps/demo/transfer/config/lib_driver_config.c \
	../../../../../apps/demo/transfer/config/lib_profile_config.c \
	../../../../../apps/demo/transfer/config/lib_system_config.c \
	../../../../../apps/demo/transfer/config/lib_update_config.c \
	../../../../../apps/demo/transfer/config/log_config.c \
	../../../../../apps/demo/transfer/examples/at_char_com/app_at_char_com.c \
	../../../../../apps/demo/transfer/examples/at_char_com/at_char_cmds.c \
	../../../../../apps/demo/transfer/examples/at_char_com/at_char_uart.c \
	../../../../../apps/demo/transfer/examples/at_char_com/ble_at_char.c \
	../../../../../apps/demo/transfer/examples/at_char_com/ble_at_char_client.c \
	../../../../../apps/demo/transfer/examples/at_char_com/ble_at_char_server.c \
	../../../../../apps/demo/transfer/examples/dongle/app_dongle.c \
	../../../../../apps/demo/transfer/examples/dongle/ble_dg_central.c \
	../../../../../apps/demo/transfer/examples/dongle/dg_audio_demo.c \
	../../../../../apps/demo/transfer/examples/idle/app_idle.c \
	../../../../../apps/demo/transfer/examples/multi_conn/app_multi_conn.c \
	../../../../../apps/demo/transfer/examples/multi_conn/ble_multi.c \
	../../../../../apps/demo/transfer/examples/multi_conn/ble_multi_central.c \
	../../../../../apps/demo/transfer/examples/multi_conn/ble_multi_peripheral.c \
	../../../../../apps/demo/transfer/examples/nonconn_trans/app_nonconn_trans.c \
	../../../../../apps/demo/transfer/examples/nonconn_trans/ble_noconn_deal.c \
	../../../../../apps/demo/transfer/examples/trans_data/app_trans.c \
	../../../../../apps/demo/transfer/examples/trans_data/ble_trans.c \
	../../../../../apps/demo/transfer/modules/app_comm_proc.c \
	../../../../../apps/demo/transfer/modules/bt/app_comm_ble.c \
	../../../../../apps/demo/transfer/modules/user_cfg.c \
	../../../../../apps/demo/transfer/version.c \


# 需要编译的 .S 文件
S_SRC_FILES :=


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=save-temps \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=-warn-stack-size=256 \
	--sort-common \
	--dont-complain-call-overflow \
	--gc-sections \
	--start-group \
	../../../../../apps/include_lib/liba/bd47/flash/agreement.a \
	../../../../../apps/include_lib/liba/bd47/flash/update_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/cpu_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/fs_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/malloc_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/sys_timer_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/bt_controller_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/bt_protocol_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/crypto_toolbox_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/periphy_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/new_vm_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/printf_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/dev_mg_lib.a \
	../../../../../apps/include_lib/liba/bd47/flash/lib_ima_stream_decode.a \
	../../../../../apps/include_lib/liba/bd47/flash/lib_sbc_stddec.a \
	--end-group \
	-T../../../../../apps/app/post_build/bd47/app.ld \
	-M=../../../../../apps/app/post_build/bd47/app.map \
	-flto \
	--plugin-opt=-inline-threshold=5 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS:$(ROOT_PREFIX)/%=%))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES:$(ROOT_PREFIX)/%=%))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../../apps/app/post_build/bd47/flash/app_ld.c -o ../../../../../apps/app/post_build/bd47/app.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../../apps/app/post_build/bd47/flash/download_bat.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../../apps/app/post_build/bd47/flash/download_sh.c -o ../../../../../apps/app/post_build/bd47/download.sh
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../../apps/app/post_build/bd47/flash/isd_config_ini.c -o ../../../../../apps/app/post_build/bd47/isd_config.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : $(ROOT_PREFIX)/%.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.S.o : $(ROOT_PREFIX)/%.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.s.o : $(ROOT_PREFIX)/%.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : $(ROOT_PREFIX)/%.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : $(ROOT_PREFIX)/%.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cc.o : $(ROOT_PREFIX)/%.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

-include $(DEP_FILES)
