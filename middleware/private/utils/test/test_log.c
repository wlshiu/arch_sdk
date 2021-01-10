/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_log.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/09
 * @license
 * @description
 */

#include "_types.h"
#include "bsp.h"
#include "utils_printf.h"
#include "utils_log.h"
#include "unity_config.h"
#include "unity.h"

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

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test log", "[log system]", TEST_PRIORITY_NORMAL)
{
    {
        bsp_stdio_t     bsp_io = {0};
        bsp_get_std_io(&bsp_io);
        utils_register_stdout(bsp_io.pf_stdout_char, bsp_io.pf_stdout_string);
        utils_log_init();
    }

    info("log info\n");
    err("log error\n");
    info_color(LOG_YELLOW, "log info with yellow\n");

    return;
}