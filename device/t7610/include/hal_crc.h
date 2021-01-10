/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_crc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_crc_H_wZaXdYys_lD50_Hbhn_sP9g_ujHoQrMwYt6m__
#define __hal_crc_H_wZaXdYys_lD50_Hbhn_sP9g_ujHoQrMwYt6m__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum crc_type
{
    CRC_TYPE_NONE   = 0,
    CRC_TYPE_5      = 4,
    CRC_TYPE_8      = 7,
    CRC_TYPE_16     = 15,
    CRC_TYPE_24     = 23,
    CRC_TYPE_32     = 31,
} crc_type_t;

typedef enum crc_err
{
    CRC_ERR_OK               = 0,
    CRC_ERR_TYPE,
    CRC_ERR_TYPE_NOT_SUPPORT,
    CRC_ERR_TIMEOUT,
    CRC_ERR_LENGTH,
    CRC_ERR_UNKNOWN,
} crc_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
typedef void (*crc_dma_irq_cb)(void);
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  @brief  hal_crc_init (Non thread-safe)
 *
 *  @param [in] type     crc type
 *  @return
 *      error number, reference enum crc_err
 */
crc_err_t
hal_crc_init(crc_type_t type);

/**
 *  @brief  hal_crc_deinit (Non thread-safe)
 *
 *  @return
 *      error number, reference enum crc_err
 */
crc_err_t
hal_crc_deinit(void);

/**
 *  @brief  hal_crc_calculation (Non thread-safe)
 *
 *  @param [in] pData         input data which is calculated
 *  @param [in] bit_length    the bit length of input data
 *  @param [in] pOutputData   output data which has been calculated
 *  @return
 *      error number, reference enum crc_err
 */
crc_err_t
hal_crc_calculation(
    uint8_t     *pData,
    uint32_t    bit_length,
    uint32_t    *pOutputData);

/**
 *  @brief  hal_crc_calculation (Non thread-safe)
 *
 *  @param [in] pData         input data which is calculated
 *  @param [in] bit_length    the bit length of input data
 *  @param [in] irq_cb        dma callback function
 *  @return
 *      error number, reference enum crc_err
 */
crc_err_t
hal_crc_calculation_by_dma(
    uint8_t *pData,
    uint32_t bit_length,
    crc_dma_irq_cb irq_cb);

/**
 *  @brief  hal_crc_calculation (Non thread-safe)
 *
 *  @param [in] pOutputData   output data which has been calculated
 *  @return
 *      error number, reference enum crc_err
 */
crc_err_t hal_crc_get_output_by_dma(uint32_t *pOutputData);

#ifdef __cplusplus
}
#endif

#endif
