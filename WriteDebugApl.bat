set MCUX_WORKSPACE_LOC=.
set MCUX_FLASH_DIR=./MIMXRT1062xxxxA_support/Flash
set MCUX_IDE_DIR=D:/nxp/MCUXpressoIDE_10.2.1_795/ide/
set MCUX_IDE_BIN=%MCUX_IDE_DIR%/bin

%MCUX_IDE_BIN%/crt_emu_cm_redlink --flash-load "%MCUX_WORKSPACE_LOC%/IMXRT1060-EVK_APL/Debug/IMXRT1060-EVK_APL_crc16.bin" -p MIMXRT1062xxxxA --load-base=0x60400000 --flash-driver-reset SYSRESETREQ -x %MCUX_WORKSPACE_LOC%/IMXRT1060-EVK_BOOT/Debug --flash-dir %MCUX_FLASH_DIR% 

pause
