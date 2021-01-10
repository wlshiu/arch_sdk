/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_fc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_fc_H_w9TIXujm_lD8m_HBFS_saQ2_uWcoxb3HlN5u__
#define __hal_fc_H_w9TIXujm_lD8m_HBFS_saQ2_uWcoxb3HlN5u__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define FC_ONE_SECTOR_SIZE      (0x4ul << 10)

typedef enum fc_err
{
    FC_ERR_OK               = 0,
    FC_ERR_NULL_POINTER     = (FC_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    FC_ERR_NO_INSTANCE      = (FC_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    FC_ERR_WRONG_PARAM      = (FC_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    FC_ERR_NOT_SUPPORT      = (FC_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),
    FC_ERR_UNKNOWN_DEVICE   = (FC_ERRNO_BASE | HAL_ERRNO_UNKNOWN_DEVICE),
    FC_ERR_WRONG_ADDERSS,
    FC_ERR_DATA_PROTECTED,

} fc_err_t;

/**
 *  capability of Flash Control
 */
typedef enum fc_capability
{
    FC_ERASE_MODE_PAGE       = (0x1 << 0),
    FC_ERASE_MODE_SECTOR     = (0x1 << 1),
    FC_ERASE_MODE_BLOCK      = (0x1 << 2),
    FC_ERASE_MODE_CHIP       = (0x1 << 3),
    FC_3BYTES_ADDR           = (0x1 << 4),
    FC_4BYTES_ADDR           = (0x1 << 5),
    FC_1I4O_MODE             = (0x1 << 6),
    FC_4IO_MODE              = (0x1 << 7),
} fc_capability_t;

/**
 *  the divider of clock of flash
 */
typedef enum fc_clk_div
{
    FC_CLK_DIV_1    = 0x0,
    FC_CLK_DIV_2    = 0x1,
    FC_CLK_DIV_3    = 0x2,
    FC_CLK_DIV_4    = 0x3,
    FC_CLK_DIV_5    = 0x4,
    FC_CLK_DIV_6    = 0x5,
    FC_CLK_DIV_7    = 0x6,
    FC_CLK_DIV_8    = 0x7,
    FC_CLK_DIV_9    = 0x8,
    FC_CLK_DIV_10   = 0x9,
    FC_CLK_DIV_11   = 0xA,
    FC_CLK_DIV_12   = 0xB,
    FC_CLK_DIV_13   = 0xC,
    FC_CLK_DIV_14   = 0xD,
    FC_CLK_DIV_15   = 0xE,
    FC_CLK_DIV_16   = 0xF,
} fc_clk_div_t;

typedef enum fc_protect_level
{
    FC_PROTECT_LEVEL_NO_PROTECT  = 0,
    FC_PROTECT_LEVEL_1           = 1,
    FC_PROTECT_LEVEL_2           = 2,
    FC_PROTECT_LEVEL_3           = 3,
    FC_PROTECT_LEVEL_4           = 4,
    FC_PROTECT_LEVEL_5           = 5,
    FC_PROTECT_LEVEL_6           = 6,
    FC_PROTECT_LEVEL_7           = 7,
    FC_PROTECT_LEVEL_8           = 8,
    FC_PROTECT_LEVEL_FULL_PROTECT = 9,
    FC_PROTECT_LEVEL_ALL,
} fc_protect_level_t;

/**
 *  Security Registers of a flash
 */
typedef enum fc_sec_reg
{
    FC_SEC_REG_0    = 0,
    FC_SEC_REG_1    = 1,
    FC_SEC_REG_2    = 2,
} fc_sec_reg_t;

/**
 *  the operation code of Security Registers of a flash
 */
typedef enum fc_sec_reg_opcode
{
    FC_SEC_REG_OPCODE_READ,
    FC_SEC_REG_OPCODE_WRITE,
    FC_SEC_REG_OPCODE_ERASE
} fc_sec_reg_opcode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct fc_attr
{
    uint32_t    space_bytes;
    uint32_t    protection_max_level;
    uint32_t    one_block_bytes;
    uint32_t    one_protected_unit_bytes;

} fc_attr_t;

typedef struct fc_sec_reg_ctrl
{
    fc_sec_reg_opcode_t     opcode;         /*!< operation code, reference enum fc_sec_reg_opcode */
    fc_sec_reg_t            idx;            /*!< the index of Security Registers */
    uint32_t                offset;         /*!< the offset bytes of a Security Register (It must be 4-bytes alignment) */
    uintptr_t               buf_addr;       /*!< buffer address (in system r/w memory) */
    int                     buf_bytes;      /*!< the buffer size.
                                                 It should be assigned buffer length
                                                 and this API will report the real data length */
} fc_sec_reg_ctrl_t;
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
 *  @brief  hal_fc_init (Non thread-safe)
 *
 *  @param [in] divider     the divider of a flash chip, reference enum fc_clk_div
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_init(
    fc_clk_div_t    divider) __reloc;


/**
 *  @brief  hal_fc_deinit (Non thread-safe)
 *
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_deinit(void) __reloc;


/**
 *  @brief  hal_fc_erase (Non thread-safe)
 *              flash erase
 *              ps. this API will block program flow (H/W limitation)
 *
 *  @param [in] mode            erase mode, reference erase mode of enum fc_capability
 *  @param [in] flash_addr      the memory mapping address of a flash
 *  @param [in] count           count of page/sector/block (depend on erase mode)
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_erase(
    fc_capability_t     mode,
    uintptr_t           flash_vir_addr,
    int                 count) __reloc;

fc_err_t
hal_fc_erase_phy(
    fc_capability_t     mode,
    uintptr_t           flash_phy_addr,
    int                 count) __reloc;

/**
 *  @brief  hal_fc_read (Non thread-safe)
 *              read data from flash
 *
 *  @param [in] buf_addr        buffer address (in system memory)
 *  @param [in] flash_addr      the memory mapping address of a flash
 *  @param [in] length          sequence data size (bytes and 4-alignment)
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_read(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         length) __reloc;

fc_err_t
hal_fc_read_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         length) __reloc;


/**
 *  @brief  hal_fc_write (Non thread-safe)
 *              write data to flash
 *              ps. this API will block program flow (H/W limitation) and increase P/E cycles
 *
 *  @param [in] buf_addr        buffer address (in system r/w memory)
 *  @param [in] flash_phy_addr  the memory mapping address of a flash
 *  @param [in] length          sequence data size (bytes and 4-alignment)
 *  @param [in] cb_mmap         the callback for memory mapping if necessary
 *                              - this callback return value:
 *                                  0    : ok
 *                                  other: fail
 *  @param [in] pCache_buf      The cache buffer (in system memory)
 *                              ps. It MUST be 4-alignment and more than FC_ONE_SECTOR_SIZE
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_write(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         length,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr),
    uint32_t    *pCache_buf) __reloc;

fc_err_t
hal_fc_write_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         length,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr),
    uint32_t    *pCache_buf) __reloc;

/**
 *  @brief  hal_fc_write_raw (Non thread-safe)
 *
 *  @param [in] buf_addr        buffer address (in system r/w memory)
 *  @param [in] flash_phy_addr  the memory mapping address of a flash
 *  @param [in] bytes           sequence data size (bytes and 4-alignment)
 *  @param [in] cb_mmap         the callback for memory mapping if necessary
 *                              - this callback return value:
 *                                  0    : ok
 *                                  other: fail
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_write_raw(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         bytes,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr)) __reloc;

fc_err_t
hal_fc_write_raw_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         bytes,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr)) __reloc;

/**
 *  @brief  hal_fc_get_sr (Non thread-safe)
 *
 *  @return
 *      the value (1 byte) of status register
 */
uint8_t
hal_fc_get_sr(void) __reloc;


/**
 *  @brief  hal_fc_get_sr1 (Non thread-safe)
 *
 *  @return
 *      the value (1 byte) of status register 1
 */
uint8_t
hal_fc_get_sr1(void) __reloc;


/**
 *  @brief  hal_fc_set_protection (Non thread-safe)
 *
 *  @param [in] level                   the protection level of the flash, reference enum fc_protect_level
 *  @param [in] has_use_volatile        use volatile register of the flash or not
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_set_protection(
    fc_protect_level_t      level,
    uint32_t                has_use_volatile) __reloc;


/**
 *  @brief  hal_fc_get_flash_base_addr (Non thread-safe)
 *
 *  @return
 *      the memory base value
 */
uintptr_t
hal_fc_get_flash_base_addr(void) __reloc;


/**
 *  @brief  hal_fc_get_flash_space (Non thread-safe)
 *
 *  @param [in] pFc_attr        the attribute of the flash, reference struct fc_attr
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_get_flash_attr(
    fc_attr_t   *pFc_attr) __reloc;


/**
 *  @brief  hal_fc_ctrl_sec_reg (Non thread-safe)
 *
 *  @param [in] pCtrl           the controlling configuration of Security Registers
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_ctrl_sec_reg(
    fc_sec_reg_ctrl_t   *pCtrl) __reloc;

/**
 *  @brief  hal_fc_get_unique_id_get (Non thread-safe)
 *
 *  @param [in] puid           the pointer of the Unique ID.
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_get_unique_id_get(
    uint8_t *puid) __reloc;

#ifdef __cplusplus
}
#endif

#endif
