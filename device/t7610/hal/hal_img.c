/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_img.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/06
 * @license
 * @description
 */


#include <string.h>
#include "hal_scu.h"
#include "hal_fc.h"
#include "hal_img.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CRC_CSR_BASE          0x58000000ul

typedef enum crc_type
{
    CRC_TYPE_CRC5       = 0x04,
    CRC_TYPE_CRC8       = 0x07,
    CRC_TYPE_CRC16      = 0x0F,
    CRC_TYPE_CRC24      = 0x17,
    CRC_TYPE_CRC32      = 0x1F,
} crc_type_t;

#define CRC32_POLYNOMIAL        0x04C11DB7


#define HAL_IMG_1_SECTOR_SIZE           FC_ONE_SECTOR_SIZE
#define HAL_IMG_PART_GDATA_PHY_ADDR     0x000FF000ul
//=============================================================================
//                  Macro Definition
//=============================================================================
#define crc32_hw_init(pReg)                             \
    do{ (pReg)->CRC_MODEb.CRC_RESET = 1;                \
        while( (pReg)->CRC_MODEb.CRC_RESET ) {}         \
        (pReg)->CRC_MODEb.CRC_TYPE  = CRC_TYPE_CRC32;   \
        (pReg)->CRC_MODEb.CRC_MSBIN = 0;                \
        (pReg)->CRC_MODEb.CRC_XOREN = 1;                \
        (pReg)->CRC_MODEb.CRC_bREV  = 1;                \
        (pReg)->CRC_POLY            = CRC32_POLYNOMIAL; \
    }while(0)

#define crc32_hw_calc(pData_u8, length, pReg, pCrc_value)                  \
    do{ uint32_t    i = 0;                                                 \
        (pReg)->CRC_INIT_DATA = *(pCrc_value);                             \
        (pReg)->CRC_bLEN      = length << 3;                               \
        while( i != length ) (pReg)->CRC_DATA_ENTRY_U8 = pData_u8[i++];    \
        while( !((pReg)->CRC_MODEb.CRC_FLAG) ) {}                          \
        *(pCrc_value) = (pReg)->CRC_CODE & 0xFFFFFFFF;                     \
    }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct crc_csr
{
    union {
        volatile uint32_t    CRC_MODE;
        struct {
            volatile uint32_t CRC_TYPE  : 5;    // bit[4:0], follow enum CRC_TYPE_e
            volatile uint32_t CRC_MSBIN : 1;    // bit[5], 0 : input LSB first, 1 : input MSB first
            volatile uint32_t CRC_FLAG  : 1;    // bit[6]
            volatile uint32_t CRC_RESET : 1;    // bit[7]
            volatile uint32_t           : 1;    // bit[8]
            volatile uint32_t CRC_bREV  : 1;    // bit[9], 0 : output not reverse, 1 : output reverse
            volatile uint32_t CRC_XOREN : 1;    // bit[10], 0 : not xor, 1 : xor 0xFFFFFFFF
            volatile uint32_t           : 21;   // bit[31:11]
        } CRC_MODEb;
    };

    volatile uint32_t    CRC_POLY;
    volatile uint32_t    CRC_CODE;

    union {
        volatile uint32_t    CRC_DATA_ENTRY_U32;
        volatile uint16_t    CRC_DATA_ENTRY_U16;
        volatile uint8_t     CRC_DATA_ENTRY_U8;
    };

    volatile uint32_t    CRC_bLEN;
    volatile uint32_t    CRC_INIT_DATA;

} crc_csr_t;


/**
 * image attribute context of a image volume
 */
