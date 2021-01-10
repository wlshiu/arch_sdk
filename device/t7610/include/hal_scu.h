/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_scu.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/13
 * @license
 * @description
 */

#ifndef __hal_scu_H_wSTr7hSv_ll0o_HGXE_sTL2_u1lEBG5nZZHB__
#define __hal_scu_H_wSTr7hSv_ll0o_HGXE_sTL2_u1lEBG5nZZHB__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum scu_err
{
    SCU_ERR_OK                  = 0,
    SCU_ERR_NULL_POINTER        = (SCU_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    SCU_ERR_NO_INSTANCE         = (SCU_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    SCU_ERR_WRONG_PARAM         = (SCU_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    SCU_ERR_NOT_SUPPORT         = (SCU_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),
    SCU_ERR_NOT_ALIGNMENT,

} scu_err_t;

/**
 *  the memory mapping type
 */
typedef enum scu_mmp_type
{
    SCU_MMP_TYPE_PROM   = 1,
    SCU_MMP_TYPE_SRAM   = 2,
    SCU_MMP_TYPE_FLASH  = 0,

} scu_mmp_type_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
#if defined(CONFIG_CHIP_ALIAS_PHOENIX)
/**
 *  reset flag
 */
typedef enum scu_rst_flag
{
    SCU_RST_CPU_CM4               = (0x1ul << 0 ),
    SCU_RST_USER_BRESET           = (0x1ul << 1 ),
    SCU_RST_AHB2ERAM_HRST         = (0x1ul << 2 ),
    SCU_RST_AHB2ERAM_1_HRST       = (0x1ul << 3 ),
    SCU_RST_AHB2ERAM_ROM_HRST     = (0x1ul << 4 ),
    SCU_RST_ATE_SPI_FLASH_HRST    = (0x1ul << 5 ),
    SCU_RST_BP010_AHB_7X9_HRST    = (0x1ul << 6 ),
    SCU_RST_DMAC_HRST             = (0x1ul << 7 ),
    SCU_RST_H2P_HRST              = (0x1ul << 8 ),
    SCU_RST_GMAC_HRST             = (0x1ul << 9 ),
    SCU_RST_USER_IP_TOP_HRST      = (0x1ul << 10),
    SCU_RST_EFUSE_CTRL_PRST       = (0x1ul << 11),
    SCU_RST_H2P_PRST              = (0x1ul << 12),
    SCU_RST_GPIO_PRST             = (0x1ul << 13),
    SCU_RST_I2C_PRST              = (0x1ul << 14),
    SCU_RST_PWMTMR_PRST           = (0x1ul << 15),
    SCU_RST_UART_PRST             = (0x1ul << 16),
    SCU_RST_UART_1_PRST           = (0x1ul << 17),
    SCU_RST_UART_2_PRST           = (0x1ul << 18),
    SCU_RST_UART_3_PRST           = (0x1ul << 19),
    SCU_RST_UART_4_PRST           = (0x1ul << 20),
    SCU_RST_UART_5_PRST           = (0x1ul << 21),
    SCU_RST_UART_6_PRST           = (0x1ul << 22),
    SCU_RST_SSP_SPI_PRST          = (0x1ul << 23),
    SCU_RST_SSP_SSP1_PRST         = (0x1ul << 24),
    SCU_RST_SSP_SSP2_PRST         = (0x1ul << 25),
    SCU_RST_WDT_PRST              = (0x1ul << 26),
} scu_rst_flag_t;

/**
 *  bootstrap control
 */
typedef struct scu_sw_strap
{
    volatile uint32_t                  : 8;
    volatile uint32_t   remap_type     : 2;     /*!< boot mode */
    volatile uint32_t   core_type      : 2;     /*!< CPU/DSP jTag selection */
    volatile uint32_t   debug_mode_en  : 1;     /*!< debug mode enable or not */
    volatile uint32_t   sys_clk_src    : 1;     /*!< PLL bypass mode */
    volatile uint32_t   clk_speed_type : 1;     /*!< system clock selection */
    volatile uint32_t   sw_strap_type  : 1;     /*!< Software applicaiotn according to H/w or S/w reset */

} scu_sw_strap_t;
#endif

typedef struct scu_call
{
    uintptr_t       reg;
    unsigned long   value;
    unsigned long   mask;
} scu_call_t;

/**
 *  scu handle
 */
typedef struct scuex_handle
{
    uint32_t    instance;       /*!< the instance of target module */
    uint32_t    instance_ex;    /*!< the extension instance of target module */

} scuex_handle_t;
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
 *  @brief  hal_scu_init (Non thread-safe)
 *              system control unit initialize
 *
 *  @param [in] ppHScu          the scu handler
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_init(
    scuex_handle_t  **ppHScu);


/**
 *  @brief  hal_scu_exec_script (Non thread-safe)
 *
 *  @param [in] pCalls              the script with struct scu_call format
 *  @param [in] call_array_size     the array size of the script, the cardinal number is sizeof(scu_call_t)
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_exec_script(
    scu_call_t  *pCalls,
    uint32_t    call_array_size);


/**
 *  @brief  hal_scu_get_mmp_type (Non thread-safe)
 *
 *  @param [in] pType               the current memory mapping type
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_get_mmp_type(
    uint32_t    *pType);


/**
 *  @brief  hal_scu_set_mmp_type (Non thread-safe)
 *
 *  @param [in] type                memory mapping type
 *                                      0: Flash = 0x00000000, SRAM0 = 0x20000000, SRAM1 = 0x30000000
 *                                      1: Flash = 0x60000000, SRAM0 = 0x20000000, SRAM1 = 0x30000000
 *                                      2: Flash = 0x60000000, SRAM0 = 0x20000000, SRAM1 = 0x00000000
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_set_mmp_type(
    uint32_t    type);


/**
 *  @brief  hal_scu_set_boot_offset (Non thread-safe)
 *
 *  @param [in] offset
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_set_boot_offset(
    uint32_t    offset);


/**
 *  @brief  hal_scu_get_boot_offset (Non thread-safe)
 *
 *  @param [in] pOffset             return the booting offset of flash
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_get_boot_offset(
    uint32_t    *pOffset);


/**
 *  @brief  hal_scu_reboot (Non thread-safe)
 *              Trigger system reboot right now.
 *
 *  @param [in] is_boot_record      enable boot recording or not
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_reboot(
    uint32_t    is_boot_record);


#if defined(CONFIG_CHIP_ALIAS_PHOENIX)
/**
 *  @brief  hal_scu_set_hw_reset (Non thread-safe)
 *
 *  @param [in] flags               module flags, reference 'enum scu_rst_flag'
 *  @param [in] is_enable           enable reset or not
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_set_hw_reset(
    uint32_t    flags,
    bool        is_enable);


/**
 *  @brief  hal_scu_set_sw_strap (Non thread-safe)
 *
 *  @param [in] pSw_strap           strap value, reference 'struct scu_sw_strap'
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_set_sw_strap(
    scu_sw_strap_t  *pSw_strap);


/**
 *  @brief  hal_scu_get_sw_strap (Non thread-safe)
 *
 *  @param [in] pSw_strap           strap value, reference 'struct scu_sw_strap'
 *  @return
 *      error number, reference enum scu_err
 */
scu_err_t
hal_scu_get_sw_strap(
    scu_sw_strap_t  *pSw_strap);

#endif


#ifdef __cplusplus
}
#endif

#endif
