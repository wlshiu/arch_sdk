/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_img.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/06
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_scu.h"
#include "hal_fc.h"
#include "hal_img.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ENABLE_DEBUG

#define CONFIG_TEST_FLASH_PHY_ADDRESS       0x30000

#ifndef LOG_YELLOW
#define LOG_YELLOW              "\033[33m"
#endif

#ifndef LOG_RESET
#define LOG_RESET               "\033[m"
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#define blocking()              do { __asm("nop"); } while(1)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

static uintptr_t        g_img_attr_part_addr = 0ul;
static uintptr_t        g_img_gdata_part_addr = 0ul;

static uint32_t         g_flash_space = 0ul;

static uint8_t          g_in_buf[5 << 10] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_img_log_mem(
    char                *prefix,
    uint8_t             *pAddr,
    int                 bytes,
    unsigned int        has_out_u32le)
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

static int
_img_reboot(void)
{
#define SW_STRAP_TYPE_BLD       0   /*!< boot to bootloader */
#define SW_STRAP_TYPE_AUTO      1   /*!< auto booting */

    uint32_t    remap_type = 0;
    uint32_t    offset = 16;

    hal_scu_set_hw_reset(SCU_RST_ATE_SPI_FLASH_HRST, false);

    {
        scu_sw_strap_t      sw_strap = {0};

        hal_scu_get_sw_strap(&sw_strap);

        sw_strap.sw_strap_type = SW_STRAP_TYPE_AUTO;
        hal_scu_set_sw_strap(&sw_strap);
    }

    hal_scu_set_mmp_type(remap_type);
    hal_scu_set_boot_offset(offset);
    hal_scu_reboot(true);

    while(1) __asm("nop");
    return 0;
}

static uint32_t*
_img_malloc(int length)
{
    static uint32_t     g_tmp_buf[1024] = {0};
    uint32_t            *pBuf = 0;
    do {
        if( length > sizeof(g_tmp_buf) )
            break;

        pBuf = g_tmp_buf;
    } while(0);
    return pBuf;
}

static void
_img_free(void *p)
{
    return;
}

static void
_test_img_attr(
    int     vol_idx)
{
    uint32_t    rval = 0;
    uintptr_t   vir_addr = 0ul;

    for(int i = 0; i < 17; i++)
    {
        img_attr_flag_t     attr_flag = IMG_ATTR_NONE;

        switch(i)
        {
            case 0:
                attr_flag = IMG_ATTR_BOOTABLE;
                print_log("----- test IMG_ATTR_BOOTABLE\n");
                break;
            case 1:
                attr_flag = IMG_ATTR_PRETEST;
                print_log("----- test IMG_ATTR_PRETEST\n");
                break;
            default:
                attr_flag = IMG_ATTR_PRIORITY;
                print_log("----- test IMG_ATTR_PRIORITY\n");
                break;
        }

        #if defined(CONFIG_ENABLE_DEBUG)
        {
            int                     volume_id = -1;
            uintptr_t               phy_addr = 0ul;
            img_partition_hdr_t     *pHdr = 0;
            uint32_t                data = 0;

            rval = hal_img_get_volume_id(&volume_id);
            if( rval ) break;

            phy_addr = (volume_id == vol_idx)
                     ? g_img_attr_part_addr
                     : g_img_attr_part_addr + (g_flash_space >> 1);

            hal_phy2vir((uintptr_t)phy_addr, &vir_addr);

            pHdr = (img_partition_hdr_t*)vir_addr;

            data = *((uint32_t*)(pHdr + 1));
            print_log("%s@%x\n", __func__, ((uint32_t*)(pHdr + 1)));
            print_log("is_bootable = %x\n"
                      "is_pre_test = %x\n"
                      "priority    = %x\n",
                      data & 0x1, (data >> 15) & 0x1, (data >> 1) & 0x3FFF);
        }
        #endif

        rval = (uint32_t)hal_img_clear_attr(vol_idx, attr_flag, _img_malloc, _img_free);
        if( rval )
        {
            print_log("clear img attr fail (err: x%x)\n", rval);
            blocking();
        }

        #if defined(CONFIG_ENABLE_DEBUG)
        {
            img_partition_hdr_t     *pHdr = (img_partition_hdr_t*)vir_addr;
            uint32_t                data = 0;
            data = *((uint32_t*)(pHdr + 1));
            print_log("%s@%x\n", __func__, ((uint32_t*)(pHdr + 1)));
            print_log("  is_bootable = %x\n"
                      "  is_pre_test = %x\n"
                      "  priority    = %x\n",
                      data & 0x1, (data >> 15) & 0x1, (data >> 1) & 0x3FFF);
        }
        #endif
    }
    return;
}