typedef struct img_attr
{
    union {
        volatile uint32_t    attr_value;
        struct {
            volatile uint32_t   is_pre_test     : 1;  /**< first priority to execute */
            volatile uint32_t   priority        : 14; /**< image executing priority */
            volatile uint32_t   is_bootable     : 1;  /**< network bootstrap is success or not */
        } b;
    };

#define CONFIG_IMG_ATTR_CRC_MAX_TIMES       64
    uint32_t    crc32[];    /**< default value is 0xFFFFFFFF. If not be used, set to 0x0. */
} img_attr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t         g_flash_space = 0ul;
// static uint32_t         g_mmp_type = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
_hal_img_calc_crc32(uint8_t *pData, int len)
{
    uint32_t    crc = 0xFFFFFFFF;

    if( !pData || len == 0 )
        return crc;

    crc32_hw_init((crc_csr_t*)CRC_CSR_BASE);
    crc32_hw_calc(pData, len, (crc_csr_t*)CRC_CSR_BASE, &crc);
    return crc;
}

static img_err_t
_hal_img_get_flash_space(
    uint32_t    *pFlash_space)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        int         rst = 0;
        fc_attr_t   fc_attr = { .space_bytes = 0ul, };

        rst = hal_fc_get_flash_attr(&fc_attr);
        if( rst )
        {
            rval = IMG_ERR_UNKNOWN_DEVICE;
            break;
        }

        *pFlash_space = fc_attr.space_bytes;
    } while(0);
    return rval;
}

static img_err_t
_hal_img_verify_gdata(
    uintptr_t       flash_phy_addr,
    img_gdata_t     **ppGData_user)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        uintptr_t               vir_addr = 0ul;
        img_partition_hdr_t     *pPart_hdr = 0;
        img_gdata_t             *pGData = 0;
        img_partition_t         *pEntry = 0;
        uint32_t                *pCrc32 = 0;

        if( hal_phy2vir(flash_phy_addr, &vir_addr) )
        {
            rval = IMG_ERR_UNKNOWN_DEVICE;
            break;
        }

        pPart_hdr = (img_partition_hdr_t*)vir_addr;
        pGData    = (img_gdata_t*)((uintptr_t)pPart_hdr + pPart_hdr->payload_offset);
        pEntry    = &pGData->part_entries[IMG_PARTITION_ID_GDATA];
        if( pPart_hdr->tag_id != HAL_IMG_UID_GDATA || pEntry->uid != HAL_IMG_UID_GDATA )
        {
            rval = IMG_ERR_INVALID_PARTITION;
            break;
        }

        pCrc32 = (uint32_t*)(((uintptr_t)(pGData + 1) + 0x3) & ~0x3);
        if( *pCrc32 != _hal_img_calc_crc32((uint8_t*)pPart_hdr, ALIGN4(pPart_hdr->valid_size)) )
        {
            rval = IMG_ERR_CRC_FAIL;
            break;
        }

        *ppGData_user = pGData;
    } while(0);

    return rval;
}

static img_err_t
_hal_img_get_partition_addr(
    uint32_t            vol_index,
    img_partition_id_t  part_id,
    uintptr_t           *pPhy_addr)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        int             volume_id = -1;
        uintptr_t       phy_addr = 0ul;
        img_gdata_t     *pGData = 0;

        rval = hal_img_get_volume_id(&volume_id);
        if( rval ) break;

        phy_addr = HAL_IMG_PART_GDATA_PHY_ADDR;
        phy_addr = (volume_id == vol_index) ? phy_addr : (phy_addr + (g_flash_space >> 1));

        rval = _hal_img_verify_gdata(phy_addr, &pGData);
        if( rval ) break;

        phy_addr = pGData->part_entries[part_id].partition_start.addr_value & 0xFFFFFFFF;
        phy_addr = (volume_id == vol_index) ? phy_addr : (phy_addr + (g_flash_space >> 1));

        *pPhy_addr = phy_addr;
    } while(0);
    return rval;
}

