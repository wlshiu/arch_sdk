/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/2
 * @license
 * @description
 */

#ifndef __bsp_H_wRYhvKYP_loVI_H0qt_sJQT_ubEM0JLpihYM__
#define __bsp_H_wRYhvKYP_loVI_H0qt_sJQT_ubEM0JLpihYM__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
//                  Constant Definition
//=============================================================================
struct bsp_desc;
typedef int (*cb_set_bsp_desc_t)(struct bsp_desc **ppDesc);

typedef int (*cb_stdout_char_t)(int c);
typedef int (*cb_stdout_string_t)(char *pBuf, int length);
typedef int (*cb_stdin_string_t)(char *pBuf, int length);
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
} bsp_stdio_t;

/**
 *  the description of BSP
 */
typedef struct bsp_desc
{
    int     (*pf_init)(void);
    int     (*pf_deinit)(void);

    int     (*pf_set_gpio)(void *pExtra);

    // standard I/O methods in the BSP
    cb_stdout_char_t      pf_stdout_char;
    cb_stdout_string_t    pf_stdout_string;
    cb_stdin_string_t     pf_stdin_str;

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
 *  @brief  initialize board
 *
 *  @param [in] pf_set_desc     assign the user description of BSP if pf_set_desc is not zero
 *  @return
 *      0: ok, other: fail
 */
int
bsp_init(cb_set_bsp_desc_t pf_set_desc);

int
bsp_deinit(void);

int
bsp_set_gpio(void *pExtra);


/**
 *  @brief  Get the standard I/O of BSP
 *
 *  @param [in] pBSP_stdio      report the functions of standard I/O
 *  @return
 *      0: ok, other: fail
 */
int
bsp_get_std_io(
    bsp_stdio_t     *pBSP_stdio);


#ifdef __cplusplus
}
#endif

#endif
