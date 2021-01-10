/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_fc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/24
 * @license
 * @description
 */


#include <stdlib.h>
#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_fc.h"

#include "unity_config.h"
#include "unity.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define BUF_IDX_TX      0
#define BUF_IDX_RX      1
//=============================================================================
//                  Macro Definition
//=============================================================================
#define __pause()       do{ __asm volatile("BKPT #01"); }while(0)

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct flash_protect_area
{
    uint16_t        protected_unit_num;
    uint16_t        real_level;
    uintptr_t       area_head;
    uintptr_t       area_end;

} flash_protect_area_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t                  g_buf[2][FC_ONE_SECTOR_SIZE] __attribute__ ((aligned(4))) = {{0}};
static uint8_t                  g_fc_cache[FC_ONE_SECTOR_SIZE + 20] __attribute__ ((used, aligned(4))) = {0};
static flash_protect_area_t     g_protect_area[FC_PROTECT_LEVEL_ALL] =
{
    { .protected_unit_num = 0,   .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 1,   .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 2,   .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 4,   .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 8,   .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 16,  .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 32,  .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 64,  .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 128, .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
    { .protected_unit_num = 256, .area_head = (uintptr_t)-1, .area_end = (uintptr_t)-1},
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_calc_protection_area(
    flash_protect_area_t    *pArea,
    uint32_t                total_level)
{
    fc_attr_t       fc_attr = { .space_bytes = 0, };
    uintptr_t       flash_mmp_base = 0ul;

    flash_mmp_base = hal_fc_get_flash_base_addr();

    hal_fc_get_flash_attr(&fc_attr);

    // calculate protection area (tail protection)
    for(int i = 0; i < total_level; i++)
    {
        flash_protect_area_t    *pCur_area = &pArea[i];
        uint32_t                protected_unit_num = 0;
        uint32_t                vaild_level = fc_attr.protection_max_level - 1;

        pCur_area->area_end = flash_mmp_base + fc_attr.space_bytes - 1;
        if( i == 0 )
        {
            pCur_area->area_head = pCur_area->area_end;
            continue;
        }

        pCur_area->real_level = i;
        protected_unit_num    = pCur_area->protected_unit_num;

        if( i > vaild_level )
        {
            pCur_area->real_level = vaild_level;
            protected_unit_num    = pArea[vaild_level].protected_unit_num;
        }


        if( i >= fc_attr.protection_max_level )
        {
            pCur_area->real_level = fc_attr.protection_max_level;
            pCur_area->area_head  = flash_mmp_base;
            continue;
        }

        pCur_area->area_head = pCur_area->area_end + 1
                                - (protected_unit_num * fc_attr.one_protected_unit_bytes);
        pCur_area->area_head = ((int)pCur_area->area_head < 0) ? 0 : pCur_area->area_head;
    }

    return;
}

static void
_fc_test_eras(void)
{
    uint32_t    time_start = 0;
    int         sector_cnt = 1;
    int         block_cnt = 1;
    uint8_t     *pBuf = (uint8_t*)g_buf;
    int         buf_len = sizeof(g_buf);
    int         len = 0;
    fc_attr_t   fc_attr = { .space_bytes = 0, };
    uintptr_t   flash_mmp_base = 0ul;

    print_log("\n\n ---\ntest flash erase\n");

    flash_mmp_base = hal_fc_get_flash_base_addr();
    hal_fc_get_flash_attr(&fc_attr);

    flash_mmp_base += (fc_attr.space_bytes >> 1);

    memset(g_buf, 0xAA, sizeof(g_buf));

    time_start = hal_get_tick();
    hal_fc_erase(FC_ERASE_MODE_SECTOR, flash_mmp_base, sector_cnt);
    print_log("erase %u * sector (%x): use %u ms\n",
              sector_cnt, flash_mmp_base, hal_get_tick() - time_start);

    len = FC_ONE_SECTOR_SIZE * sector_cnt;
    len = (len < buf_len) ? len : buf_len;
    hal_fc_read((uintptr_t)pBuf, flash_mmp_base, len);
    for(int i = 0; i < len; i++)
    {
        if( pBuf[i] != 0xFF )
        {
            print_log("sector erase, %u-th byte: erase fail \n", i);
            __pause();
        }
    }

    time_start = hal_get_tick();
    hal_fc_erase(FC_ERASE_MODE_BLOCK, flash_mmp_base, block_cnt);
    print_log("erase %u * block: use %u ms\n",
              block_cnt, hal_get_tick() - time_start);

    len = fc_attr.one_block_bytes * block_cnt;
    len = (len < buf_len) ? len : buf_len;
    hal_fc_read((uintptr_t)pBuf, flash_mmp_base, len);
    for(int i = 0; i < len; i++)
    {
        if( pBuf[i] != 0xFF )
        {
            print_log("block erase, %u-th bytes: erase fail \n", i);
            __pause();
        }
    }

#if 0
    time_start = hal_get_tick();
    hal_fc_erase(FC_ERASE_MODE_CHIP, flash_mmp_base, 1);
    print_log("erase chip: use %u ms\n", hal_get_tick() - time_start);

    len  = fc_attr.space_bytes >> 1;
    pBuf = (uint8_t*)flash_mmp_base;
    for(int i = 0; i < len; i++)
    {
        if( pBuf[i] != 0xFF )
        {
            print_log("chip erase, %u-th bytes: erase fail \n", i);
            __pause();
        }
    }
#endif
    return;
}

static void
_fc_test_read_writ(void)
{
    uint32_t    default_value = 0xAA;
    uintptr_t   flash_mmp_base = 0ul;

    print_log("\n\n ---\ntest flash read/write\n");
    print_log("  <- SR= %x, SR1= %x\n", hal_fc_get_sr() & 0xFF, hal_fc_get_sr1() & 0xFF);

    flash_mmp_base = hal_fc_get_flash_base_addr();

    do {
        int         sector_num = 0;
        fc_attr_t   fc_attr = { .space_bytes = 0, };

        hal_fc_get_flash_attr(&fc_attr);

        flash_mmp_base += (fc_attr.space_bytes >> 1);

        sector_num = (fc_attr.space_bytes >> 1) / FC_ONE_SECTOR_SIZE;

        for(int i = 0; i < sector_num; ++i)
        {
            fc_err_t        rval = FC_ERR_OK;
            uintptr_t       flash_addr = flash_mmp_base + i * FC_ONE_SECTOR_SIZE;
            int             len = FC_ONE_SECTOR_SIZE;
            uint32_t        time_start = hal_get_tick();

            // erase sector
            rval = hal_fc_erase(FC_ERASE_MODE_SECTOR, flash_addr, 1);
            if( rval )
            {
                __pause();
            }
            print_log("erase %u-th sector (%x): use %u ms\n", i, flash_addr, hal_get_tick() - time_start);

            // read sector and compare
            time_start = hal_get_tick();
            hal_fc_read((uintptr_t)g_buf[BUF_IDX_RX], flash_addr, len);
            print_log("read: use %u ms\n", hal_get_tick() - time_start);

            for(int j = 0; j < len; j++)
            {
                if( g_buf[BUF_IDX_RX][j] != 0xFF )
                {
                    print_log("%u-th byte: erase fail \n", j);
                    __pause();
                }
            }

            // write sector
            memset(g_buf[BUF_IDX_TX], default_value, len);
            time_start = hal_get_tick();
            rval = hal_fc_write_raw((uintptr_t)g_buf[BUF_IDX_TX], flash_addr, len, 0);
            if( rval )
            {
                __pause();
            }
            print_log("  wr: %u bytes, %u ms\n", len, hal_get_tick() - time_start);

            // read sector and compare
            hal_fc_read((uintptr_t)g_buf[BUF_IDX_RX], flash_addr, len);
            for(int j = 0; j < len; j++)
            {
                if( g_buf[BUF_IDX_RX][j] != g_buf[BUF_IDX_TX][j] )
                {
                    print_log("%u-th byte: program fail (rx= %x, tx= %x) \n",
                        j, g_buf[BUF_IDX_RX][j], g_buf[BUF_IDX_TX][j]);
                    __pause();
                }
            }
            print_log("  read and compare ok\n");
        }
    } while(0);

    return;
}


static void
_fc_test_protection(void)
{
    fc_err_t    rval = FC_ERR_OK;
    uintptr_t   flash_mmp_base = 0ul;
    fc_attr_t   fc_attr = { .space_bytes = 0, };

    print_log("\n\n ---\ntest protection\n");

    flash_mmp_base = hal_fc_get_flash_base_addr();

    hal_fc_get_flash_attr(&fc_attr);

    for(fc_protect_level_t level = FC_PROTECT_LEVEL_1; level < FC_PROTECT_LEVEL_ALL; level++)
    {
        uintptr_t   head = 0;
        uintptr_t   tail = 0;
        uint32_t    real_lv = 0;
        int         len = (int)fc_attr.one_protected_unit_bytes;

        len = (len < sizeof(g_buf[BUF_IDX_TX])) ? len : sizeof(g_buf[BUF_IDX_TX]);

        for(int j = 0; j < len; j++)
        {
            g_buf[BUF_IDX_TX][j] = rand() & 0xFF;
        }

        // set un-protect
        hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

        // get protection info
        head    = g_protect_area[level].area_head;
        tail    = g_protect_area[level].area_end;
        real_lv = g_protect_area[level].real_level;

        if( head < (flash_mmp_base + (fc_attr.space_bytes >> 1)) )
            continue;

        print_log("erase %x \n", head);
        rval = hal_fc_erase(FC_ERASE_MODE_SECTOR, head, 1);
        if( rval )
        {
            print_log("%s", "erase sector fail\n");
            __pause();
        }

        rval = hal_fc_write_raw((uintptr_t)g_buf[BUF_IDX_TX], head, len, 0);
        if( rval )
        {
            print_log("%s", "write to flash fail\n");
            __pause();
        }

        // set protect
        hal_fc_set_protection((fc_protect_level_t)real_lv, true);

        print_log("\n --- protection level %u\n", level);
        print_log("  <- SR= %x, SR1= %x\n", hal_fc_get_sr() & 0xFF, hal_fc_get_sr1() & 0xFF);
        print_log("  range: %x ~ %x \n\n", head, tail);

        // erase sector
        rval = hal_fc_erase(FC_ERASE_MODE_SECTOR, head, 1);
        if( rval )
        {
            print_log("erase sector fail (err= %x)\n", rval);
        }

        // read sector and check protection is working or not
        rval = hal_fc_read((uintptr_t)g_buf[BUF_IDX_RX], head, len);
        if( rval )
        {
            print_log("%s", "read from flash fail\n");
            __pause();
        }

        if( memcmp(g_buf[BUF_IDX_RX], g_buf[BUF_IDX_TX], len) )
        {
            print_log("%s", "protect fail \n");

            print_log("TX: %x %x %x %x\n",
                      g_buf[BUF_IDX_TX][0], g_buf[BUF_IDX_TX][1],
                      g_buf[BUF_IDX_TX][2], g_buf[BUF_IDX_TX][3]);
            print_log("RX: %x %x %x %x\n",
                      g_buf[BUF_IDX_RX][0], g_buf[BUF_IDX_RX][1],
                      g_buf[BUF_IDX_RX][2], g_buf[BUF_IDX_RX][3]);
            __pause();
            break;
        }

        print_log("%s", "  test ok\n");
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
#if defined(CONFIG_CPU_ARM_CM4)
void user_pre_sys_setting(void)
{
    /**
     *  initialize system tick
     *      SystemFrequency / 1000    : trigger IRQ every 1ms
     *      SystemFrequency / 100000  : trigger IRQ every 10us
     *      SystemFrequency / 1000000 : trigger IRQ every 1us
     *
     *  In FreeRTOS, it can't set SysTick_Config()
     */
    SysTick_Config(hal_sys_get_cpu_clk() / 1000);
    return;
}
#endif

TEST("test FC", "[FC write/read]", TEST_PRIORITY_LOW)
{
    comm_init("\n\n################################ Test Flash\n");

    srand(123);

    do {
        hal_fc_init(FC_CLK_DIV_4);

        _calc_protection_area(g_protect_area, FC_PROTECT_LEVEL_ALL);

        hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);
        print_log("  <- SR= %x, SR1= %x\n", hal_fc_get_sr() & 0xFF, hal_fc_get_sr1() & 0xFF);

        _fc_test_eras();

        _fc_test_read_writ();

        _fc_test_protection();

        print_log("\n========== done~~\n");
    } while(0);

    return;
}
