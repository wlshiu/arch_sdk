/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file remap0.lds.S
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

#define IRAM_M41_MEM_BASE   0x30000000
#define IRAM_M41_MEM_SIZE   (256 << 10)
#define IRAM_M41_MEM_END    0x30040000

#define RELOC_AREA_SIZE     (32 << 10)

MEMORY
{
    /* LMA regions */
    LMA_CORE  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x80000
    LMA_DATA  (r)   : ORIGIN = 0x00080000, LENGTH = 0x8000

    /* VMA regions */
    VMA_CORE  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x80000

    VMA_DATA0 (rw)  : ORIGIN = IRAM_M40_MEM_BASE, LENGTH = IRAM_M40_MEM_SIZE - CONFIG_SYSTEM_STACK_SIZE
    VMA_STACK (rw)  : ORIGIN = IRAM_M40_MEM_END - CONFIG_SYSTEM_STACK_SIZE, LENGTH = CONFIG_SYSTEM_STACK_SIZE
    VMA_RELOC (rw)  : ORIGIN = IRAM_M41_MEM_BASE, LENGTH = RELOC_AREA_SIZE
    VMA_DATA1 (rw)  : ORIGIN = IRAM_M41_MEM_BASE + RELOC_AREA_SIZE, LENGTH = IRAM_M41_MEM_SIZE - RELOC_AREA_SIZE
}