static int
_hal_img_mmap(
    uintptr_t   phy_addr,
    uintptr_t   *pVir_addr)
{
    int         rval = 0;
    do {
        if( hal_phy2vir(phy_addr, pVir_addr) )
        {
            rval = IMG_ERR_UNKNOWN_DEVICE;
            break;
        }
    } while(0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
img_err_t
hal_img_get_volume_id(int *pVol_index)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        uint32_t    offset = 0;

        if( !pVol_index )
        {
            rval = IMG_ERR_NULL_POINTER;
            break;
        }

        // hal_scu_get_mmp_type(&g_mmp_type);

        if( !g_flash_space )
        {
            rval = _hal_img_get_flash_space(&g_flash_space);
            if( rval ) break;
        }

        hal_scu_get_boot_offset(&offset);

        *pVol_index = (offset < (g_flash_space >> 1)) ? 0 : 1;

    } while(0);
    return rval;
}

img_err_t
hal_img_get_gdata(
    int             volume_index,
    img_gdata_t     **ppGData)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        int         vol_id = -1;
        uintptr_t   phy_addr = 0ul;

        if( !ppGData )
        {
            rval = IMG_ERR_NULL_POINTER;
            break;
        }

        rval = hal_img_get_volume_id(&vol_id);
        if( rval ) break;

        phy_addr = HAL_IMG_PART_GDATA_PHY_ADDR;
        phy_addr = (volume_index == vol_id) ? phy_addr : (phy_addr + (g_flash_space >> 1));

        rval = _hal_img_verify_gdata(phy_addr, ppGData);
        if( rval ) break;

    } while(0);
    return rval;
}

img_err_t
hal_img_clear_attr(
    uint32_t            vol_index,
    img_attr_flag_t     attr_flag,
    uint32_t*  (*cb_malloc)(int length),
    void       (*cb_free)(void *p))
{
    img_err_t               rval = IMG_ERR_OK;
    img_partition_hdr_t     *pPart_hdr = 0;

    do {
        uintptr_t           vir_addr = 0ul;
        uintptr_t           phy_addr = 0ul;

        if( !cb_malloc || !cb_free )
        {
            rval = IMG_ERR_WRONG_PARAM;
            break;
        }

        rval = _hal_img_get_partition_addr(vol_index, IMG_PARTITION_ID_IMG_ATTR, &phy_addr);
        if( rval ) break;

        {   // convert memory and set attribute
            int             i = 0;
            img_attr_t      *pImg_attr_new = 0;
            int             valid_size = 0;

            valid_size = sizeof(img_partition_hdr_t) + sizeof(img_attr_t) + (CONFIG_IMG_ATTR_CRC_MAX_TIMES << 2);
            if( !(pPart_hdr = (img_partition_hdr_t*)cb_malloc(valid_size)) )
            {
                rval = IMG_ERR_NULL_POINTER;
                break;
            }
            memset(pPart_hdr, 0xFF, valid_size);

            if( hal_phy2vir(phy_addr, &vir_addr) )
            {
                rval = IMG_ERR_UNKNOWN_DEVICE;
                break;
            }

            memcpy((void*)pPart_hdr, (void*)vir_addr, valid_size);

            pImg_attr_new = (img_attr_t*)(pPart_hdr + 1);

            //---------------------
            // set attribute flag
            if( attr_flag == IMG_ATTR_BOOTABLE )
            {
                if( !pImg_attr_new->b.is_bootable )
                    break;

                pImg_attr_new->b.is_bootable = 0;
            }
            else if( attr_flag == IMG_ATTR_PRETEST )
            {
                if( !pImg_attr_new->b.is_pre_test )
                    break;

                pImg_attr_new->b.is_pre_test = 0;
            }
            else if( attr_flag == IMG_ATTR_PRIORITY )
            {
                if( !pImg_attr_new->b.priority )
                    break;

                pImg_attr_new->b.priority >>= 0x1;
            }
            else
            {
                rval = IMG_ERR_WRONG_PARAM;
                break;
            }

            for(i = 0; i < CONFIG_IMG_ATTR_CRC_MAX_TIMES; i++)
            {
                if( pImg_attr_new->crc32[i] == 0x0 )
                    continue;

                pImg_attr_new->crc32[i] = 0x0;
                break;
            }

            if( ++i < (CONFIG_IMG_ATTR_CRC_MAX_TIMES - 1) )
            {
                pImg_attr_new->crc32[i] = _hal_img_calc_crc32((uint8_t*)pPart_hdr,
                                                              ALIGN4(sizeof(img_partition_hdr_t) + sizeof(img_attr_t)));

                // set un-protect
                hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

                phy_addr += hal_fc_get_flash_base_addr();

                if( hal_fc_write_raw((uintptr_t)pImg_attr_new,
                                     (uintptr_t)phy_addr + sizeof(img_partition_hdr_t),
                                     sizeof(img_attr_t) + (CONFIG_IMG_ATTR_CRC_MAX_TIMES << 2),
                                     _hal_img_mmap) )
                {
                    rval = IMG_ERR_PROGRAM_FAIL;
                    break;
                }
            }
        }
    } while(0);

    // set protect
    hal_fc_set_protection(FC_PROTECT_LEVEL_FULL_PROTECT, true);

    if( pPart_hdr && cb_free ) cb_free(pPart_hdr);

    return rval;
}

img_err_t
hal_img_write_raw(
    img_wr_info_t   *pInfo)
{
    img_err_t       rval = IMG_ERR_OK;
    uint8_t         *pCache = 0;
    do {
        uintptr_t       phy_addr = 0ul;

        if( !pInfo || !pInfo->cb_malloc || !pInfo->cb_free )
        {
            rval = IMG_ERR_WRONG_PARAM;
            break;
        }

        // request cache buffer
        if( !(pCache = (uint8_t*)pInfo->cb_malloc(HAL_IMG_1_SECTOR_SIZE)) )
        {
            rval = IMG_ERR_NULL_POINTER;
            break;
        }

        if( (uintptr_t)pCache & 0x3 )
        {
            rval = IMG_ERR_NOT_ALIGNMENT;
            break;
        }
        memset(pCache, 0xFF, HAL_IMG_1_SECTOR_SIZE);

        {
            int     volume_id = -1;

            rval = hal_img_get_volume_id(&volume_id);
            if( rval ) break;

            phy_addr = pInfo->phy_dest_address & ~0xF0000000ul;
            phy_addr = (volume_id == pInfo->target_vol_index)
                     ? phy_addr : (phy_addr + (g_flash_space >> 1));

            phy_addr += hal_fc_get_flash_base_addr();
        }

        {   // program data
            int     ret = 0;

            hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

            ret = hal_fc_write((uintptr_t)pInfo->pData,
                               phy_addr,
                               pInfo->data_len,
                               _hal_img_mmap,
                               (uint32_t*)pCache);
            if( ret )
            {
                rval = IMG_ERR_PROGRAM_FAIL;
                break;
            }
        }
    } while(0);

    // set protect
    hal_fc_set_protection(FC_PROTECT_LEVEL_FULL_PROTECT, true);

    if( pCache && pInfo->cb_free ) pInfo->cb_free(pCache);

    return rval;
}

