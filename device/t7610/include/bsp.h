/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/03
 * @license
 * @description
 */

#ifndef __bsp_H_wna6Yqry_lUt4_HAWi_ss9X_upstLx31cIin__
#define __bsp_H_wna6Yqry_lUt4_HAWi_ss9X_upstLx31cIin__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
struct bsp_desc;
typedef hal_errno_t (*cb_set_bsp_desc_t)(struct bsp_desc **ppDesc);

typedef hal_errno_t (*cb_stdout_char_t)(int c);
typedef int         (*cb_stdout_string_t)(char *pBuf, int length);
typedef int         (*cb_stdin_string_t)(char *pBuf, int length);
typedef hal_errno_t (*cb_stdio_flush_t)(void);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct bsp_stdio
{
    cb_stdout_char_t      pf_stdout_char;
    cb_stdout_string_t    pf_stdout_string;
    cb_stdin_string_t     pf_stdin_str;
    cb_stdio_flush_t      pf_stdio_flush;
} bsp_stdio_t;

/**
 *  set GPIO of bsp
 */
typedef struct bsp_gpio_info
{
    void            *pHGpio;        /*!< the handler of a GPIO port */
    uint32_t        pin;            /*!< the pin id */
    uint8_t         mode;           /*!< direction mode, reference enum gpio_mode */
    uint8_t         trig_mode;      /*!< trigger mode, reference enum gpio_trig */
    uint8_t         data;           /*!< data type, reference enum gpio_data */
} bsp_gpio_info_t;

/**
 *  the description of BSP
 */
typedef struct bsp_desc
{
    hal_errno_t (*pf_init)(void);
    hal_errno_t (*pf_deinit)(void);

    hal_errno_t (*pf_set_gpio)(bsp_gpio_info_t *pInfo);

    // standard I/O methods in the BSP
    cb_stdout_char_t      pf_stdout_char;
    cb_stdout_string_t    pf_stdout_string;
    cb_stdin_string_t     pf_stdin_str;
    cb_stdio_flush_t      pf_stdio_flush;

    // private data
    void                  *pPriv_data;
} bsp_desc_t;
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
 *  @brief  initialize board (Non thread-safe)
 *
 *  @param [in] pf_set_desc     assign the user description of BSP if pf_set_desc is not zero
 *  @return
 *      0: ok, other: fail
 */
hal_errno_t
bsp_init(cb_set_bsp_desc_t pf_set_desc);

hal_errno_t
bsp_deinit(void);


/**
 *  @brief  bsp_set_gpio (Non thread-safe)
 *
 *  @param [in] pInfo           setting info, reference struct bsp_gpio_info
 *  @return
 *      0: ok, other: fail
 */
hal_errno_t
bsp_set_gpio(bsp_gpio_info_t *pInfo);


/**
 *  @brief  Get the standard I/O of BSP
 *
 *  @param [in] pBSP_stdio      report the functions of standard I/O
 *  @return
 *      0: ok, other: fail
 */
hal_errno_t
bsp_get_std_io(
    bsp_stdio_t     *pBSP_stdio);


#ifdef __cplusplus
}
#endif

#endif
