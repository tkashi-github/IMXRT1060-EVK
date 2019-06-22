# IMXRT1060-EVK Sample Project

 ## Notes:
 This project is Sample Project For IMXRT1060-EVK.
 * Modify MCUX_IDE_DIR in WriteDebugApl.bat/WriteDebugBoot.bat.
 * For MCUXpressoIDE_11.0.0
 * Maybe, MMC or MMC compatible card support.
 * Update Main Program by BIN File. (please put "IMXRT1060-EVK_APL_crc16.bin" root file system)
 
 ## License
 * The MIT license applies to the code I created in the "source" directory. 
 * This project also uses "MCUXpresso Software Development Kit (SDK) 2.6.0". 
 * Please refer to "SW-Content-Register.txt" for the license of SDK.
       
 ## Software
 * CMSIS-5
 * Amazon FreeRTOS
 * FatFs
 * lwip
 * lvgl

----
 ## IMXRT1060-EVK_BOOT
  This project is a bootloader. A main functions are as follows.
  * Console (source/Task/consoleTask.c, h)
  * File System (source/Task/StorageTask.c, h)
  * Load another binary and jump (update.c)

----
 ## IMXRT1060-EVK_APL
  This project is a sample application. A main functions are as follows.
  * Console (source/Task/consoleTask.c, h)
  * File System (source/Task/StorageTask.c, h)
  * Lan (lwip)
  * Temperature Monitor (source/Task/TempMonieTask.c, h)
  * Touch screen (source/Task/TouchScreenTask.c, h)
  * GUI (lvgl)

----
# !!indent is Tab and width is 4!!
 
