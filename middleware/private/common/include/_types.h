/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file _types.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/02
 * @license
 * @description
 */

#ifndef ___types_H_wn03fgZU_lZYW_HWgl_ssIK_uJT8BGcwfncl__
#define ___types_H_wn03fgZU_lZYW_HWgl_ssIK_uJT8BGcwfncl__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <_errno.h>

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#ifndef __unused
#define __unused                    __attribute__ ((unused))
#endif

#ifndef __used
#define __used                      __attribute__ ((used))
#endif

#ifndef __align
#define __align(num)                __attribute__ ((aligned(num)))
#endif

#ifndef __section
#define __section(x)                __attribute__ ((section(x)))
#endif

#ifndef __weak
#define __weak                      __attribute__ ((weak))
#endif

#ifndef __constructor
#define __constructor(priority)     __attribute__((constructor(priority)))
#endif

#ifndef NULL
#define NULL    0
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#ifndef __pause
#define __pause()                   do { __asm volatile("BKPT #01"); } while(0)
#endif

#ifndef align_4
#define align_4(x)                  (((uintptr_t)(x) + 0x3) & ~0x3)
#endif

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
