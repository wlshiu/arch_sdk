/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_dma.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_dma_H_wGnOMlCV_lQCh_HGgR_sZUH_uOxdSucn1mQF__
#define __hal_dma_H_wGnOMlCV_lQCh_HGgR_sZUH_uOxdSucn1mQF__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum dma_err
{
    DMA_ERR_UNKNOWN              = -4,
    DMA_ERR_CHANNEL_ALLOC_FAIL   = -3,
    DMA_ERR_TIMEOUT              = -2,
    DMA_ERR_LENGTH_TOO_LONG      = -1,
    DMA_ERR_OK                   = 0,
} dma_err_t;

typedef enum dma_channel
{
	DMA_CHANNEL_0 = 0,
	DMA_CHANNEL_1,
	DMA_CHANNEL_2,
	DMA_CHANNEL_3,
	DMA_CHANNEL_4,
	DMA_CHANNEL_5,
	DMA_CHANNEL_6,
	DMA_CHANNEL_7,
	DMA_CHANNEL_MAX,
} dma_channel_t;

typedef void (*dma_irq_cb)(dma_channel_t channel);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct dma_config
{
    uint8_t        fix_src_enable      :1;    // fix source address to be ADDRSRC
    uint8_t        fix_dst_enable      :1;    // fix destination address to be ADDRDES
    uint8_t        handshake_enable    :1;    // apply handshake mode
    uint8_t        channel_alloc_enable:1;
    uint8_t        channel_id          :4;
    dma_irq_cb     irq_cb;
} dma_config_t;

/**
 *  dma handle
 */
typedef struct dma_handle
{
    uint32_t    instance;   /*!< the instance of target module */
    uint16_t    channel_id; /*!< the channel id of dma module */
    IRQn_Type   irq_id;     /*!< the IRQ id of target module */
} dma_handle_t;
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
 *  @brief  hal_dma_init (Non thread-safe)
 *
 *  @param [in] pConfig     the configuration of dma module
 *  @param [in] ppHdma     the dma handler
 *  @return
 *      error number, reference enum dma_err
 */
dma_err_t
hal_dma_init(
    dma_config_t   *pConfig,
    dma_handle_t   **ppHdma);

/**
 *  @brief  hal_dma_deinit (Non thread-safe)
 *
 *  @param [in] ppHdma         the dma handler
 *  @return
 *      error number, reference enum dma_err
 */
dma_err_t
hal_dma_deinit(dma_handle_t **ppHdma);

/**
 *  @brief  hal_dma_memcpy (Non thread-safe)
 *
 *  @param [in] pHdma      the dma handler
 *  @param [in] pDst       the sending destination address
 *  @param [in] pSrc       the sending source address
 *  @param [in] length     data length of sending
 *  @return
 *      error number, reference enum dma_err
 */
dma_err_t
hal_dma_memcpy(
    dma_handle_t   *pHdma,
    void           *pDst,
    void           *pSrc,
    uint32_t       length);
#ifdef __cplusplus
}
#endif

#endif