static void
_test_img_wr_raw(
    int     vol_idx)
{
    uint32_t    rval = 0;
    do {
        img_wr_info_t       info = {0};

        for(int i = 0; i < sizeof(g_in_buf); i++)
        {
            g_in_buf[i] = i & 0xFF;
        }

        info.target_vol_index = vol_idx;
        info.phy_dest_address = CONFIG_TEST_FLASH_PHY_ADDRESS;
        info.cb_malloc        = _img_malloc;
        info.cb_free          = _img_free;
        info.pData            = g_in_buf;
        info.data_len         = sizeof(g_in_buf);
        rval = (uint32_t)hal_img_write_raw(&info);
        if( rval )
        {
            print_log("program fail (err: x%x)\n", rval);
            blocking();
        }

        print_log("------- dump");

        {   // dump data
            int         volume_id = -1;
            uintptr_t   phy_addr = CONFIG_TEST_FLASH_PHY_ADDRESS;
            uintptr_t   vir_addr = 0ul;

            rval = hal_img_get_volume_id(&volume_id);
            if( rval ) break;

            phy_addr = (volume_id == vol_idx) ? phy_addr : phy_addr + (g_flash_space >> 1);

            rval = (uint32_t)hal_phy2vir(phy_addr, &vir_addr);
            if( rval )
            {
                print_log("memory mapping fail (err: x%x)\n", rval);
                blocking();
            }

            _img_log_mem("", (uint8_t*)(vir_addr - 16), sizeof(g_in_buf) + 64, 1);
        }
    } while(0);
    return;
}

