/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_img.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/06
 * @license
 * @description
 */

#ifndef __hal_img_H_wMC7A5t5_lir4_H4bu_s98B_uThkddbyQNX9__
#define __hal_img_H_wMC7A5t5_lir4_H4bu_s98B_uThkddbyQNX9__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum img_err
{
    IMG_ERR_OK             = 0,
    IMG_ERR_NULL_POINTER   = (IMG_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    IMG_ERR_NO_INSTANCE    = (IMG_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    IMG_ERR_WRONG_PARAM    = (IMG_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    IMG_ERR_NOT_SUPPORT    = (IMG_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),
    IMG_ERR_UNKNOWN_DEVICE = (IMG_ERRNO_BASE | HAL_ERRNO_UNKNOWN_DEVICE),
    IMG_ERR_PROGRAM_FAIL,
    IMG_ERR_NOT_ALIGNMENT,
    IMG_ERR_OVERLAP,
    IMG_ERR_OUT_RANGE,
    IMG_ERR_INVALID_PARTITION,
    IMG_ERR_CRC_FAIL,

} img_err_t;


/**
 *  the image attribute flags of a image volume
 */
typedef enum img_attr_flag
{
    IMG_ATTR_NONE       = 0,
    IMG_ATTR_BOOTABLE,
    IMG_ATTR_PRETEST,
    IMG_ATTR_PRIORITY,
} img_attr_flag_t;

/**
 *  image partition type
 */
typedef enum img_partition_id
{
    IMG_PARTITION_ID_IMG_ATTR = 0,
    IMG_PARTITION_ID_M4,
    IMG_PARTITION_ID_GDATA,

    // optional
    IMG_PARTITION_ID_M0,
    IMG_PARTITION_ID_DSP,
    IMG_PARTITION_ID_CFG,
    IMG_PARTITION_ID_SYSLOG,
    IMG_PARTITION_ID_USR1,
    IMG_PARTITION_ID_USR2,
    IMG_PARTITION_ID_USR3,

    IMG_PARTITION_TOTAL_NUM,

} img_partition_id_t;

typedef enum img_program_state
{
    IMG_PROGRAM_STATE_IDLE       = 0,
    IMG_PROGRAM_STATE_IMG_HEADER = (0x1 << 0),
    IMG_PROGRAM_STATE_IMG_DATA   = (0x1 << 1),
    IMG_PROGRAM_STATE_IMG_CRC    = (0x1 << 2),
} img_program_state_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define HAL_IMG_UID_IMG_ATTR            FOUR_CC('a', 't', 't', 'r')
#define HAL_IMG_UID_IMG_M4              FOUR_CC('m', '4', 'b', 'n')
#define HAL_IMG_UID_IMG_M0              FOUR_CC('m', '0', 'b', 'n')
#define HAL_IMG_UID_IMG_DSP             FOUR_CC('d', 's', 'p', 'b')

#define HAL_IMG_UID_GDATA               FOUR_CC('g', 'd', 'a', 't')

#define HAL_IMG_UID_CONFIG              FOUR_CC('c', 'n', 'f', 'g')
#define HAL_IMG_UID_SYSLOG              FOUR_CC('s', 'l', 'o', 'g')
#define HAL_IMG_UID_USR1                FOUR_CC('u', 's', 'r', '1')
#define HAL_IMG_UID_USR2                FOUR_CC('u', 's', 'r', '2')
#define HAL_IMG_UID_USR3                FOUR_CC('u', 's', 'r', '3')
//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  image partition header
 */
typedef struct img_partition_hdr
{
    uint32_t    tag_id;
    uint16_t    has_crc;
    uint16_t    payload_offset;
    uint32_t    valid_size;
    uint32_t    partition_size; // involve self and payload

} img_partition_hdr_t;

/**
 *  memory address
 */
typedef struct cross_addr
{
    union {
        uintptr_t       addr_value;
        uint64_t        reserved;
    };
} cross_addr_t;

/**
 *  the header of an image partition
 */
typedef struct img_partition
{
    uint32_t        uid;
    cross_addr_t    partition_start;
    uint32_t        partition_len;
    cross_addr_t    raw_addr;
    uint32_t        raw_len;

} img_partition_t;

/**
 *  the image global data
 */
typedef struct img_gdata
{
#define CONFIG_IMG_PARTITION_NUM_PER_VOLUME         16
    uint32_t                reserved;
    img_partition_t         part_entries[CONFIG_IMG_PARTITION_NUM_PER_VOLUME];

    uint32_t                reserved1[(0x3 << 10) >> 2];

} img_gdata_t;

/**
 *  the writing partition info
 */
typedef struct img_wr_partition
{
    uint32_t                target_vol_index    : 1;   /*!< the target volume index */
    uint32_t                has_attach_crc      : 1;   /*!< attach crc to the tail or not */
    uint32_t                target_phy_address  : 21;  /*!< the target relative address of the flash physical in an image volume. */
    img_partition_id_t      partition_id        : 5;
    img_program_state_t     state               : 4;   /*!< the state of writing partition */

    img_partition_hdr_t     *pPartition_hdr;
    uint8_t                 *pData;         /*!< the writing data */
    int                     data_len;       /*!< the length of writing data */

    /**
     *  memory access for internal using
     */
    uint32_t*  (*cb_malloc)(int length);
    void       (*cb_free)(void *p);

} img_wr_partition_t;

/**
 *  the writing raw data info
 */
typedef struct img_wr_info
{
    uint32_t        target_vol_index;   /*!< the target volume index */
    uintptr_t       phy_dest_address;   /*!< the physical (relative) destination address of the flash in an image volume */
    uint8_t         *pData;             /*!< the writing data */
    int             data_len;           /*!< the length of writing data */

    /**
     *  memory access for internal using
     */
    uint32_t*  (*cb_malloc)(int length);
    void       (*cb_free)(void *p);

} img_wr_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  @brief  hal_img_get_volume_id (Non thread-safe)
 *
 *  @param [in] pVol_index          report the current image volume index
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_get_volume_id(int *pVol_index);


/**
 *  @brief  hal_img_get_gdata (Non thread-safe)
 *
 *  @param [in] volume_index        the target volume index
 *  @param [in] ppGData             report the pointer of the GData partition
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_get_gdata(
    int             volume_index,
    img_gdata_t     **ppGData);


/**
 *  @brief  hal_img_clear_attr (Non thread-safe)
 *
 *  @param [in] vol_index           the target image volume index
 *  @param [in] attr_flag           the target attribute flag, reference enum img_attr_flag
 *  @param [in] cb_malloc           the callback of memory allocation, the address MUST be 32-bits alignment
 *  @param [in] cb_free             the callback of memory release
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_clear_attr(
    uint32_t            vol_index,
    img_attr_flag_t     attr_flag,
    uint32_t*  (*cb_malloc)(int length),
    void       (*cb_free)(void *p));


/**
 *  @brief  hal_img_write_raw (Non thread-safe)
 *              This API will directly program data to flash
 *              and it also increases P/E cycles.
 *
 *  @param [in] pInfo               the writing info and this info MUST be kept for the next data slice handling,
 *                                  reference struct hal_img_wr_info
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_write_raw(
    img_wr_info_t   *pInfo);


/**
 *  @brief  hal_img_wr_partition (Non thread-safe)
 *              This API will sequentially program a partition data to flash
 *              and it also increases P/E cycles.
 *
 *  @param [in] pInfo
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_wr_partition(
    img_wr_partition_t  *pInfo);


/**
 *  @brief  hal_img_verify_partition (Non thread-safe)
 *
 *  @param [in] volume_index                the target volume index
 *  @param [in] partition_phy_addr          the physical address of a partition
 *  @return
 *      error number, reference enum img_err
 */
img_err_t
hal_img_verify_partition(
    int         volume_index,
    uintptr_t   partition_phy_addr);


#ifdef __cplusplus
}
#endif

#endif
