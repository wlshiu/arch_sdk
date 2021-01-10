/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file utils.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/02
 * @license
 * @description
 */

#ifndef __utils_H_w8DPG1Gy_liGX_H8Ru_sv7X_uAObZ0b9bP8Q__
#define __utils_H_w8DPG1Gy_liGX_H8Ru_sv7X_uAObZ0b9bP8Q__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define DEFINE_BFIELD(bit_size)                         \
    typedef struct bfield_ {                            \
        unsigned short bits_field[((bit_size) + 0xF) >> 4]; \
    } bfield_t;

#define BFIELD_SET(pBSet, bit_order)        ((pBSet)->bits_field[(bit_order) >> 4] |=  (1 << ((bit_order) & 0xF)))
#define BFIELD_CLR(pBSet, bit_order)        ((pBSet)->bits_field[(bit_order) >> 4] &= ~(1 << ((bit_order) & 0xF)))
#define BFIELD_IS_SET(pBSet, bit_order)     ((pBSet)->bits_field[(bit_order) >> 4] &   (1 << ((bit_order) & 0xF)))
#define BFIELD_ZERO(pBSet)                  memset((void*)(pBSet), 0x0, sizeof(bfield_t))

#define bit_field_t                         bfield_t

#define _stringize(s)                       #s
#define utils_to_string(a)                  _stringize(a)

__attribute__((always_inline)) static inline uint32_t __get_lr(void)
{
    register uint32_t result;
    __asm volatile ("MOV %0, LR\n" : "=r" (result) );
    return (result);
}
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif
