/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_wdt.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_wdt_H_wNiVRFf6_leLQ_HNTM_sK4Q_u91A5CFVeiqS__
#define __hal_wdt_H_wNiVRFf6_leLQ_HNTM_sK4Q_u91A5CFVeiqS__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum wdt_err
{
    WDT_ERR_OK             = 0,
    WDT_ERR_NULL_POINTER   = (WDT_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    WDT_ERR_NO_INSTANCE    = (WDT_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    WDT_ERR_WRONG_PARAM    = (WDT_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    WDT_ERR_NOT_SUPPORT    = (WDT_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),

} wdt_err_t;

typedef enum wdt_mode
{
    WDT_MODE_IRQ    = 1,
    WDT_MODE_RESET,
} wdt_mode_t;
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
/**
 *  @brief      hal_wdt_init (Non thread-safe)
 *
 *  @param [in] mode    WDT mode, reference enum wdt_mode
 *
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_init(
    wdt_mode_t  mode);


/**
 *  @brief  hal_wdt_deinit (Non thread-safe)
 *
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_deinit(void);


/**
 *  @brief  hal_wdt_enable (Non thread-safe)
 *              enable WDT
 *
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_enable(void);


/**
 *  @brief  hal_wdt_disable (Non thread-safe)
 *              disable WDT
 *
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_disable(void);


/**
 *  @brief  hal_wdt_set_timeout (Non thread-safe)
 *
 *  @param [in] timeout     timeout ticks base on PCLK
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_set_timeout(
    uint32_t    timeout);


/**
 *  @brief  hal_wdt_clear_irq (Non thread-safe)
 *              clear IRQ flag
 *
 *  @return
 *      error code, reference enum wdt_err
 */
wdt_err_t
hal_wdt_clear_irq(void) __isr_func_n;


#ifdef __cplusplus
}
#endif

#endif
