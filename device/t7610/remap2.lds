/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file remap2.lds.S
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/21
 * @license
 * @description
 */

/* H/w configuration */
#define IRAM_M40_MEM_BASE   0x20000000
#define IRAM_M40_MEM_SIZE   (256 << 10)
#define IRAM_M40_MEM_END    0x20040000

#define RELOC_AREA_SIZE     (32 << 10)

MEMORY
{
    /* LMA regions */
    LMA_CORE  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x20000
    LMA_DATA  (r)   : ORIGIN = 0x00020000, LENGTH = 0x8000

    /* VMA regions */
    VMA_CORE  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x20000

    VMA_DATA0 (rw)  : ORIGIN = IRAM_M40_MEM_BASE, LENGTH = 0x8000
    VMA_STACK (rw)  : ORIGIN = IRAM_M40_MEM_END - CONFIG_SYSTEM_STACK_SIZE, LENGTH = CONFIG_SYSTEM_STACK_SIZE
    VMA_RELOC (rw)  : ORIGIN = IRAM_M40_MEM_BASE + 0x8000, LENGTH = RELOC_AREA_SIZE
    VMA_DATA1 (rw)  : ORIGIN = IRAM_M40_MEM_BASE + 0x8000 + RELOC_AREA_SIZE, LENGTH = 0x20000
}

