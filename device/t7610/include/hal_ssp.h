/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_ssp.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_ssp_H_wTmGthzz_l6Bc_H7YA_swCx_uGsX31pGcOHP__
#define __hal_ssp_H_wTmGthzz_l6Bc_H7YA_swCx_uGsX31pGcOHP__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum ssp_err
{
    SSP_ERR_OK                  = 0,
    SSP_ERR_NULL_POINTER        = (SSP_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    SSP_ERR_NO_INSTANCE         = (SSP_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    SSP_ERR_WRONG_PARAM         = (SSP_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    SSP_ERR_NOT_SUPPORT         = (SSP_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),
    SSP_ERR_NOT_SUPPORT_ROLE,
    SSP_ERR_OVER_CLOCK,
    SSP_ERR_BUSY,

} ssp_err_t;

typedef enum ssp_role
{
    SSP_ROLE_UNKNOWN     = 0,
    SSP_ROLE_MASTER,
    SSP_ROLE_SLAVE,
} ssp_role_t;

typedef enum ssp_cpol
{
    SSP_CPOL_LOW,
    SSP_CPOL_HIGH,
} ssp_cpol_t;

typedef enum ssp_cpha
{
    SSP_CPHA_1_EDGE,
    SSP_CPHA_2_EDGE,
} ssp_cpha_t;

/**
 *  the first bit type of SSP
 */
typedef enum ssp_fb
{
    SSP_FB_MSB,
    SSP_FB_LSB,
} ssp_fb_t;

/**
 *  the data size of SSP
 */
typedef enum ssp_datasize
{
    SSP_DATASIZE_8B     = 8,
    SSP_DATASIZE_16B    = 16,
    SSP_DATASIZE_32B    = 32,
} ssp_datasize_t;

/**
 *  the event type of SSP
 */
typedef enum ssp_event
{
    SSP_EVENT_TX_END    = (0x1ul << 0),
    SSP_EVENT_RX_IN     = (0x1ul << 1),

} ssp_event_t;

/**
 *  @brief  cb_ssp_filter_data_t
 *              filter received data from SSP
 *              p.s. The callback function CAN NOT be blocked, and it MUST execute as briefly as possible.
 *  @param [in] data_size       the data size of SSP, reference enum ssp_datasize
 *  @param [in] value            received data value
 *  @param [in] pTunnel_info    tunnel info from hal_ssp_irq_handle API
 *  @return
 *      valid bytes
 */
typedef uint32_t (*cb_ssp_filter_data_t)(ssp_datasize_t data_size, uint32_t value, void *pTunnel_info);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  configuration of ssp module when initialize
 */
typedef struct ssp_config
{
    uint32_t        pclk;               /*!< the PCLK value */
    uint32_t        clock_rate;         /*!< the clock (KHz) of SSP */
    ssp_role_t      role;               /*!< the role of ssp, reference enum ssp_role */
    ssp_cpol_t      cpol;               /*!< the Clock Polarity of SSP, reference enum ssp_cpol */
    ssp_cpha_t      cpha;               /*!< the Clock Phase of SSP, reference enum ssp_cpha */
    ssp_fb_t        first_bit;          /*!< data transfers start from MSB or LSB bit, reference enum ssp_fb */
    ssp_datasize_t  data_size;          /*!< the data size of SSP, reference enum ssp_datasize */
    uint32_t        chip_select;        /*!< the chip selection port */
    bool            is_loopback_mode;   /*!< is used for self-test only */
} ssp_config_t;


/**
 *  @brief cb_usr_cs (It only works at SSP_ROLE_MASTER)
 *          Callback to external chip selection function.
 *          p.s. The callback function CAN NOT be blocked, and it MUST execute as briefly as possible.
 *
 *  @param [in] is_cs_active        CS is active or not (SSP_CS_ACTIVE/SSP_CS_INACTIVE)
 *  @param [in] pUsr_info           the user info from struct ssp_data_set
 *
 *  @return
 *      error code, reference enum ssp_err
 */
#define SSP_CS_ACTIVE       1
#define SSP_CS_INACTIVE     0
typedef ssp_err_t (*cb_usr_cs_t)(uint32_t is_cs_active, void *pUsr_info);

typedef struct ssp_data_set
{
    uint32_t        *pData;         /*!< the sending data buffer */
    uint32_t        length;         /*!< data length. It should be assigned buffer length (4-alignment)
                                         and this API will report the real data length */
    // ssp_datasize_t  data_size;      /*!< the data size of SSP, reference enum ssp_datasize */
    void            *pUsr_info;     /*!< the user info */

    cb_usr_cs_t     cb_usr_cs;      /*!< external chip selection callback */

} ssp_data_set_t;

/**
 *  ssp handle
 */
typedef struct ssp_handle
{
    uint32_t        instance;   /*!< the instance of target module */
    uint16_t        port_id;    /*!< the port id of target module */
    uint16_t        role;       /*!< the port id of target module */
    IRQn_Type       irq_id;     /*!< the IRQ id of target module */

    ssp_datasize_t  data_size;  /*!< the data size of SSP, reference enum ssp_datasize */

} ssp_handle_t;
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
 *  @brief  hal_ssp_init (Non thread-safe)
 *
 *  @param [in] target_id       target SSP id (from 0, ...)
 *  @param [in] pConfig         the configuration of SSP module
 *  @param [in] ppHSSP          the SSP handler
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_init(
    uint32_t        target_id,
    ssp_config_t    *pConfig,
    ssp_handle_t    **ppHSSP);


/**
 *  @brief  hal_ssp_deinit (Non thread-safe)
 *
 *  @param [in] ppHSSP          the SSP handler
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_deinit(
    ssp_handle_t    **ppHSSP);


/**
 *  @brief  hal_ssp_send (Non thread-safe)
 *
 *  @param [in] pHSSP           the SSP handler
 *  @param [in] pData_set       the setting of sending data, reference struct ssp_data_set
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_send(
    ssp_handle_t    *pHSSP,
    ssp_data_set_t  *pData_set);


/**
 *  @brief  hal_ssp_recv (Non thread-safe)
 *
 *  @param [in] pHSSP           the SSP handler
 *  @param [in] pData_set       the setting of receiving data, reference struct ssp_data_set
 *  @return
 *      rror code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_recv(
    ssp_handle_t    *pHSSP,
    ssp_data_set_t  *pData_set);


/**
 *  @brief  hal_ssp_get_state (Non thread-safe)
 *              Get the state of SSP
 *  @param [in] pHSSP           the SSP handler
 *  @return
 *      SSP_ERR_OK  : SSP is idle
 *      SSP_ERR_BUSY: SSP is busy
 */
ssp_err_t
hal_ssp_get_state(
    ssp_handle_t    *pHSSP);


/**
 *  @brief  hal_ssp_irq_handle (Non thread-safe)
 *              The irq handle
 *  @param [in] pHSSP           the SSP handler
 *  @param [in] cb_filter_data  the callback function for filtering incoming data
 *  @param [in] pTunnel_info    the tunnel info from user
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_irq_handle(
    ssp_handle_t            *pHSSP,
    cb_ssp_filter_data_t    cb_filter_data,
    void                    *pTunnel_info) __isr_func_n;



#ifdef __cplusplus
}
#endif

#endif
