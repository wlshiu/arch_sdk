/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_errno.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_errno_H_wlYsgY7b_lRWV_Hkcz_sBRA_u9vDDFa1Dedi__
#define __hal_errno_H_wlYsgY7b_lRWV_Hkcz_sBRA_u9vDDFa1Dedi__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum hal_errno
{
    HAL_ERRNO_OK    = 0,
    HAL_ERRNO_NULL_POINTER,
    HAL_ERRNO_NO_INSTANCE,
    HAL_ERRNO_WRONG_PARAM,
    HAL_ERRNO_NOT_SUPPORT,
    HAL_ERRNO_UNKNOWN_DEVICE,

    HAL_ERRNO_UNKNOWN,
} hal_errno_t;

#define UART_ERRNO_BASE         0xED100000ul
#define WDT_ERRNO_BASE          0xED200000ul
#define TMR_ERRNO_BASE          0xED300000ul
#define I2C_ERRNO_BASE          0xED400000ul
#define GPIO_ERRNO_BASE         0xED500000ul
#define SSP_ERRNO_BASE          0xED600000ul
#define FC_ERRNO_BASE           0xED700000ul
#define DMA_ERRNO_BASE          0xED800000ul
#define CRC_ERRNO_BASE          0xED900000ul
#define ETH_ERRNO_BASE          0xEDA00000ul
#define SCU_ERRNO_BASE          0xEDB00000ul
#define IPC_ERRNO_BASE          0xEDC00000ul
#define IMG_ERRNO_BASE          0xEDD00000ul
//=============================================================================
//                  Macro Definition
//=============================================================================

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
