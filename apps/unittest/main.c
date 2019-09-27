/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/27
 * @license
 * @description
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "unity/unity_config.h"
#include "unity/unity.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ARG_CNT_MAX      6
//=============================================================================
//                  Macro Definition
//=============================================================================
#define unity_printf        printf
#define unity_getchar       getchar
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static test_item_t      *g_pFirst_item = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_unity_list_test_items(void)
{
    int             cnt = 1;
    test_item_t     *pCur = g_pFirst_item;

    unity_printf("\n\nlist the test items:\n");

    while( pCur )
    {
        unity_printf("+ %02d)  %s\n", cnt, pCur->pTest_name);
        pCur = pCur->next;
        cnt++;
    }
    return cnt;
}

static void
_unity_run_single_test(const test_item_t *pItem)
{
    Unity.TestFile       = pItem->file;
    Unity.CurrentDetail1 = pItem->pDescription;
    UnityDefaultTestRun(pItem->pf_exec, pItem->pTest_name, pItem->line);
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void setUp(void)
{
  /* This is run before EACH TEST */
}

void tearDown(void)
{
    /* This is run after EACH TEST */
}

void
unity_register_item(test_item_t *pItem)
{
    if( g_pFirst_item )
    {
        test_item_t     *pCur = g_pFirst_item;

        while( pCur->next )
            pCur = pCur->next;

        pCur->next = pItem;
    }
    else
    {
        g_pFirst_item = pItem;
    }

    return;
}

void unity_run(void)
{
    char    line[128] = {0};
    int     pos = 0;

    _unity_list_test_items();

    while(1)
    {
        char    ch = unity_getchar();

        if( ch != '\n' && ch != '\r' )
        {
            if( pos < sizeof(line) - 1 )
                line[pos++] = ch;
            continue;
        }

        line[pos++] = ch;

        {   // parsing line
            char            *pCur = &line[0], *pEnd = 0;
            unsigned short  arg_cnt = 0, is_arg_head = 1;;
            char            *pCmd_args[CONFIG_ARG_CNT_MAX] = {0};
            long            item_index = 0;

            test_item_t     *pCur_item = g_pFirst_item;

            pEnd = &line[pos];
            while( pCur < pEnd )
            {
                char    c = *pCur;

                if( c == ' ' || c == '\t' || c == '\n' )
                {
                    *pCur++ = '\0';
                    is_arg_head = 1;
                    continue;
                }

                if( is_arg_head && (arg_cnt < CONFIG_ARG_CNT_MAX) &&
                    ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                     (c >= 'A' && c <= 'Z') || c == '_' || c == '?') )
                {
                    pCmd_args[arg_cnt++] = pCur;
                    is_arg_head = 0;
                }

                pCur++;
            }

            if( !arg_cnt )
            {
                continue;
            }

            UNITY_BEGIN();

            if( !strncmp(pCmd_args[0], "all", strlen("all")) )
            {
                test_item_t     *pCur_item = g_pFirst_item;

                while( pCur_item )
                {
                    _unity_run_single_test((const test_item_t*)pCur_item);

                    pCur_item = pCur_item->next;
                }
            }
            else if( (item_index = strtol(pCmd_args[0], &pCur, 10)) )
            {
                // single test with index


                while( pCur_item )
                {
                    if( item_index == 1 )
                        _unity_run_single_test((const test_item_t*)pCur_item);

                    item_index--;
                    pCur_item = pCur_item->next;
                }
            }

            UNITY_END();

            memset(line, 0x0, sizeof(line));
        }
    }
    return;
}

int main(void)
{
    int     rval = 0;
    unity_run();
    return rval;
}


TEST("test a + b", "[ADD function]")
{
    int     a = 5, b = 3;

    TEST_ASSERT((a + b)==8);
}
