/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sdram_init.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/24
 * @license
 * @description
 */


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define REG_SDRAM_CTRL_BASE                 0x5A000000ul
#define MAX_SDRAM_CLK_DELAY                 0x1F
#define CONFIG_SDRAM_TEST_PATTERN           0xA5A55A5Aul
#define CONFIG_SDRAM_TEST_LENGTH            (0x8ul << 10)
//=============================================================================
//                  Macro Definition
//=============================================================================
#ifndef __section
#define __section(x)                __attribute__ ((section(x)))
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sdram_reg_ctrl
{
    union {
        volatile uint32_t        CRTP1;
        struct {
            volatile uint32_t   TCL  : 2;
            volatile uint32_t        : 2;
            volatile uint32_t   TWR  : 2;
            volatile uint32_t        : 2;
            volatile uint32_t   TRF  : 4;
            volatile uint32_t   TRCD : 3;
            volatile uint32_t        : 1;
            volatile uint32_t   TRP  : 4;
            volatile uint32_t   TRAS : 4;
            volatile uint32_t        : 8;
        } CRTP1b;
    };

    union {
        volatile uint32_t        CRTP2;
        struct {
            volatile uint32_t   REF_INTV : 16;
            volatile uint32_t   INI_REFT : 4;
            volatile uint32_t   INI_PREC : 4;
            volatile uint32_t            : 8;
        } CRTP2b;
    };

    union {
        volatile uint32_t        CRCFG1;
        struct {
            volatile uint32_t   BNKSIZE : 4;
            volatile uint32_t   MBW     : 2;
            volatile uint32_t           : 2;
            volatile uint32_t   DSZ     : 3;
            volatile uint32_t           : 1;
            volatile uint32_t   DDW     : 2;
            volatile uint32_t           : 18;
        } CRCFG1b;
    };

    union {
        volatile uint32_t        CRCFG2;
        struct {
            volatile uint32_t   SREF      : 1;
            volatile uint32_t   PWDN      : 1;
            volatile uint32_t   ISMR      : 1;
            volatile uint32_t   IREF      : 1;
            volatile uint32_t   IPREC     : 1;
            volatile uint32_t   AMTSEL    : 1;
            volatile uint32_t             : 2;
            volatile uint32_t   T_PDSR    : 10;
            volatile uint32_t             : 2;
            volatile uint32_t   SREF_MODE : 1;
            volatile uint32_t             : 11;
        } CRCFG2b;
    };
} sdram_reg_ctrl_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int sdram_init(void) __attribute__ ((used));
int sdram_init(void)
{
    int     rval = 0;

    do {
        sdram_reg_ctrl_t    *pReg = (sdram_reg_ctrl_t*)REG_SDRAM_CTRL_BASE;

        pReg->CRTP1 = 0x00A33923;
        pReg->CRCFG1 = 0x00002111;
        pReg->CRCFG2 = 0x18;

        // wait pre-charge and refresh
        while( pReg->CRCFG2 ) {}

        pReg->CRCFG2 = 0x04;

        // wait set_load_register
        while( pReg->CRCFG2 ) {}
    } while(0);
    return rval;
}