static void
_test_img_wr_partition(
    int     vol_idx)
{
#define CONFIG_PARTIAL_SLICE_SIZE       0x200
    uint32_t    rval = 0;
    do {
        img_partition_hdr_t     partition_hdr = {0};
        img_wr_partition_t      part_info = {0};
        uint32_t                raw_size = sizeof(g_in_buf);

        for(int i = 0; i < raw_size; i++)
        {
            g_in_buf[i] = i & 0xFF;
        }

        // set partition header
        partition_hdr.payload_offset = sizeof(img_partition_hdr_t);
        partition_hdr.valid_size     = sizeof(img_partition_hdr_t) + raw_size;
        partition_hdr.has_crc        = 1;
        partition_hdr.partition_size = (partition_hdr.valid_size + (FC_ONE_SECTOR_SIZE - 1)) & ~(FC_ONE_SECTOR_SIZE - 1);

        //-----------------------------
        // writing partition info
        part_info.cb_malloc        = _img_malloc;
        part_info.cb_free          = _img_free;

        part_info.partition_id     = IMG_PARTITION_ID_USR2;
        part_info.target_vol_index = vol_idx & 0x1;
        part_info.state            = IMG_PROGRAM_STATE_IDLE;

        // header
        part_info.target_phy_address = CONFIG_TEST_FLASH_PHY_ADDRESS;
        part_info.state              = IMG_PROGRAM_STATE_IMG_HEADER;
        part_info.pPartition_hdr     = &partition_hdr;
        rval = hal_img_wr_partition(&part_info);
        if( rval )
        {
            print_log("write partition fail (err: x%x)\n", rval);
            blocking();
        }

        {   // data
            int     remain = raw_size;

            part_info.state = IMG_PROGRAM_STATE_IMG_DATA;
            part_info.pData = g_in_buf;

            part_info.target_phy_address += sizeof(img_partition_hdr_t);
            while( remain )
            {
                uint32_t    slice_size = (remain < CONFIG_PARTIAL_SLICE_SIZE) ? remain : CONFIG_PARTIAL_SLICE_SIZE;

                part_info.data_len = slice_size;
                rval = hal_img_wr_partition(&part_info);
                if( rval )
                {
                    print_log("write partition fail (err: x%x)\n", rval);
                    blocking();
                }

                part_info.target_phy_address += slice_size;
                part_info.pData              += slice_size;
                remain                       -= slice_size;
            }
        }

        if( partition_hdr.has_crc ) // CRC
        {
            part_info.pPartition_hdr    = &partition_hdr;
            part_info.state             = IMG_PROGRAM_STATE_IMG_CRC;
            rval = hal_img_wr_partition(&part_info);
            if( rval )
            {
                print_log("write partition fail (err: x%x)\n", rval);
                blocking();
            }
        }

        print_log("------- dump");

        {   // dump data
            int         volume_id = -1;
            uintptr_t   phy_addr = CONFIG_TEST_FLASH_PHY_ADDRESS;
            uintptr_t   vir_addr = 0ul;

            rval = hal_img_get_volume_id(&volume_id);
            if( rval ) break;

            phy_addr = (volume_id == vol_idx) ? phy_addr : phy_addr + (g_flash_space >> 1);

            rval = (uint32_t)hal_phy2vir(phy_addr, &vir_addr);
            if( rval )
            {
                print_log("memory mapping fail (err: x%x)\n", rval);
                blocking();
            }

            _img_log_mem("", (uint8_t*)(vir_addr - 16), sizeof(g_in_buf) + 64, 0);
        }
    } while(0);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void user_pre_sys_setting(void)
{
    SysTick_Config(hal_sys_get_pclk() / 1000);

    hal_fc_init(FC_CLK_DIV_4);
    return;
}

int main(void)
{
    comm_init("\n\n################################ Test Image\n");

    do {
        int         rval = 0;
        int         cur_vol_idx = -1;
        int         act_vol_idx = -1;
        uint32_t    offset = 0;

        hal_scu_get_boot_offset(&offset);
        rval = (uint32_t)hal_img_get_volume_id(&cur_vol_idx);
        if( rval )
        {
            print_log("get image volume fail (err: x%x)\n", rval);
            blocking();
        }

        {
            fc_attr_t   fc_attr = { .space_bytes = 0ul, };
            rval = hal_fc_get_flash_attr(&fc_attr);
            if( rval ) break;

            g_flash_space = fc_attr.space_bytes;
        }

        print_log("current volume '%u' (offset = %x), flash base %x\n\n",
                  cur_vol_idx, offset, hal_fc_get_flash_base_addr());

        #if 0
        act_vol_idx = cur_vol_idx;
        #else
        act_vol_idx = cur_vol_idx ^ 0x1;
        #endif // 1

        print_log("\n############ access '%u-th' volume\n\n", act_vol_idx);

        {   // show gData message
            img_gdata_t     *pGData = 0ul;
            hal_img_get_gdata(act_vol_idx, &pGData);
            for(int i = 0; i < CONFIG_IMG_PARTITION_NUM_PER_VOLUME; i++)
            {
                img_partition_t     *pPart = &pGData->part_entries[i];
                if( !pPart->uid || pPart->uid == (-1) )
                    continue;

                switch(i)
                {
                    case IMG_PARTITION_ID_IMG_ATTR:
                        g_img_attr_part_addr = pPart->partition_start.addr_value & 0xFFFFFFFF;
                        break;
                    case IMG_PARTITION_ID_GDATA:
                        g_img_gdata_part_addr = pPart->partition_start.addr_value & 0xFFFFFFFF;
                    default:
                        break;
                }

                print_log("entry %c%c%c%c\n",
                          (pPart->uid & 0xFF), ((pPart->uid >> 8) & 0xFF),
                          ((pPart->uid >> 16) & 0xFF), ((pPart->uid >> 24) & 0xFF));
                print_log("  part: x%x, %u\n", pPart->partition_start.addr_value, pPart->partition_len);
                print_log("  raw : x%x, %u\n", pPart->raw_addr.addr_value, pPart->raw_len);
            }

            rval = (int)hal_img_verify_partition(act_vol_idx, g_img_gdata_part_addr);
            if( rval )
            {
                print_log("verify partition fail (err: 0x%x)\n", (uint32_t)rval);
            }
        }

        print_log("\n========================= test image attr\n\n");
        _test_img_attr(act_vol_idx);
        rval = hal_img_verify_partition(act_vol_idx, g_img_attr_part_addr);
        if( rval )
        {
            print_log("attr partition fail (err: %x)\n", rval);
        }

        act_vol_idx ^= 0x1;
        print_log("\n############ access '%u-th' volume\n\n", act_vol_idx);
        _test_img_attr(act_vol_idx);
        rval = hal_img_verify_partition(act_vol_idx, g_img_attr_part_addr);
        if( rval )
        {
            print_log("attr partition fail (err: 0x%x)\n", rval);
        }

        print_log("\n========================= test write raw data\n\n");
        _test_img_wr_raw(act_vol_idx);

        print_log("\n========================= test write image partition\n\n");
        _test_img_wr_partition(act_vol_idx);

        rval = hal_img_verify_partition(act_vol_idx, CONFIG_TEST_FLASH_PHY_ADDRESS);
        if( rval )
        {
            print_log("program partition fail (err: 0x%x)\n", rval);
        }
    } while(0);

    print_log("--------end\n\n");

    while(1)
        __asm("nop");

    _img_reboot();

    return 0;
}