img_err_t
hal_img_wr_partition(
    img_wr_partition_t  *pInfo)
{
    img_err_t       rval = IMG_ERR_OK;
    uint8_t         *pCache = 0;

    do {
        uintptr_t               phy_addr = 0ul;

        if( !pInfo || !pInfo->cb_malloc || !pInfo->cb_free ||
            pInfo->partition_id >= IMG_PARTITION_TOTAL_NUM ||
            !pInfo->target_phy_address )
        {
            rval = IMG_ERR_WRONG_PARAM;
            break;
        }

        // request cache buffer
        if( !(pCache = (uint8_t*)pInfo->cb_malloc(HAL_IMG_1_SECTOR_SIZE)) )
        {
            rval = IMG_ERR_NULL_POINTER;
            break;
        }

        if( (uintptr_t)pCache & 0x3 )
        {
            rval = IMG_ERR_NOT_ALIGNMENT;
            break;
        }
        memset(pCache, 0xFF, HAL_IMG_1_SECTOR_SIZE);

        //--------------------------
        // header
        if( pInfo->state & IMG_PROGRAM_STATE_IMG_HEADER )
        {
            int                     ret = 0;
            img_partition_hdr_t     *pHdr = pInfo->pPartition_hdr;
            if( !pHdr || pHdr->partition_size & (HAL_IMG_1_SECTOR_SIZE - 1) ||
                pHdr->valid_size & 0x3 )
            {
                rval = IMG_ERR_WRONG_PARAM;
                break;
            }

            switch( pInfo->partition_id )
            {
                case IMG_PARTITION_ID_IMG_ATTR:     pHdr->tag_id = HAL_IMG_UID_IMG_ATTR; break;
                case IMG_PARTITION_ID_M4:           pHdr->tag_id = HAL_IMG_UID_IMG_M4;   break;
                case IMG_PARTITION_ID_GDATA:        pHdr->tag_id = HAL_IMG_UID_GDATA;    break;
                case IMG_PARTITION_ID_M0:           pHdr->tag_id = HAL_IMG_UID_IMG_M0;   break;
                case IMG_PARTITION_ID_DSP:          pHdr->tag_id = HAL_IMG_UID_IMG_DSP;  break;
                case IMG_PARTITION_ID_CFG:          pHdr->tag_id = HAL_IMG_UID_CONFIG;   break;
                case IMG_PARTITION_ID_SYSLOG:       pHdr->tag_id = HAL_IMG_UID_SYSLOG;   break;
                case IMG_PARTITION_ID_USR1:         pHdr->tag_id = HAL_IMG_UID_USR1;     break;
                case IMG_PARTITION_ID_USR2:         pHdr->tag_id = HAL_IMG_UID_USR2;     break;
                case IMG_PARTITION_ID_USR3:         pHdr->tag_id = HAL_IMG_UID_USR3;     break;
                default:
                    rval = IMG_ERR_WRONG_PARAM;
                    break;
            }

            if( rval ) break;

            hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

            memset(pCache, 0xFF, HAL_IMG_1_SECTOR_SIZE);

            {
                int     volume_id = -1;

                rval = hal_img_get_volume_id(&volume_id);
                if( rval ) break;

                phy_addr = pInfo->target_phy_address & ~0xF0000000ul;
                phy_addr = (volume_id == pInfo->target_vol_index)
                         ? phy_addr : (phy_addr + (g_flash_space >> 1));

                phy_addr += hal_fc_get_flash_base_addr();
            }

            ret = hal_fc_write((uintptr_t)pHdr,
                               (uintptr_t)phy_addr,
                               sizeof(img_partition_hdr_t),
                               _hal_img_mmap,
                               (uint32_t*)pCache);
            if( ret )
            {
                rval = IMG_ERR_PROGRAM_FAIL;
                break;
            }
        }

        if( pInfo->state & IMG_PROGRAM_STATE_IMG_DATA )
        {
            int         ret = 0;

            hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

            memset(pCache, 0xFF, HAL_IMG_1_SECTOR_SIZE);

            {
                int     volume_id = -1;

                rval = hal_img_get_volume_id(&volume_id);
                if( rval ) break;

                phy_addr = pInfo->target_phy_address & ~0xF0000000ul;
                phy_addr = (pInfo->state & IMG_PROGRAM_STATE_IMG_HEADER)
                         ? phy_addr + sizeof(img_partition_hdr_t)
                         : phy_addr;
                phy_addr = (volume_id == pInfo->target_vol_index)
                         ? phy_addr : (phy_addr + (g_flash_space >> 1));

                phy_addr += hal_fc_get_flash_base_addr();
            }

            ret = hal_fc_write((uintptr_t)pInfo->pData,
                               (uintptr_t)phy_addr,
                               pInfo->data_len,
                               _hal_img_mmap,
                               (uint32_t*)pCache);
            if( ret )
            {
                rval = IMG_ERR_PROGRAM_FAIL;
                break;
            }
        }

        if( pInfo->state & IMG_PROGRAM_STATE_IMG_CRC )
        {
            int                     ret = 0;
            img_partition_hdr_t     *pHdr = pInfo->pPartition_hdr;
            uintptr_t               vir_addr = 0ul;
            uint32_t                crc32 = 0ul;

            if( !pHdr )
            {
                rval = IMG_ERR_WRONG_PARAM;
                break;
            }

            {
                int     volume_id = -1;

                rval = hal_img_get_volume_id(&volume_id);
                if( rval ) break;

                phy_addr = pInfo->target_phy_address & ~0xF0000000ul;

                if( pInfo->state & IMG_PROGRAM_STATE_IMG_HEADER )
                    phy_addr += sizeof(img_partition_hdr_t);

                if( pInfo->state & IMG_PROGRAM_STATE_IMG_DATA )
                    phy_addr += pInfo->data_len;

                phy_addr = (volume_id == pInfo->target_vol_index)
                         ? phy_addr : (phy_addr + (g_flash_space >> 1));

                phy_addr += hal_fc_get_flash_base_addr();
            }

            if( hal_phy2vir((uintptr_t)phy_addr, &vir_addr) )
            {
                rval = IMG_ERR_UNKNOWN_DEVICE;
                break;
            }

            crc32 = _hal_img_calc_crc32((uint8_t*)vir_addr - pHdr->valid_size, ALIGN4(pHdr->valid_size));

            hal_fc_set_protection(FC_PROTECT_LEVEL_NO_PROTECT, true);

            memset(pCache, 0xFF, HAL_IMG_1_SECTOR_SIZE);

            ret = hal_fc_write((uintptr_t)&crc32,
                               (uintptr_t)phy_addr,
                               sizeof(uint32_t),
                               _hal_img_mmap,
                               (uint32_t*)pCache);
            if( ret )
            {
                rval = IMG_ERR_PROGRAM_FAIL;
                break;
            }
        }
    } while(0);

    // set protect
    hal_fc_set_protection(FC_PROTECT_LEVEL_FULL_PROTECT, true);

    if( pCache && pInfo->cb_free ) pInfo->cb_free(pCache);
    return rval;
}

