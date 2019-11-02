/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/23
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
static bsp_desc_t       *g_pBSP_desc = 0;

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
bsp_init(cb_set_bsp_desc_t pf_set_desc)
{
    int     rval = 0;
    do {
        if( pf_set_desc )
        {
            rval = pf_set_desc(&g_pBSP_desc);
            if( rval )  break;
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

int
bsp_deinit(void)
{
    int     rval = 0;

    if( g_pBSP_desc && g_pBSP_desc->pf_deinit )
        rval = g_pBSP_desc->pf_deinit();

    return rval;
}

int
bsp_set_gpio(void *pExtra)
{
    int     rval = 0;

    if( g_pBSP_desc && g_pBSP_desc->pf_set_gpio )
        rval = g_pBSP_desc->pf_set_gpio(pExtra);

    return rval;
}

int
bsp_get_std_io(
    bsp_stdio_t     *pBSP_stdio)
{
    int     rval = -1;

    if( g_pBSP_desc )
    {
        pBSP_stdio->pf_stdout_char   = g_pBSP_desc->pf_stdout_char;
        pBSP_stdio->pf_stdout_string = g_pBSP_desc->pf_stdout_string;
        pBSP_stdio->pf_stdin_str     = g_pBSP_desc->pf_stdin_str;
        rval = 0;
    }

    return rval;
}
