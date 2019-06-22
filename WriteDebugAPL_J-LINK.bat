set MCUX_WORKSPACE_LOC=D:/Workspace/IMXRT1060-EVK
set MCUX_JLINK=C:/Program Files (x86)/SEGGER/JLink_V644i/

"%MCUX_JLINK%/JLink.exe" -if SWD -speed auto -commanderscript "%MCUX_WORKSPACE_LOC%/IMXRT1060-EVK_APL/script.jlink" -device MIMXRT1062xxx6A -SelectEmuBySN 260114942 

pause
