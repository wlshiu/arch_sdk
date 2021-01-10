/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file common_basic.lds.S
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/21
 * @license
 * @description
 */


#include "autoconfig.h"

#if (CONFIG_TARGET_REMAP_TYPE == 0)
#include "remap0.lds"
#elif (CONFIG_TARGET_REMAP_TYPE == 2)
#include "remap2.lds"
#else
#error "Unknown memory type !!!"
#endif


ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))

        __text_start__ = .;
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > VMA_CORE AT>LMA_CORE

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > VMA_CORE AT>LMA_CORE

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > VMA_CORE AT>LMA_CORE
    __exidx_end = .;

    .test_pool :
    {
        __test_pool_start__ = ALIGN(4);
        KEEP(*(test_pool*))
        __test_pool_end__ = ALIGN(4);
    } > VMA_CORE AT>LMA_CORE

    .cli_cmd_pool :
    {
        __cli_cmd_pool_start__ = ALIGN(4);
        KEEP(*(cli_cmd_pool*))
        __cli_cmd_pool_end__ = ALIGN(4);
    } > VMA_CORE AT>LMA_CORE

    __reloc_lma_start__ = ALIGN(4);
    .reloc :
    {
        __reloc_vma_start__ = ALIGN(4);
        KEEP(*(.vpool))
        KEEP(*(reloc_text*))
        KEEP(*(reloc_data*))
        KEEP(*(isr_func*))
        __reloc_vma_end__ = ALIGN(4);
    } > VMA_RELOC  AT> LMA_CORE

    __text_end__ = __reloc_lma_start__ + SIZEOF(.reloc);

    .data :
    {
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

    } > VMA_DATA0 AT> LMA_CORE

    .os_heap :
    {
        . = ALIGN(4);
        __os_heap_start__ = .;
        __g3lib_heap_start__ = .;
        KEEP(*(.bss.sys_ucHeap))
        __g3lib_heap_end__ = .;
        KEEP(*(.bss.ucHeap))
        . = ALIGN(4);
        __os_heap_end__ = .;
    } > VMA_DATA1

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        __g3lib_bss_start__ = .;
        *libg3.a:*.o (.bss*)
        . = ALIGN(4);
        __g3lib_bss_end__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > VMA_DATA0


    .heap (COPY):
    {
        __end__ = .;
        __HeapBase = .;

        PROVIDE(end = .);
        KEEP(*(.heap*))
        __HeapLimit = .;
    } > VMA_DATA0

    .stack_dummy (COPY):
    {
        KEEP(*(.stack*))
    } > VMA_STACK

    __StackTop = ORIGIN(VMA_STACK) + LENGTH(VMA_STACK);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "System Stack and System Heap overlap")

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }
}
