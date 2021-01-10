/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_shell.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/09
 * @license
 * @description
 */

#include "_types.h"
#include "bsp.h"
#include "utils__printf.h"
#include "utils__log.h"
#include "utils__string.h"
#include "unity_config.h"
#include "unity.h"

#include "shell.h"

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
static char        g_line_buf[256] = {0};
static char        g_history[1024] = {0};
static sh_args_t   g_sh_args = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_sh_io_init(sh_set_t *pSet_info)
{
    return 0;
}

static int
_sh_io_read(
    uint8_t     *pBuf,
    uint32_t    length,
    void        *pExtra)
{
    static cb_stdin_string_t    stdin_gets = 0;
    uint32_t                    byte_cnt = 0;

    if( !stdin_gets )
    {
        bsp_stdio_t     bsp_io = {0};
        bsp_get_std_io(&bsp_io);

        stdin_gets = bsp_io.pf_stdin_str;
    }

    byte_cnt = stdin_gets((char*)pBuf, length);

    return byte_cnt;
}

static int
_sh_io_write(
    uint8_t     *pBuf,
    uint32_t    length,
    void        *pExtra)
{
    static cb_stdout_string_t   stdout_puts = 0;
    int                         len  = 0;

    if( !stdout_puts )
    {
        bsp_stdio_t     bsp_io = {0};
        bsp_get_std_io(&bsp_io);

        stdout_puts = bsp_io.pf_stdout_string;
    }

    stdout_puts((char*)pBuf, length);

    return len;
}

static sh_io_desc_t    g_sh_std_io =
{
    .cb_init   = _sh_io_init,
    .cb_read   = _sh_io_read,
    .cb_write  = _sh_io_write,
};

static int
_sh_cmd_test(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    log_out("[%s:%d] test cmd\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
        log_out("argv[%d]:%s\n", i, argv[i]);

    return 0;
}

sh_cmd_add("test", "this is 'test' cmd", _sh_cmd_test, SH_CMD_REGISTER_PRIORITY_NORMAL);


static int
_sh_cmd_rd(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do {
        uint32_t    addr = 0, cnt = 0;
        uint32_t    *pCur = 0;

        if( argc != 3 )
        {
            log_out("wrong parameters\n");
            break;
        }

        addr = (*(argv[1] + 1) == 'x')
               ? strtox(argv[1] + 2, 0)
               : strtol(argv[1], 0);

        cnt  = (*(argv[2] + 1) == 'x')
               ? strtox(argv[2] + 2, 0)
               : strtol(argv[2], 0);

        pCur = (uint32_t*)addr;
        cnt  = (cnt + 0x3) >> 2;

        for(int i = 0; i < cnt; ++i)
        {
            if( !(i & 0x3) )
            {
                log_out("\n%08X |", addr);
                addr += 16;
            }

            log_out(" %08X", pCur[i]);
        }
        log_out("\n\n");
    } while(0);
    return 0;
}

static int
_sh_cmd_wr(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do {
        uint32_t    addr = 0;
        uint32_t    *pCur = 0;

        if( argc < 3 )
        {
            log_out("wrong parameters\n");
            break;
        }

        addr = (*(argv[1] + 1) == 'x')
               ? strtox(argv[1] + 2, 0)
               : strtol(argv[1], 0);

        pCur = (uint32_t*)addr;
        for(int i = 0; i < (argc - 2); ++i)
        {
            *pCur++ = (*(argv[i + 2] + 1) == 'x')
                      ? strtox(argv[i + 2] + 2, 0)
                      : strtol(argv[i + 2], 0);
        }
    } while(0);

    return 0;
}


static int
_sh_cmd_reboot(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do {
        uint32_t    remap_type = (uint32_t)-1;

        hal_scu_get_mmp_type(&remap_type);

        if( *(argv[1]) != '-' )
        {
            log_out("wrong parameters\n");
            break;
        }

        if( *(argv[1] + 1) == 'd' )
        {
            log_out("Memory Re-map type: %d\n", (int)remap_type);
            break;
        }
        else if( *(argv[1] + 1) == 'b' )
        {
            uint32_t    offset = 0;

            remap_type = strtol(argv[2], 0);
            log_out("switch remap to %d\n", remap_type);

            offset = (*(argv[3] + 1) == 'x')
                     ? strtox(argv[3] + 2, 0)
                     : strtol(argv[3], 0);
            if( offset & 0xF )
            {
                log_out("offset is not 16-alignment\n");
                break;
            }

            if( remap_type != 0 && remap_type != 2 )
            {
                log_out("Not support re-map type %d\n", remap_type);
                break;
            }

            hal_scu_set_hw_reset(SCU_RST_ATE_SPI_FLASH_HRST, false);

            {
                scu_sw_strap_t      sw_strap = {0};

                hal_scu_get_sw_strap(&sw_strap);

                // enter bootloader mode of PROM
                sw_strap.sw_strap_type = 0;
                hal_scu_set_sw_strap(&sw_strap);
            }

            for(int i = 0; i < 100; ++i)
                __asm("nop");

            hal_scu_set_mmp_type(remap_type);
            hal_scu_set_boot_offset(offset);
            hal_scu_reboot(true);
            break;
        }
    } while(0);

    return 0;
}


static sh_cmd_t     g_sh_cmd_rd =
{
    .pCmd_name      = "rd",
    .cmd_exec       = _sh_cmd_rd,
    .pDescription   = "read memory data\n"
                      "  usage: rd [address] [length]\n"
                      "    e.g. rd 0x30000000 32",
};

static sh_cmd_t     g_sh_cmd_wr =
{
    .pCmd_name      = "wr",
    .cmd_exec       = _sh_cmd_wr,
    .pDescription   = "write memory\n"
                      "  usage: wr [address] [data 1] ... [data 6]\n"
                      "    e.g. wr 0x30000000 0x123 0x456",
};

static sh_cmd_t     g_sh_cmd_reboot =
{
    .pCmd_name      = "reboot",
    .cmd_exec       = _sh_cmd_reboot,
    .pDescription   = "reboot with memory remap type\n"
                      "  usage: reboot [-d(display)/-b(boot)] [type] [boot bytes-offset (16-alignment)]\n"
                      "    [type]: 0= flash, 1= PROM, 2= M4-SRAM1\n"
                      "  e.g. reboot -d or reboot -b 1 0\n",
};
//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test console", "[vt100 console]"  LOG_YELLOW " (BLOCKING)"LOG_RESET, TEST_PRIORITY_LOW)
{
    {
        bsp_stdio_t     bsp_io = {0};
        bsp_get_std_io(&bsp_io);
        utils_register_stdout(bsp_io.pf_stdout_char, bsp_io.pf_stdout_string);
    }

    {
        sh_set_t    sh_set = {0};

        sh_set.pLine_buf    = g_line_buf;
        sh_set.line_buf_len = sizeof(g_line_buf);
        sh_set.pHistory_buf = g_history;
        sh_set.line_size    = sizeof(g_line_buf);
        sh_set.cmd_deep     = 3;
        sh_set.history_buf_size = SHELL_CALC_HISTORY_BUFFER(sh_set.line_size, sh_set.cmd_deep);
        shell_init(&g_sh_std_io, &sh_set);

        shell_register_cmd(&g_sh_cmd_rd);
        shell_register_cmd(&g_sh_cmd_wr);
        shell_register_cmd(&g_sh_cmd_reboot);

        g_sh_args.is_blocking = 1;

        shell_proc(&g_sh_args);
    }

    return;
}
