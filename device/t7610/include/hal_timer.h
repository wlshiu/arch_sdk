/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_timer.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_timer_H_wsYSgkSj_lEPm_HM79_sXf4_uBQOiJvkYtvR__
#define __hal_timer_H_wsYSgkSj_lEPm_HM79_sXf4_uBQOiJvkYtvR__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum tmr_err
{
    TMR_ERR_OK             = 0,
    TMR_ERR_NULL_POINTER   = (TMR_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    TMR_ERR_NO_INSTANCE    = (TMR_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    TMR_ERR_WRONG_PARAM    = (TMR_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    TMR_ERR_NOT_SUPPORT    = (TMR_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),

} tmr_err_t;

typedef enum tmr_mode
{
    TMR_MODE_REPEAT = 1,    /*!< periodic trigger */
    TMR_MODE_ALARM,         /*!< single trigger */
} tmr_mode_t;

typedef enum tmr_clk
{
    TMR_CLK_PCLK        = 0,
    TMR_CLK_EXTERNAL,
} tmr_clk_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define TMR_TIMEOUT_MSEC(msec)          ((msec) * (hal_sys_get_pclk() / 1000ul))
#define TMR_TIMEOUT_USEC(usec)          ((usec) * (hal_sys_get_pclk() / 1000000ul))
//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  timer initial configuration
 */
typedef struct tmr_config
{
    tmr_clk_t      clk_source;  /*!< the clock source */


} tmr_config_t;

/**
 *  timer handle
 */
typedef struct tmr_handle
{
    uint32_t    instance;   /*!< the instance of target module */
    uint32_t    port_id;    /*!< the port id of target module */
    IRQn_Type   irq_id;     /*!< the IRQ id of target module */

} tmr_handle_t;
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
 *  @brief  hal_tmr_init (Non thread-safe)
 *
 *  @param [in] target_id       target timer id (from 0, ...)
 *  @param [in] pConfig         the configuration of timer module
 *  @param [in] ppHTmr          the uart handler
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_init(
    uint32_t        target_id,
    tmr_config_t    *pConfig,
    tmr_handle_t    **ppHTmr);


/**
 *  @brief  hal_tmr_deinit (Non thread-safe)
 *
 *  @param [in] ppHTmr          the timer handler
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_deinit(
    tmr_handle_t    **ppHTmr);


/**
 *  @brief  hal_tmr_enable (Non thread-safe)
 *
 *  @param [in] pHTmr           the timer handler
 *  @param [in] mode            the timer mode, reference enum tmr_mode
 *  @param [in] timeout         the timerout value
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_enable(
    tmr_handle_t    *pHTmr,
    tmr_mode_t      mode,
    uint32_t        timeout);


/**
 *  @brief  hal_tmr_disable (Non thread-safe)
 *
 *  @param [in] pHTmr           the timer handler
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_disable(
    tmr_handle_t    *pHTmr);


/**
 *  @brief  hal_tmr_get_count (Non thread-safe)
 *
 *  @param [in] pHTmr           the timer handler
 *  @param [in] pCount          the curent count value of timer
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_get_count(
    tmr_handle_t    *pHTmr,
    uint32_t        *pCount);


/**
 *  @brief  hal_tmr_clear_irq (Non thread-safe)
 *              clear IRQ flag
 *
 *  @param [in] pHTmr           the timer handler
 *  @return
 *      error code, reference enum tmr_err
 */
tmr_err_t
hal_tmr_clear_irq(
    tmr_handle_t    *pHTmr) __isr_func_n;


#ifdef __cplusplus
}
#endif

#endif
