/*
 * GENERATED FILE - DO NOT EDIT
 * Copyright (c) 2008 - 2013 Code Red Technologies Ltd, 
 * Copyright 2015, 2018 NXP
 * (c) NXP Semiconductors 2013-2019
 * Generated linker script file for MIMXRT1062xxxxA
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.3.0 [Build 2200] [2018-12-03] on 2019/04/13 19:55:37
 */

INCLUDE "IMXRT1060-EVK_APL_Debug_library.ld"
INCLUDE "IMXRT1060-EVK_APL_Debug_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        LONG(LOADADDR(.data_RAM3));
        LONG(    ADDR(.data_RAM3));
        LONG(  SIZEOF(.data_RAM3));
        LONG(LOADADDR(.data_RAM4));
        LONG(    ADDR(.data_RAM4));
        LONG(  SIZEOF(.data_RAM4));
        LONG(LOADADDR(.data_RAM5));
        LONG(    ADDR(.data_RAM5));
        LONG(  SIZEOF(.data_RAM5));
        LONG(LOADADDR(.data_RAM6));
        LONG(    ADDR(.data_RAM6));
        LONG(  SIZEOF(.data_RAM6));
        LONG(LOADADDR(.data_RAM7));
        LONG(    ADDR(.data_RAM7));
        LONG(  SIZEOF(.data_RAM7));
        LONG(LOADADDR(.data_RAM8));
        LONG(    ADDR(.data_RAM8));
        LONG(  SIZEOF(.data_RAM8));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        LONG(    ADDR(.bss_RAM3));
        LONG(  SIZEOF(.bss_RAM3));
        LONG(    ADDR(.bss_RAM4));
        LONG(  SIZEOF(.bss_RAM4));
        LONG(    ADDR(.bss_RAM5));
        LONG(  SIZEOF(.bss_RAM5));
        LONG(    ADDR(.bss_RAM6));
        LONG(  SIZEOF(.bss_RAM6));
        LONG(    ADDR(.bss_RAM7));
        LONG(  SIZEOF(.bss_RAM7));
        LONG(    ADDR(.bss_RAM8));
        LONG(  SIZEOF(.bss_RAM8));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

        *(.after_vectors*)

    } > Prgram

    .text : ALIGN(4)
    {
       *(.text*)
       *(.rodata .rodata.* .constdata .constdata.*)
       . = ALIGN(4);
    } > Prgram
    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4) 
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > Prgram

    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > Prgram
    __exidx_end = .;
 
    _etext = .;
        

    /* Reserve and place Heap within memory map */
    _HeapSize = 0x0;
    .heap :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += _HeapSize;
        . = ALIGN(4); 
        _pvHeapLimit = .;
    } > SRAM_OC2
    /* DATA section for NonCache */

    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$NonCache)
       *(.NonCacheable.init)
        *(.data.$RAM2*)
        *(.data.$NonCache*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
     } > NonCache AT>Prgram
    /* DATA section for HEAP */

    .data_RAM3 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM3 = .) ;
        *(.ramfunc.$RAM3)
        *(.ramfunc.$HEAP)
        *(.data.$RAM3*)
        *(.data.$HEAP*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM3 = .) ;
     } > HEAP AT>Prgram
    /* DATA section for SRAM_ITC */

    .data_RAM4 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM4 = .) ;
        *(.ramfunc.$RAM4)
        *(.ramfunc.$SRAM_ITC)
        *(.data.$RAM4*)
        *(.data.$SRAM_ITC*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM4 = .) ;
     } > SRAM_ITC AT>Prgram
    /* DATA section for SRAM_DTC */

    .data_RAM5 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM5 = .) ;
        *(.ramfunc.$RAM5)
        *(.ramfunc.$SRAM_DTC)
        *(.data.$RAM5*)
        *(.data.$SRAM_DTC*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM5 = .) ;
     } > SRAM_DTC AT>Prgram
    /* DATA section for VRAM */

    .data_RAM6 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM6 = .) ;
        *(.ramfunc.$RAM6)
        *(.ramfunc.$VRAM)
        *(.data.$RAM6*)
        *(.data.$VRAM*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM6 = .) ;
     } > VRAM AT>Prgram
    /* DATA section for SRAM_OC2 */

    .data_RAM7 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM7 = .) ;
        *(.ramfunc.$RAM7)
        *(.ramfunc.$SRAM_OC2)
        *(.data.$RAM7*)
        *(.data.$SRAM_OC2*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM7 = .) ;
     } > SRAM_OC2 AT>Prgram
    /* DATA section for MAIN_STACK */

    .data_RAM8 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM8 = .) ;
        *(.ramfunc.$RAM8)
        *(.ramfunc.$MAIN_STACK)
        *(.data.$RAM8*)
        *(.data.$MAIN_STACK*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM8 = .) ;
     } > MAIN_STACK AT>Prgram
    /* MAIN DATA SECTION */
    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > Prgram

    /* Main DATA section (Prgram) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > Prgram AT>Prgram

    /* BSS section for NonCache */
    .bss_RAM2 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       *(.bss.$RAM2*)
       *(.bss.$NonCache*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
    } > NonCache 

    /* BSS section for HEAP */
    .bss_RAM3 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM3 = .) ;
       *(.bss.$RAM3*)
       *(.bss.$HEAP*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM3 = .) ;
    } > HEAP 

    /* BSS section for SRAM_ITC */
    .bss_RAM4 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM4 = .) ;
       *(.bss.$RAM4*)
       *(.bss.$SRAM_ITC*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM4 = .) ;
    } > SRAM_ITC 

    /* BSS section for SRAM_DTC */
    .bss_RAM5 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM5 = .) ;
       *(.bss.$RAM5*)
       *(.bss.$SRAM_DTC*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM5 = .) ;
    } > SRAM_DTC 

    /* BSS section for VRAM */
    .bss_RAM6 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM6 = .) ;
       *(.bss.$RAM6*)
       *(.bss.$VRAM*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM6 = .) ;
    } > VRAM 

    /* BSS section for SRAM_OC2 */
    .bss_RAM7 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM7 = .) ;
       *(.bss.$RAM7*)
       *(.bss.$SRAM_OC2*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM7 = .) ;
    } > SRAM_OC2 

    /* BSS section for MAIN_STACK */
    .bss_RAM8 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM8 = .) ;
       *(.bss.$RAM8*)
       *(.bss.$MAIN_STACK*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM8 = .) ;
    } > MAIN_STACK 

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > Prgram

    /* NOINIT section for NonCache */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       *(.NonCacheable)
       *(.noinit.$RAM2*)
       *(.noinit.$NonCache*)
       . = ALIGN(4) ;
    } > NonCache 

    /* NOINIT section for HEAP */
    .noinit_RAM3 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM3*)
       *(.noinit.$HEAP*)
       . = ALIGN(4) ;
    } > HEAP 

    /* NOINIT section for SRAM_ITC */
    .noinit_RAM4 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM4*)
       *(.noinit.$SRAM_ITC*)
       . = ALIGN(4) ;
    } > SRAM_ITC 

    /* NOINIT section for SRAM_DTC */
    .noinit_RAM5 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM5*)
       *(.noinit.$SRAM_DTC*)
       . = ALIGN(4) ;
    } > SRAM_DTC 

    /* NOINIT section for VRAM */
    .noinit_RAM6 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM6*)
       *(.noinit.$VRAM*)
       . = ALIGN(4) ;
    } > VRAM 

    /* NOINIT section for SRAM_OC2 */
    .noinit_RAM7 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM7*)
       *(.noinit.$SRAM_OC2*)
       . = ALIGN(4) ;
    } > SRAM_OC2 

    /* NOINIT section for MAIN_STACK */
    .noinit_RAM8 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM8*)
       *(.noinit.$MAIN_STACK*)
       . = ALIGN(4) ;
    } > MAIN_STACK 

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > Prgram

     _StackSize = 0x4000;
     /* Reserve space in memory for Stack */
    .heap2stackfill  :
    {
        . += _StackSize;
    } > MAIN_STACK
    /* Locate actual Stack in memory map */
    .stack ORIGIN(MAIN_STACK) + LENGTH(MAIN_STACK) - _StackSize - 0:  ALIGN(4)
    {
        _vStackBase = .;
        . = ALIGN(4);
        _vStackTop = . + _StackSize;
    } > MAIN_STACK

    /* Provide basic symbols giving location and size of main text
     * block, including initial values of RW data sections. Note that
     * these will need extending to give a complete picture with
     * complex images (e.g multiple Flash banks).
     */
    _image_start = LOADADDR(.text);
    _image_end = LOADADDR(.data) + SIZEOF(.data);
    _image_size = _image_end - _image_start;
}