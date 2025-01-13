


set ELF_NAME=%1%
cd /d %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe

%OBJDUMP% -d -print-imm-hex -print-dbg %ELF_NAME%.elf > %ELF_NAME%.lst
%OBJCOPY% -O binary -j .app_code %ELF_NAME%.elf %ELF_NAME%.bin
%OBJCOPY% -O binary -j .data %ELF_NAME%.elf data.bin
%OBJCOPY% -O binary -j .lowpower_overlay %ELF_NAME%.elf lowpower_overlay.bin
%OBJCOPY% -O binary -j .update_overlay %ELF_NAME%.elf update_overlay.bin
%OBJDUMP% -section-headers %ELF_NAME%.elf
copy /b %ELF_NAME%.bin+data.bin+lowpower_overlay.bin+update_overlay.bin app.bin


@echo *******************************************************************************************************
@echo BD47 flash
@echo *******************************************************************************************************
@echo % date %
cd / d % ~dp0

isd_download.exe -tonorflash -dev bd47 -boot 0x3f31000 -div8 -wait 300 -uboot uboot.boot -app app.bin -res cfg_tool.bin

ufw_maker.exe -fw_to_ufw jl_isd.fw
copy jl_isd.ufw update.ufw
del jl_isd.ufw

::-format all
@REM 常用命令说明
@rem - format vm
@rem - format all
@rem - reboot 500

ping / n 2 127.1 > null
IF EXIST null del null
