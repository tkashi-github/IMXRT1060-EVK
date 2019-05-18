/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008 - 2013 Code Red Technologies Ltd, 
 * Copyright 2015, 2018 NXP
 * (c) NXP Semiconductors 2013-2019
 * Generated linker script file for MIMXRT1062xxxxA
 * Created from memory.ldt by FMCreateLinkMemory
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.3.1 [Build 2233] [2019-02-20] on 2019/05/18 22:54:38
 */

MEMORY
{
  /* Define each memory region */
  Prgram (rwx) : ORIGIN = 0x80000000, LENGTH = 0x400000 /* 4M bytes (alias RAM) */  
  NonCache (rwx) : ORIGIN = 0x80400000, LENGTH = 0x800000 /* 8M bytes (alias RAM2) */  
  HEAP (rwx) : ORIGIN = 0x80c00000, LENGTH = 0x1400000 /* 20M bytes (alias RAM3) */  
  SRAM_ITC (rwx) : ORIGIN = 0x0, LENGTH = 0x40000 /* 256K bytes (alias RAM4) */  
  SRAM_DTC (rwx) : ORIGIN = 0x20000400, LENGTH = 0x3fc00 /* 255K bytes (alias RAM5) */  
  VRAM (rwx) : ORIGIN = 0x20200000, LENGTH = 0x40000 /* 256K bytes (alias RAM6) */  
  SRAM_OC2 (rwx) : ORIGIN = 0x20240000, LENGTH = 0x20000 /* 128K bytes (alias RAM7) */  
  MAIN_STACK (rwx) : ORIGIN = 0x20260000, LENGTH = 0x20000 /* 128K bytes (alias RAM8) */  
}

  /* Define a symbol for the top of each memory region */
  __base_Prgram = 0x80000000  ; /* Prgram */  
  __base_RAM = 0x80000000 ; /* RAM */  
  __top_Prgram = 0x80000000 + 0x400000 ; /* 4M bytes */  
  __top_RAM = 0x80000000 + 0x400000 ; /* 4M bytes */  
  __base_NonCache = 0x80400000  ; /* NonCache */  
  __base_RAM2 = 0x80400000 ; /* RAM2 */  
  __top_NonCache = 0x80400000 + 0x800000 ; /* 8M bytes */  
  __top_RAM2 = 0x80400000 + 0x800000 ; /* 8M bytes */  
  __base_HEAP = 0x80c00000  ; /* HEAP */  
  __base_RAM3 = 0x80c00000 ; /* RAM3 */  
  __top_HEAP = 0x80c00000 + 0x1400000 ; /* 20M bytes */  
  __top_RAM3 = 0x80c00000 + 0x1400000 ; /* 20M bytes */  
  __base_SRAM_ITC = 0x0  ; /* SRAM_ITC */  
  __base_RAM4 = 0x0 ; /* RAM4 */  
  __top_SRAM_ITC = 0x0 + 0x40000 ; /* 256K bytes */  
  __top_RAM4 = 0x0 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_DTC = 0x20000400  ; /* SRAM_DTC */  
  __base_RAM5 = 0x20000400 ; /* RAM5 */  
  __top_SRAM_DTC = 0x20000400 + 0x3fc00 ; /* 255K bytes */  
  __top_RAM5 = 0x20000400 + 0x3fc00 ; /* 255K bytes */  
  __base_VRAM = 0x20200000  ; /* VRAM */  
  __base_RAM6 = 0x20200000 ; /* RAM6 */  
  __top_VRAM = 0x20200000 + 0x40000 ; /* 256K bytes */  
  __top_RAM6 = 0x20200000 + 0x40000 ; /* 256K bytes */  
  __base_SRAM_OC2 = 0x20240000  ; /* SRAM_OC2 */  
  __base_RAM7 = 0x20240000 ; /* RAM7 */  
  __top_SRAM_OC2 = 0x20240000 + 0x20000 ; /* 128K bytes */  
  __top_RAM7 = 0x20240000 + 0x20000 ; /* 128K bytes */  
  __base_MAIN_STACK = 0x20260000  ; /* MAIN_STACK */  
  __base_RAM8 = 0x20260000 ; /* RAM8 */  
  __top_MAIN_STACK = 0x20260000 + 0x20000 ; /* 128K bytes */  
  __top_RAM8 = 0x20260000 + 0x20000 ; /* 128K bytes */  