img_err_t
hal_img_verify_partition(
    int         volume_index,
    uintptr_t   partition_phy_addr)
{
    img_err_t       rval = IMG_ERR_OK;
    do {
        uintptr_t               vir_addr = 0ul;

        {
            int             volume_id = -1;
            uintptr_t       phy_addr = 0ul;

            rval = hal_img_get_volume_id(&volume_id);
            if( rval ) break;

            phy_addr = partition_phy_addr;
            phy_addr = (volume_id == volume_index)
                     ? phy_addr : (phy_addr + (g_flash_space >> 1));

            if( hal_phy2vir(phy_addr, &vir_addr) )
            {
                rval = IMG_ERR_UNKNOWN_DEVICE;
                break;
            }
        }

        {
            img_partition_hdr_t     *pPart_hdr = (img_partition_hdr_t*)vir_addr;
            uint32_t                crc32 = 0ul, *pCRC = 0ul;

            if( pPart_hdr->tag_id == HAL_IMG_UID_IMG_ATTR )
            {
                img_attr_t      *pAttr = (img_attr_t*)(pPart_hdr + 1);
                for(int j = 0; j < CONFIG_IMG_ATTR_CRC_MAX_TIMES; j++)
                {
                    if( pAttr->crc32[j] == 0x0 )
                        continue;

                    pCRC = &pAttr->crc32[j];
                    break;
                }
            }
            else
            {
                pCRC = (uint32_t*)(vir_addr + pPart_hdr->valid_size);
            }

            crc32 = _hal_img_calc_crc32((uint8_t*)pPart_hdr, ALIGN4(pPart_hdr->valid_size));
            if( crc32 != *pCRC )
            {
                rval = IMG_ERR_CRC_FAIL;
                break;
            }
        }
    } while(0);
    return rval;
}
