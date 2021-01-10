/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file utils_log.c
/** @file utils_log.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/02
 * @license
 * @description
 */

#include <stdarg.h>
#include <string.h>
#include "utils.h"
#include "utils_log.h"
#include "utils_printf.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_LOG_BUFFER_SIZE      256
//=============================================================================
//                  Macro Definition
//=============================================================================
#define LOG_TYPE_STATE(a)           utils_log(UTILS_LOG_INFO, "%s= %d\n", utils_to_string(a), BFIELD_IS_SET(&g_log_type_set, a))

//=============================================================================
//                  Structure Definition
//=============================================================================
DEFINE_BFIELD(UTILS_LOG_MAX);
//=============================================================================
//                  Global Data Definition
//=============================================================================
static bit_field_t      g_log_type_set;
static cb_log_rec_t     g_cb_log_recorder = 0;

#if defined(CONFIG_ENABLE_LOG_TEE)
static char             g_log_buf[CONFIG_LOG_BUFFER_SIZE] = {0};
#endif
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
utils_log_init(void)
{
    BFIELD_ZERO(&g_log_type_set);
    BFIELD_SET(&g_log_type_set, UTILS_LOG_ERR);
    BFIELD_SET(&g_log_type_set, UTILS_LOG_INFO);
    return;
}

void
utils_log_register_recorder(
    cb_log_rec_t    cb_recorder)
{
    g_cb_log_recorder = cb_recorder;
    return;
}

void
utils_log(utils_log_type_t type, const char *fmt, ...)
{
    if( BFIELD_IS_SET(&g_log_type_set, type) )
    {
        va_list     va;
        va_start(va, fmt);

    #if defined(CONFIG_ENABLE_LOG_TEE)
        {
            memset(g_log_buf, 0x0, CONFIG_LOG_BUFFER_SIZE);
            utils_vsprintf(g_log_buf, sizeof(g_log_buf), fmt, va);

            utils_puts(g_log_buf);
            if( g_cb_log_recorder )
                g_cb_log_recorder(type, g_log_buf, strlen(g_log_buf));
        }
    #else
        utils_vsprintf(0, 0, fmt, va);
    #endif

        va_end(va);
    }
    return;
}

void
utils_log_on(utils_log_type_t type)
{
    BFIELD_SET(&g_log_type_set, type);
    return;
}

void
utils_log_off(utils_log_type_t type)
{
    BFIELD_CLR(&g_log_type_set, type);
    return;
}

void
utils_log_status(void)
{
    LOG_TYPE_STATE(UTILS_LOG_ERR);
    LOG_TYPE_STATE(UTILS_LOG_INFO);

    return;
}

void
utils_log_mem(
    utils_log_type_t   type,
    char                *prefix,
    uint8_t             *pAddr,
    int                 bytes,
    unsigned int        has_out_u32le)
{
    if( has_out_u32le )
    {
        uintptr_t   addr = (uintptr_t)pAddr;
        uint32_t    cnt = (bytes + 0x3) >> 2;
        uint32_t    *pCur = (uint32_t*)pAddr;

        for(int i = 0; i < cnt; i++)
        {
            if( (i & 0x3) == 2 )
                utils_log(type, " -");
            else if( !(i & 0x3) )
            {
                utils_log(type, "\n%s%08X |", prefix, addr);
                addr += 16;
            }

            utils_log(type, " %08X", pCur[i]);
        }
        utils_log(type, "\n\n");
    }
    else
    {
        uintptr_t   addr = (uintptr_t)pAddr;
        uint8_t     *pCur = pAddr;

        for(int i = 0; i < bytes; i++)
        {
            if( (i & 0xF) == 8 )
                utils_log(type, " -");
            else if( !(i & 0xF) )
            {
                utils_log(type, "\n%s%08X |", prefix, addr);
                addr += 16;
            }

            utils_log(type, " %02X", pCur[i]);
        }
        utils_log(type, "\n\n");
    }
    return;
}
