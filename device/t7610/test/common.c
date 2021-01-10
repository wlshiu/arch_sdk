/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file common.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/26
 * @license
 * @description
 */


#include <string.h>
#include <stdarg.h>
#include "common.h"

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
static cb_stdout_string_t       g_logout = 0;

char    g_msg[128] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_uint2strhex(char *pStr, unsigned int number, const char nibbles_to_print)
{
#define MAX_NIBBLES     (8)
    int     nibble = 0;
    char    nibbles = nibbles_to_print;

    if( (unsigned)nibbles > MAX_NIBBLES )
        nibbles = MAX_NIBBLES;

    while( nibbles > 0 )
    {
        nibbles--;
        nibble = (int)(number >> (nibbles * 4)) & 0x0F;
        if( nibble <= 9 )
            pStr[strlen(pStr)] = (char)('0' + nibble);
        else
            pStr[strlen(pStr)] = (char)('A' - 10 + nibble);
    }
    return;
}

static void
_uint2str(char *pStr, unsigned int number)
{
    uint32_t    divisor = 1;

    /* figure out initial divisor */
    while (number / divisor > 9)
    {
        divisor *= 10;
    }

    /* now mod and print, then divide divisor */
    do {
        pStr[strlen(pStr)] = (char)('0' + (number / divisor % 10));
        divisor /= 10;
    } while (divisor > 0);
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
comm_init(char *pBanner)
{
    bsp_stdio_t     bsp_io ={ .pf_stdout_char = 0, };
    bsp_init(0);
    bsp_get_std_io(&bsp_io);

    g_logout = bsp_io.pf_stdout_string;

    strcpy(g_msg, pBanner);

    g_logout(g_msg, strlen(g_msg));
    return;
}

void
print_log(const char* format, ...)
{
    char    *pch = (char*)format;
    va_list va;
    va_start(va, format);

    do {
        if( !pch ) break;

        while (*pch)
        {
            /* format identification character */
            if( *pch == '%' )
            {
                pch++;

                if( pch )
                {
                    switch( *pch )
                    {
                        case 'u':
                            {
                                const unsigned int number = va_arg(va, unsigned int);
                                memset(g_msg, 0x0, sizeof(g_msg));

                                _uint2str(g_msg, number);
                                g_logout(g_msg, strlen(g_msg));
                            }
                            break;
                        case 'x':
                        case 'X':
                        case 'p':
                            {
                                const unsigned int number = va_arg(va, unsigned int);
                                memset(g_msg, 0x0, sizeof(g_msg));

                                strcpy((void*)&g_msg[strlen(g_msg)], (void*)"0x");
                                _uint2strhex(g_msg, number, 8);
                                g_logout(g_msg, strlen(g_msg));
                                break;
                            }
                        case 's':
                            {
                                char *string = va_arg(va, char *);
                                g_logout(string, strlen(string));
                            }
                            break;
                        case 'c':
                            {
                                char    c = va_arg(va, int);
                                g_logout(&c, 1);
                            }
                            break;
                        default:
                                break;
                    }
                }
            }
            else if (*pch == '\n')
            {
                g_logout(pch, 1);
            }
            else
            {
                g_logout(pch, 1);
            }

            pch++;
        }
    } while(0);

    va_end(va);
    return;
}


void
print_mem(
    char                *prefix,
    uint8_t             *pAddr,
    int                 bytes)
{
    uintptr_t   addr = (uintptr_t)pAddr;
    uint32_t    cnt = (bytes + 0x3) >> 2;
    uint32_t    *pCur = (uint32_t*)pAddr;

    for(int i = 0; i < cnt; i++)
    {
        if( (i & 0x3) == 2  )
            print_log(" -");
        else if( !(i & 0x3) )
        {
            print_log("\n%s%X |", prefix, addr);
            addr += 16;
        }

        print_log(" %X", pCur[i]);
    }
    print_log("\n\n");
    return;
}
