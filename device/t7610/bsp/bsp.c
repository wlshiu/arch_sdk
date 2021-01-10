/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/03
 * @license
 * @description
 */


#include "bsp.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern bsp_desc_t      g_bsp_fpga;
extern bsp_desc_t      g_bsp_evb;

#if defined(CONFIG_T7610_FPGA)
static bsp_desc_t       *g_pBSP_desc = &g_bsp_fpga;
#elif defined(CONFIG_T7610_EVB)
static bsp_desc_t       *g_pBSP_desc = &g_bsp_evb;
#else
    #error "unknown board !!!!"
#endif
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
hal_errno_t
bsp_init(cb_set_bsp_desc_t pf_set_desc)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        if( pf_set_desc )
        {
            rval = pf_set_desc(&g_pBSP_desc);
            if( rval ) break;
        }

        if( !g_pBSP_desc )
        {
            rval = -1;
            break;
        }

        if( g_pBSP_desc && g_pBSP_desc->pf_init )
            rval = g_pBSP_desc->pf_init();
    } while(0);

    return rval;
}

hal_errno_t
bsp_deinit(void)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    if( g_pBSP_desc && g_pBSP_desc->pf_deinit )
        rval = g_pBSP_desc->pf_deinit();

    return rval;
}

hal_errno_t
bsp_set_gpio(bsp_gpio_info_t *pInfo)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    if( g_pBSP_desc && g_pBSP_desc->pf_set_gpio )
        rval = g_pBSP_desc->pf_set_gpio(pInfo);

    return rval;
}

hal_errno_t
bsp_get_std_io(
    bsp_stdio_t     *pBSP_stdio)
{
    hal_errno_t     rval = HAL_ERRNO_NO_INSTANCE;

    if( g_pBSP_desc )
    {
        pBSP_stdio->pf_stdout_char   = g_pBSP_desc->pf_stdout_char;
        pBSP_stdio->pf_stdout_string = g_pBSP_desc->pf_stdout_string;
        pBSP_stdio->pf_stdin_str     = g_pBSP_desc->pf_stdin_str;
        pBSP_stdio->pf_stdio_flush   = g_pBSP_desc->pf_stdio_flush;
        rval = HAL_ERRNO_OK;
    }

    return rval;
}
