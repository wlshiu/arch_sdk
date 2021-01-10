/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_fc.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include <string.h>
#include "hal_fc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of flash controller
 */
#define FC_CSR_BASE             0x60FFF000ul
#define REG_FLASH_MAP_CFG       0x60FFF020ul
#define SCU_CSR_BASE            0x54000000ul

#define FC_MMP_BASE             0x60000000ul
#define FC_GD_DELAY_TICKS       150

#define FC_PAGE_SIZE_POW_2      8

#define FC_SIZE_512KB           0x0
#define FC_SIZE_1MB             0x1
#define FC_SIZE_2MB             0x2
#define FC_SIZE_4MB             0x3
#define FC_SIZE_8MB             0x4
#define FC_SIZE_16MB            0x5
#define FC_SIZE_32MB            0x6
#define FC_SIZE_64MB            0x7

#define MSK_FC_SR1_BUSY         (0x1ul << 0)
#define MSK_FC_SR1_WEL          (0x1ul << 1)
#define MSK_FC_SR1_BP0          (0x1ul << 2)
#define MSK_FC_SR1_BP1          (0x1ul << 3)
#define MSK_FC_SR1_BP2          (0x1ul << 4)
#define MSK_FC_SR1_TB           (0x1ul << 5)
#define MSK_FC_SR1_SEC          (0x1ul << 6)
#define MSK_FC_SR1_SRP          (0x1ul << 7)

#define MSK_FC_SR2_SRL          (0x1ul << 0)
#define MSK_FC_SR2_QE           (0x1ul << 1)
#define MSK_FC_SR2_LB0          (0x1ul << 2)
#define MSK_FC_SR2_LB1          (0x1ul << 3)
#define MSK_FC_SR2_LB2          (0x1ul << 4)
#define MSK_FC_SR2_LB3          (0x1ul << 5)
#define MSK_FC_SR2_CMP          (0x1ul << 6)
#define MSK_FC_SR2_SUS          (0x1ul << 7)

#define MSK_FC_SR1              0x000000FFul
#define MSK_FC_SR2              0x000000FFul
#define MSK_FC_CACHEHIT         0x000000FFul
#define MSK_FC_FACTORY_ID       0x00FFFFFFul
#define MSK_FC_SR_BUSY          MSK_FC_SR1_BUSY
#define MSK_FC_SR_WR_ENABLE     MSK_FC_SR1_WEL

#define FC_IOMODE_1I1O          0x0
#define FC_IOMODE_1I1OF         0x1
#define FC_IOMODE_1I2O          0x2
#define FC_IOMODE_2IO           0x3
#define FC_IOMODE_1I4O          0x4
#define FC_IOMODE_4IO           0x5

#define FC_MEMMODE_1I1O         0x0
#define FC_MEMMODE_1I1Of        0x1
#define FC_MEMMODE_1I2O         0x2
#define FC_MEMMODE_2IO          0x3
#define FC_MEMMODE_1I4O         0x4
#define FC_MEMMODE_4IO          0x5

#define FC_LAT_FALLING_EDGE     0x0
#define FC_LAT_0_5_CYCLE        0x1
#define FC_LAT_1_0_CYCLE        0x2
#define FC_LAT_1_5_CYCLE        0x3

#define FC_PERFENHC_MODE_DIS    0x0
#define FC_PERFENHC_MODE_EN     0x1

#define FC_MEMADDR4_3BYTES      0x0
#define FC_MEMADDR4_4BYTES      0x1

#define FC_CSHTime_1Cycle       0x0
#define FC_CSHTime_2Cycle       0x1
#define FC_CSHTime_3Cycle       0x2
#define FC_CSHTime_4Cycle       0x3
#define FC_CSHTime_5Cycle       0x4
#define FC_CSHTime_6Cycle       0x5
#define FC_CSHTime_7Cycle       0x6
#define FC_CSHTime_8Cycle       0x7
#define FC_CSHTime_9Cycle       0x8
#define FC_CSHTime_10Cycle      0x9
#define FC_CSHTime_11Cycle      0xA
#define FC_CSHTime_12Cycle      0xB
#define FC_CSHTime_13Cycle      0xC
#define FC_CSHTime_14Cycle      0xD
#define FC_CSHTime_15Cycle      0xE
#define FC_CSHTime_16Cycle      0xF

#define FC_CMD_ID(cmd)              ((cmd) & 0xFF)
#define FC_CMD_DATA_LENGTH(len)     ((((len) - 1) & 0xFF) << 16)
#define FC_CMD_DATA_EN              (0x1 << 8)
#define FC_CMD_WR_MODE_EN           (0x1 << 9)
#define FC_CMD_ADDR_EN              (0x1 << 10)
#define FC_CMD_4BYTES_ADDR_EN       (0x1 << 11)
#define FC_CMD_IO_MODE(io_mode)     (((io_mode) & 0x7) << 12)
#define FC_CMD_BLOCKING_EN          (0x1 << 24)
#define FC_CMD_LAUNCH               (0x1 << 31)

#define FC_CMD_READ             0x00
#define FC_CMD_WRITE            0x01
#define FC_CMD_NO_ADDR          0x00
#define FC_CMD_WITH_ADDR        0x01
#define FC_CMD_NO_DATA          0x00
#define FC_CMD_WITH_DATA        0x01
#define FC_CMD_ADDR_3BYTES      0x0
#define FC_CMD_ADDR_4BYTES      0x1
#define FC_CMD_MODE_1I1O        0x0
#define FC_CMD_MODE_1I1Of       0x1
#define FC_CMD_MODE_1I2O        0x2
#define FC_CMD_MODE_2IO         0x3
#define FC_CMD_MODE_1I4O        0x4
#define FC_CMD_MODE_4IO         0x5

#define FC_CACHE_ENABLE         (0x1ul << 0)
#define FC_CACHE_SIZE_4K        (0x1ul << 1)
#define FC_CACHE_HIT_ENABLE     (0x1ul << 2)

#define FC_DEV_WINBOND          0x0
#define FC_DEV_MXIC             0x1

/**
 *  manufacturer id
 */
#define FC_MANUFACTURER_ID_WINBOND      0x00EF0000ul
#define FC_MANUFACTURER_ID_GD           0x00C80000ul
#define FC_MANUFACTURER_ID_MXIC         0x00C20000ul


/**
 *  manufacturer features
 */
#define MSK_FC_WINBOND_PROTECTION       (0x7 << 2)
#define FC_WINBOND_PROTECTION(level)    (((level) == FC_PROTECT_LEVEL_FULL_PROTECT) ? MSK_FC_WINBOND_PROTECTION : (((level) & 0x7) << 2))


#define MSK_FC_GD_PROTECTION            (0x7 << 2)
#define FC_GD_PROTECTION(level)         (((level) == FC_PROTECT_LEVEL_FULL_PROTECT) ? MSK_FC_GD_PROTECTION : (((level) & 0x7) << 2))

#define FC_QUAD_EN                      (0x1ul << 9)

/**
 *  the space size of a flash
 */
#define QSPI_SPACE_512KBITS       (512ul << 10)
#define QSPI_SPACE_1MBITS         (1ul << 20)
#define QSPI_SPACE_2MBITS         (2ul << 20)
#define QSPI_SPACE_4MBITS         (4ul << 20)
#define QSPI_SPACE_8MBITS         (8ul << 20)
#define QSPI_SPACE_16MBITS        (16ul << 20)
#define QSPI_SPACE_32MBITS        (32ul << 20)
#define QSPI_SPACE_64MBITS        (64ul << 20)

/**
 *  Instruction Set of a flash
 */
// Erase Operations
#define QSPI_CMD_SECTOR_ERASE           0x20
#define QSPI_CMD_BLOCK_ERASE_32K        0x52
#define QSPI_CMD_BLOCK_ERASE_64K        0xD8
#define QSPI_CMD_CHIP_ERASE             0xC7

// Write Operations
#define QSPI_CMD_WRITE_ENABLE           0x06
#define QSPI_CMD_WRITE_DISABLE          0x04

// Status Register Operations
#define QSPI_CMD_READ_STATUS_REG        0x05
#define QSPI_CMD_READ_EX_STATUS_REG     0x35    // GD= SR1, Winbond= SR2
#define QSPI_CMD_WRITE_STATUS_REG       0x01
#define QSPI_CMD_VOLATILE_SR_EN         0x50
#define QSPI_CMD_WINBOND_WR_SR2         0x31

// Quad Operations
#define QSPI_CMD_ENTER_QUAD             0x35

// Program Operations
#define QSPI_CMD_PAGE_PROG              0x02
#define QSPI_CMD_QUAD_IN_FAST_PROG      0x32
#define QSPI_CMD_EXT_QUAD_IN_FAST_PROG  0x12

// the security register  commands
#define QSPI_CMD_SEC_REG_PROG           0x42
#define QSPI_CMD_SEC_REG_ERASE          0x44
#define QSPI_CMD_SEC_REG_READ           0x48

//=============================================================================
//                  Macro Definition
//=============================================================================
#define GET_REMAP_TYPE()                ((*((volatile uint32_t*)(SCU_CSR_BASE + 0x20)) >> 24) & 0xF)

#define IS_FC_IN_STATUS(pReg, mask)     ((pReg)->SR & (mask))
#define IS_FC_HW_BUSY(pReg)             ((pReg)->CMD & FC_CMD_LAUNCH)

#define FC_CACHE_START(pReg)            ((pReg)->CACHE |= FC_CACHE_ENABLE)
#define FC_CACHE_STOP(pReg)             ((pReg)->CACHE &= ~FC_CACHE_ENABLE)

#define FC_DELAY(timeout)               do { for(int __k = 0; __k < (timeout); __k++); }while(0)

/**
 *  @brief GET_FLASH_BOOT_START_POS()
 *              Get the BootStartPos in SPI Flash
 *  @return     physical position in SPI Flash
 *
 */
#define GET_FLASH_BOOT_START_POS()      ((*((uint32_t*)REG_FLASH_MAP_CFG)) << 4)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct fc_csr_cmd
{
    union {
        struct {
            volatile uint32_t CMDID    : 8;   /**< flash Instruction cmd  */
            volatile uint32_t CMDDATA  : 1;   /**< with or without data   */
            volatile uint32_t CMDWR    : 1;   /**< read (0) or write (1) mode control */
            volatile uint32_t CMDADDR  : 1;   /**< with or without address    */
            volatile uint32_t CMDADDR4 : 1;   /**< 3 or 4 bytes address mode  */
            volatile uint32_t CMDMODE  : 3;   /**< 0: 1-IO mode, 5: 4-IO mode */
            volatile uint32_t          : 1;
            volatile uint32_t LENGTH   : 8;   /**< data length of rd/wr cmd (when CMDDATA is specified)   */
            volatile uint32_t POLL     : 1;   /**< enable H/W polling flash status (blocking if flash is busy)    */
            volatile uint32_t          : 6;
            volatile uint32_t ACT      : 1;   /**< Fire flash controller, this bit is RW1C type (Write-1-to-clear)    */
        } b;

        volatile uint32_t   cmd;
    };
} fc_csr_cmd_t;

typedef struct fc_csr_cfg
{
    union {
        struct {
            volatile uint32_t CMDCLK   : 4;   /**< clock division of command access   */
            volatile uint32_t MEMCLK   : 4;   /**< clock division of memory access    */
            volatile uint32_t SIZE     : 3;   /**< flash space size   */
            volatile uint32_t          : 1;
            volatile uint32_t MEMMODE  : 3;   /**< memory mode; 0: 1-IO mode, 5: 4-IO mode    */
            volatile uint32_t          : 1;
            volatile uint32_t CMDLAT   : 2;   /**< Command access data latch timing control   */
            volatile uint32_t MEMLAT   : 2;   /**< Memory access data latch timing control    */
            volatile uint32_t TYPE     : 1;   /**< Flash type; 0: Winbond, 1: MXIC    */
            volatile uint32_t PERFENHC : 1;   /**< enable performance enhance */
            volatile uint32_t          : 1;
            volatile uint32_t MEMADDR4 : 1;   /**< 3 or 4 bytes address for memory access */
            volatile uint32_t CSNWIDTH : 4;   /**< The high state width cycles of the CSN */
            volatile uint32_t          : 4;
        } b;

        volatile uint32_t    cfg;
    };
} fc_csr_cfg_t;

typedef struct fc_csr_cache
{
    union {
        struct {
            volatile uint32_t CACHE_EN    : 1;    /**< enable cache */
            volatile uint32_t CACHE_SIZE  : 1;    /**< control cache size; 0: 8KB, 1: 4KB */
            volatile uint32_t CACHE_HITEN : 1;    /**< enable cache hit rate calculation */
            volatile uint32_t             : 29;
        } b;

        volatile uint32_t    cache;
    };
} fc_csr_cache_t;

typedef struct fc_csr_sr
{
    union {
        struct {
            volatile uint32_t STATUS : 8;
            volatile uint32_t        : 24;
        } b;

        volatile uint32_t  sr;
    };
} fc_csr_sr_t;

typedef struct fc_csr_factory
{
    union {
        struct {
            volatile uint32_t DEV_ID  : 16;
            volatile uint32_t MANU_ID : 8;
            volatile uint32_t         : 8;
        } b;

        volatile uint32_t    factory;
    };
} fc_csr_factory_t;

typedef struct fc_csr_cachehit
{
    union {
        struct {
            volatile uint32_t HITRATE : 16;   /**< Cache hit rate (when CACHE_HITEN is specified) */
            volatile uint32_t         : 16;
        } b;

        volatile uint32_t    cachehit;
    };
} fc_csr_cachehit_t;

/**
 *  status register of GD
 */
typedef struct fc_sr_gd
{
    union {
        struct {
            volatile uint32_t WIP  : 1;   /**< write in progress        */
            volatile uint32_t WEL  : 1;   /**< write enable latch       */
            volatile uint32_t BP   : 3;   /**< block protect bits       */
//            volatile uint32_t BP1  : 1;   /**< block protect bits       */
//            volatile uint32_t BP2  : 1;   /**< block protect bits       */
            volatile uint32_t BP3  : 1;   /**< block protect bits       */
            volatile uint32_t BP4  : 1;   /**< block protect bits       */
            volatile uint32_t SRP0 : 1;   /**< status register protect  */
            volatile uint32_t SRP1 : 1;   /**< status register protect  */
            volatile uint32_t QE   : 1;   /**< Quad Enable              */
            volatile uint32_t LB   : 1;   /**< One Time Program (OTP) bit, protect the Security Registers  */
            volatile uint32_t      : 2;
            volatile uint32_t HPF  : 1;   /**< The High Performance Flag */
            volatile uint32_t CMP  : 1;   /**< Complement Protect        */
            volatile uint32_t SUS  : 1;   /**< Suspend Status            */

        } b;

        volatile uint32_t   sr;
    };
} fc_sr_gd_t;

/**
 *  status register of winbond
 */
typedef struct fc_sr1_winbond
{
    union {
        struct {
            volatile uint32_t BUSY : 1;   /**< write in progress        */
            volatile uint32_t WEL  : 1;   /**< write enable latch       */
            volatile uint32_t BP   : 3;   /**< block protect bits       */
//            volatile uint32_t BP1  : 1;   /**< block protect bits       */
//            volatile uint32_t BP2  : 1;   /**< block protect bits       */
            volatile uint32_t TB   : 1;   /**< top/bottom protect       */
            volatile uint32_t SEC  : 1;   /**< sector protect           */
            volatile uint32_t SRP0 : 1;   /**< status register protect  */
            volatile uint32_t      : 24;
        } b;

        volatile uint32_t   sr1;
    };
} fc_sr1_winbond_t;

typedef struct fc_sr2_winbond
{
    union {
        struct {
            volatile uint32_t SRP1: 1;   /**< Status Register Lock          */
            volatile uint32_t QE  : 1;   /**< Quad Enable                   */
            volatile uint32_t     : 1;   /**< Reserved                      */
            volatile uint32_t LB1 : 1;   /**< Security Register Lock Bits   */
            volatile uint32_t LB2 : 1;   /**< Security Register Lock Bits   */
            volatile uint32_t LB3 : 1;   /**< Security Register Lock Bits   */
            volatile uint32_t CMP : 1;   /**< Complement Protect            */
            volatile uint32_t SUS : 1;   /**< Suspend Status                */
            volatile uint32_t     : 24;
        } SR2b;

        volatile uint32_t   sr2;
    };
} fc_sr2_winbond_t;

typedef struct fc_sr3_winbond
{
    union {
        struct {
            volatile uint32_t          : 1;   /**< Reserved                                        */
            volatile uint32_t          : 1;   /**< Reserved                                        */
            volatile uint32_t WPS      : 1;   /**< Write Protect Selection                         */
            volatile uint32_t          : 1;   /**< Reserved                                        */
            volatile uint32_t          : 1;   /**< Reserved                                        */
            volatile uint32_t DRV0     : 1;   /**< output driver strength for the Read operations. */
            volatile uint32_t DRV1     : 1;   /**< output driver strength for the Read operations.
                                                   (DRV0, DRV1) = (0, 0): 100%
                                                   (DRV0, DRV1) = (0, 1): 75%
                                                   (DRV0, DRV1) = (1, 0): 50%
                                                   (DRV0, DRV1) = (1, 1): 25% (default)            */
            volatile uint32_t HOLD_RST : 1;   /**< HOLD or Reset Functions                         */
            volatile uint32_t          : 24;
        } b;

        volatile uint32_t        sr3;
    };
} fc_sr3_winbond_t;

/**
 *  Control/Status Register (CSR) of flash controller
 */
typedef struct fc_csr
{
    volatile uint32_t    CMD;
    volatile uint32_t    ADDR;
    volatile uint32_t    CFG;
    volatile uint32_t    CACHE;
    volatile uint32_t    SR;
    volatile uint32_t    FACTORY_ID;
    volatile uint32_t    CACHEHIT;
    volatile uint32_t    reserved[57];
    volatile uint32_t    BUF[64];
} fc_csr_t;

typedef struct flash_attr
{
    uint32_t        factory_id;             /**< the flash id: manufacturer + factory_id */
    uint32_t        space_bytes;            /**< The total bytes */
    uint16_t        one_page_bytes;         /**< the bytes of a page */
    uint16_t        one_block_kbytes;       /**< the kbytes of a block */
    uint32_t        capability;

    struct {
        uint16_t    one_page_bytes : 12;    /**< security register: the bytes of a page */
        uint16_t    page_num       : 4;     /**< security register: the number of pages */
    } sec_reg;

    // protection configuration
    struct {
        uint16_t    max_level                 : 4;
        uint16_t    one_protected_unit_kbytes : 12;
    } protect;

} flash_attr_t;

typedef struct flash_dev
{
    uint32_t    factory_id;     /**< the flash id: manufacturer + factory_id */
    uint16_t    block_num;      /**< total blocks of a flash */
    uint16_t    protect_lv_max;

} flash_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
flash_attr_t     g_flash_attr __relocdata =
{
    .factory_id        = 0ul,
    .capability        = FC_ERASE_MODE_SECTOR | FC_ERASE_MODE_BLOCK | FC_ERASE_MODE_CHIP | FC_3BYTES_ADDR,
    .one_page_bytes    = (0x1ul << FC_PAGE_SIZE_POW_2),
    .one_block_kbytes  = 64,
    .sec_reg = {
        .one_page_bytes = 256,
        .page_num       = 3,    // for compatibility
    },
    .protect = { .one_protected_unit_kbytes = 64, },
};

#if 0
flash_dev_t     g_flash_list[] __relocdata =
{
    { .factory_id = 0x00EF4015ul, .block_num = 32, .protect_lv_max = 6, },
    { .factory_id = 0x00EF6015ul, .block_num = 32, .protect_lv_max = 6, },
    { .factory_id = 0x00C86015ul, .block_num = 32, .protect_lv_max = 6, },
    { .factory_id = 0x00EF4016ul, .block_num = 64, .protect_lv_max = 7, },
    { .factory_id = 0x00EF6016ul, .block_num = 64, .protect_lv_max = 7, },
    { .factory_id = 0x0ul, },
};
#endif
//=============================================================================
//                  Private Function Definition
//=============================================================================
void __reloc _hal_fc_memcpy_ram(uint8_t* dst, uint8_t* src, uint32_t len)
{
    uint32_t i;
    for(i = 0; i<len; i++)
    {
        dst[i] = src[i];
    }
}
void __reloc
_hal_fc_write_enable(
    flash_attr_t    *pAttr,
    uint32_t        is_volatile)
{
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;
    do {
        uint32_t    value = 0ul;

        if( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_WR_ENABLE) )
            break;

        ((fc_csr_cmd_t*)&value)->b.CMDID    = (is_volatile) ? QSPI_CMD_VOLATILE_SR_EN : QSPI_CMD_WRITE_ENABLE;
        ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_NO_DATA;
        ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_NO_ADDR;
        ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
        ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
        ((fc_csr_cmd_t*)&value)->b.POLL     = 0;
        ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

        pCSR->CMD = value;

        if( is_volatile )
        {
            FC_DELAY(FC_GD_DELAY_TICKS);
        }
        else
        {
            if( (pAttr->factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
                FC_DELAY(FC_GD_DELAY_TICKS);

            while( !IS_FC_IN_STATUS(pCSR, MSK_FC_SR_WR_ENABLE) ) {}
        }
    } while(0);
    return;
}

void __reloc
_hal_fc_write_disable(void)
{
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;
    do {
        uint32_t    value = 0ul;
        uint32_t    factory_id = 0ul;

        factory_id = pCSR->FACTORY_ID;

        if( !IS_FC_IN_STATUS(pCSR, MSK_FC_SR_WR_ENABLE) )
            break;

        ((fc_csr_cmd_t*)&value)->b.CMDID    = QSPI_CMD_WRITE_DISABLE;
        ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_NO_DATA;
        ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_NO_ADDR;
        ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
        ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
        ((fc_csr_cmd_t*)&value)->b.POLL     = 1;
        ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

        pCSR->CMD = value;

        if( (factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
            FC_DELAY(FC_GD_DELAY_TICKS);

        while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_WR_ENABLE) ) {}

    } while(0);

    return;
}

void __reloc
_hal_fc_erase(
    uint32_t        erase_cmd,
    uintptr_t       address,
    flash_attr_t    *pAttr)
{
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;
    uint32_t    value = 0ul;

//    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    // erase can't use SR volatile mode
    _hal_fc_write_enable(pAttr, false);

    // flash controller will auto add flash offset ==> the address must subtracted flash offset
    address = address + pAttr->space_bytes - GET_FLASH_BOOT_START_POS();
    if (address >= pAttr->space_bytes)
        address -= pAttr->space_bytes;
    pCSR->ADDR = address;

    ((fc_csr_cmd_t*)&value)->b.CMDID    = erase_cmd;
    ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_NO_DATA;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR  = (erase_cmd == QSPI_CMD_CHIP_ERASE)
                                        ? FC_CMD_NO_ADDR : FC_CMD_WITH_ADDR;
    ((fc_csr_cmd_t*)&value)->b.LENGTH   = 0;
    ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                        ? FC_CMD_ADDR_4BYTES : FC_CMD_ADDR_3BYTES;
    ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
    ((fc_csr_cmd_t*)&value)->b.POLL     = 0;
    ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

    pCSR->CMD = value;

    if( (pAttr->factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
        FC_DELAY(FC_GD_DELAY_TICKS);

    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    return;
}

void __reloc
_hal_fc_program_page(
    uintptr_t       address,
    uint8_t         *pData,
    uint16_t        length,
    uint16_t        is_quad_mode,
    flash_attr_t    *pAttr)
{
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;
    uint32_t    cmd = QSPI_CMD_PAGE_PROG;
    uint32_t    io_mode = FC_CMD_MODE_1I1O;
    uint32_t    value = 0ul;

//    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    // erase can't use SR volatile mode
    _hal_fc_write_enable(pAttr, false);

    // flash controller will auto add flash offset ==> the address must subtracted flash offset
    address = address + pAttr->space_bytes - GET_FLASH_BOOT_START_POS();
    if (address >= pAttr->space_bytes)
        address -= pAttr->space_bytes;
    pCSR->ADDR = address;

    _hal_fc_memcpy_ram((uint8_t*)(&pCSR->BUF[0]), pData, length);

    if( is_quad_mode )
    {
        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD ||
            (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            cmd     = QSPI_CMD_QUAD_IN_FAST_PROG;
            io_mode = FC_CMD_MODE_1I4O;
        }
    }

    ((fc_csr_cmd_t*)&value)->b.CMDID    = cmd;
    ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_WITH_DATA;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_WITH_ADDR;
    ((fc_csr_cmd_t*)&value)->b.LENGTH   = length - 1;
    ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                        ? FC_CMD_ADDR_4BYTES : FC_CMD_ADDR_3BYTES;
    ((fc_csr_cmd_t*)&value)->b.CMDMODE  = io_mode;
    ((fc_csr_cmd_t*)&value)->b.POLL     = 0;
    ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

    pCSR->CMD = value;

    if( (pAttr->factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
        FC_DELAY(FC_GD_DELAY_TICKS);

    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    return;
}

void __reloc
_hal_fc_set_sr(
    uint32_t        cmd,
    uint32_t        is_volatile,
    uint32_t        flags,
    uint32_t        length,
    flash_attr_t    *pAttr)
{
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;
    uint32_t    value = 0ul;

//    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    ((fc_csr_cmd_t*)&value)->b.CMDID    = cmd;
    ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_WITH_DATA;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_NO_ADDR;
    ((fc_csr_cmd_t*)&value)->b.LENGTH   = length - 1;
    ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
    ((fc_csr_cmd_t*)&value)->b.CMDADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                        ? FC_CMD_ADDR_4BYTES : FC_CMD_ADDR_3BYTES;
    ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
    ((fc_csr_cmd_t*)&value)->b.POLL     = 1;
    ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

    _hal_fc_write_enable(pAttr, is_volatile);
    pCSR->BUF[0] = flags;
    pCSR->CMD    = value;

    if( (pAttr->factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
        FC_DELAY(FC_GD_DELAY_TICKS);

    while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

    return;
}

fc_err_t __reloc
_hal_fc_get_attr(
    flash_attr_t    **ppAttr)
{
    do {
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        uint32_t        factory_id = 0ul;

        *ppAttr = 0;

        if( g_flash_attr.factory_id )
        {
            *ppAttr = &g_flash_attr;
            break;
        }

        factory_id = pCSR->FACTORY_ID;

        if( (factory_id & 0x00FF0000) != FC_MANUFACTURER_ID_GD &&
            (factory_id & 0x00FF0000) != FC_MANUFACTURER_ID_WINBOND )
            break;

        {
            flash_attr_t    *pAttr = &g_flash_attr;
            uint32_t        value = 0ul;
            union { int i[2]; double d; } u = { .d = 0.0f, };

            pAttr->capability        |= FC_1I4O_MODE;
            pAttr->factory_id        = factory_id;
            pAttr->space_bytes       = 0x1ul << (factory_id & 0xFF);

            value = pAttr->space_bytes / (pAttr->one_block_kbytes << 10);
            u.d   = value ^ (value - !!value);
            pAttr->protect.max_level = ((u.i[1] >> 20) - 1023) + 1;
            if( pAttr->space_bytes > (16 << 20) )
                pAttr->capability |= FC_4BYTES_ADDR;

            *ppAttr = pAttr;
            break;
        }

    } while(0);

    return (*ppAttr) ? FC_ERR_OK : FC_ERR_UNKNOWN_DEVICE;
}

uint32_t __reloc
_hal_fc_conver_space(
    uint32_t    space_bytes)
{
    uint32_t    rval = 0;

    space_bytes = space_bytes << 3;

    if( space_bytes == QSPI_SPACE_512KBITS )
        rval = FC_SIZE_512KB;
    else if( space_bytes == QSPI_SPACE_1MBITS )
        rval = FC_SIZE_1MB;
    else if( space_bytes == QSPI_SPACE_2MBITS )
        rval = FC_SIZE_2MB;
    else if( space_bytes == QSPI_SPACE_4MBITS )
        rval = FC_SIZE_4MB;
    else if( space_bytes == QSPI_SPACE_8MBITS )
        rval = FC_SIZE_8MB;
    else if( space_bytes == QSPI_SPACE_16MBITS )
        rval = FC_SIZE_16MB;
    else if( space_bytes == QSPI_SPACE_32MBITS )
        rval = FC_SIZE_32MB;
    else if( space_bytes == QSPI_SPACE_64MBITS )
        rval = FC_SIZE_64MB;
    return rval;
}

fc_err_t __reloc
_hal_fc_verify(
    uintptr_t       flash_addr,
    flash_attr_t    **ppAttr)
{
    fc_err_t    rval = FC_ERR_OK;
    do {
        uintptr_t   flash_mem_base = hal_fc_get_flash_base_addr();

        rval = _hal_fc_get_attr(ppAttr);
        if( rval ) break;

        if( flash_addr == (uintptr_t)(-1) )
            break;

        if( flash_addr < flash_mem_base ||
            flash_addr > (flash_mem_base + (*ppAttr)->space_bytes) )
        {
            rval = FC_ERR_WRONG_ADDERSS;
            break;
        }
    } while(0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
fc_err_t
hal_fc_init(
    fc_clk_div_t    divider)
{
    fc_err_t    rval = FC_ERR_OK;
    do {
        flash_attr_t    *pAttr = 0;
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        uint32_t        value = 0ul;

        rval = _hal_fc_get_attr(&pAttr);
        if( rval ) break;

        value = 0;
        ((fc_csr_cfg_t*)&value)->b.MEMMODE  = FC_MEMMODE_1I4O;
        ((fc_csr_cfg_t*)&value)->b.PERFENHC = FC_PERFENHC_MODE_DIS;
        ((fc_csr_cfg_t*)&value)->b.CSNWIDTH = FC_CSHTime_9Cycle;
        ((fc_csr_cfg_t*)&value)->b.CMDLAT   = FC_LAT_FALLING_EDGE;
        ((fc_csr_cfg_t*)&value)->b.MEMLAT   = FC_LAT_FALLING_EDGE;
        ((fc_csr_cfg_t*)&value)->b.CMDCLK   = (divider & 0xF);
        ((fc_csr_cfg_t*)&value)->b.MEMCLK   = (divider & 0xF);
        ((fc_csr_cfg_t*)&value)->b.SIZE     = _hal_fc_conver_space(pAttr->space_bytes);
        ((fc_csr_cfg_t*)&value)->b.TYPE     = ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_MXIC)
                                            ? FC_DEV_MXIC : FC_DEV_WINBOND;
        ((fc_csr_cfg_t*)&value)->b.MEMADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                            ? FC_MEMADDR4_4BYTES : FC_MEMADDR4_3BYTES;

        pCSR->CFG = value;

    } while(0);

    return rval;
}

fc_err_t
hal_fc_deinit(void)
{
    fc_err_t    rval = FC_ERR_OK;
    do {

    } while(0);

    return rval;
}

fc_err_t
hal_fc_erase_phy(
    fc_capability_t     mode,
    uintptr_t           flash_phy_addr,
    int                 count)
{
    fc_err_t    rval = FC_ERR_OK;
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;

    do {
        uintptr_t       act_addr = 0;
        flash_attr_t    *pAttr = 0;

        flash_phy_addr = (mode == FC_ERASE_MODE_CHIP) ? (uintptr_t)(-1) : flash_phy_addr;
        rval = _hal_fc_verify(flash_phy_addr, &pAttr);
        if( rval ) break;

        FC_CACHE_STOP(pCSR);

        if( ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD &&
              ((fc_sr_gd_t*)&pCSR->SR)->b.BP) ||
            ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND &&
              ((fc_sr1_winbond_t*)&pCSR->SR)->b.BP) )
        {
            rval = FC_ERR_DATA_PROTECTED;
            break;
        }

        // check supported erase capability
        if( !(mode & pAttr->capability) )
        {
            rval = FC_ERR_NOT_SUPPORT;
            break;
        }

        flash_phy_addr -= hal_fc_get_flash_base_addr();

        act_addr = flash_phy_addr;

        for(int i = 0; i < count; i++)
        {
            // erase command
            if( mode == FC_ERASE_MODE_SECTOR )
            {
                act_addr = flash_phy_addr + i * FC_ONE_SECTOR_SIZE;

                _hal_fc_erase(QSPI_CMD_SECTOR_ERASE, act_addr, pAttr);
            }
            else if( mode == FC_ERASE_MODE_CHIP )
            {
                i = count;

                _hal_fc_erase(QSPI_CMD_CHIP_ERASE, 0, pAttr);
            }
            else if( mode == FC_ERASE_MODE_BLOCK )
            {
                act_addr = flash_phy_addr + i * pAttr->one_block_kbytes;

                _hal_fc_erase((pAttr->one_block_kbytes == 64) ? QSPI_CMD_BLOCK_ERASE_64K : QSPI_CMD_BLOCK_ERASE_32K,
                              act_addr, pAttr);
            }
            else if( mode == FC_ERASE_MODE_PAGE )
            {
                // depend on flash feature
                rval = FC_ERR_NOT_SUPPORT;
                break;
            }
        }

    } while(0);

    FC_CACHE_START(pCSR);

    // write disable
    _hal_fc_write_disable();

    return rval;
}

fc_err_t
hal_fc_erase(
    fc_capability_t     mode,
    uintptr_t           flash_vir_addr,
    int                 count)
{
    uintptr_t flash_phy_addr = flash_vir_addr + GET_FLASH_BOOT_START_POS();

    return hal_fc_erase_phy(mode, flash_phy_addr, count);
}

fc_err_t
hal_fc_read_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         length)
{
    fc_err_t    rval = FC_ERR_OK;
    do {
        flash_attr_t    *pAttr = 0;
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        uint32_t        is_quad_mode = false;

        rval = _hal_fc_verify(flash_phy_addr, &pAttr);
        if( rval ) break;

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD ||
            (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            is_quad_mode = (hal_fc_get_sr1() & (FC_QUAD_EN >> 8));
        }

        // flash controller will auto add flash offset ==> the address must subtracted flash offset
        flash_phy_addr = flash_phy_addr + pAttr->space_bytes - GET_FLASH_BOOT_START_POS();
        if (flash_phy_addr >= pAttr->space_bytes)
            flash_phy_addr -= pAttr->space_bytes;

        // TODO: DMA case ??
        if( is_quad_mode )
        {
            ((fc_csr_cmd_t*)&pCSR->CMD)->b.CMDMODE = FC_CMD_MODE_1I4O;
            _hal_fc_memcpy_ram((void*)buf_addr, (void*)flash_phy_addr, length);
        }
        else
        {
            _hal_fc_memcpy_ram((void*)buf_addr, (void*)flash_phy_addr, length);
        }
    } while(0);

    return rval;
}

fc_err_t
hal_fc_read(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         length)
{
    uintptr_t flash_phy_addr = flash_vir_addr + GET_FLASH_BOOT_START_POS();

    return hal_fc_read_phy(buf_addr, flash_phy_addr, length);
}

fc_err_t
hal_fc_write_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         length,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr),
    uint32_t    *pCache_buf)
{
    fc_err_t    rval = FC_ERR_OK;
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;

    do {
        flash_attr_t    *pAttr = 0;
        uintptr_t       write_addr = 0;
        uintptr_t       from_pos = 0;
        uintptr_t       to_pos = 0;
        uintptr_t       flash_mmp_base = 0;
        uint16_t        is_quad_mode = false;
        uint32_t        offset = 0;
        uint32_t        valid_len = 0;
        int             remain = length;

        rval = _hal_fc_verify(flash_phy_addr, &pAttr);
        if( rval ) break;

        FC_CACHE_STOP(pCSR);

        if( ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD &&
              ((fc_sr_gd_t*)&pCSR->SR)->b.BP) ||
            ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND &&
              ((fc_sr1_winbond_t*)&pCSR->SR)->b.BP) )
        {
            rval = FC_ERR_DATA_PROTECTED;
            break;
        }

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD ||
            (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            is_quad_mode = (hal_fc_get_sr1() & (FC_QUAD_EN >> 8));
        }

        flash_mmp_base  = hal_fc_get_flash_base_addr();
        offset          = flash_phy_addr & (FC_ONE_SECTOR_SIZE  - 1);
        valid_len       = FC_ONE_SECTOR_SIZE - offset;
        to_pos          = flash_phy_addr & ~(FC_ONE_SECTOR_SIZE - 1);
        from_pos        = buf_addr;

        // read data to cache
        write_addr = to_pos;
        if( cb_mmap && cb_mmap(to_pos, &write_addr) )
        {
            rval = FC_ERR_WRONG_ADDERSS;
            break;
        }

        hal_fc_read_phy((uintptr_t)pCache_buf, write_addr, FC_ONE_SECTOR_SIZE);

        valid_len = (valid_len < length) ? valid_len : length;

        // merge data
        _hal_fc_memcpy_ram(&((uint8_t*)pCache_buf)[offset], (void*)from_pos, valid_len);
        from_pos += valid_len;

        // erase the sector
        hal_fc_erase_phy(FC_ERASE_MODE_SECTOR, write_addr, 1);

        // page program
        for(int i = 0; i < (FC_ONE_SECTOR_SIZE >> FC_PAGE_SIZE_POW_2); i++)
        {
            _hal_fc_program_page(write_addr - flash_mmp_base,
                                 (uint8_t*)pCache_buf + (i * (0x1 << FC_PAGE_SIZE_POW_2)),
                                 (0x1 << FC_PAGE_SIZE_POW_2), is_quad_mode, pAttr);

            to_pos += (0x1 << FC_PAGE_SIZE_POW_2);
            write_addr = to_pos;

            if( cb_mmap && cb_mmap(to_pos, &write_addr) )
            {
                rval = FC_ERR_WRONG_ADDERSS;
                break;
            }
        }

        if( rval ) break;

        remain -= valid_len;
        while( (remain & ~(FC_ONE_SECTOR_SIZE - 1)) )
        {
            write_addr = to_pos;

            if( cb_mmap && cb_mmap(to_pos, &write_addr) )
            {
                rval = FC_ERR_WRONG_ADDERSS;
                break;
            }

            // erase sector
            hal_fc_erase_phy(FC_ERASE_MODE_SECTOR, write_addr, 1);

            // page program
            for(int i = 0; i < (FC_ONE_SECTOR_SIZE >> FC_PAGE_SIZE_POW_2); i++)
            {
                _hal_fc_program_page(write_addr - flash_mmp_base, (uint8_t*)from_pos,
                                     (0x1 << FC_PAGE_SIZE_POW_2), is_quad_mode, pAttr);

                from_pos += (0x1 << FC_PAGE_SIZE_POW_2);
                to_pos   += (0x1 << FC_PAGE_SIZE_POW_2);

                write_addr = to_pos;

                if( cb_mmap && cb_mmap(to_pos, &write_addr) )
                {
                    rval = FC_ERR_WRONG_ADDERSS;
                    break;
                }
            }

            // update info
            remain -= FC_ONE_SECTOR_SIZE;
        }

        if( rval ) break;

        if( remain )
        {
            // read data to cache
            hal_fc_read_phy((uintptr_t)pCache_buf, write_addr, FC_ONE_SECTOR_SIZE);

            // merge data
            _hal_fc_memcpy_ram((void*)pCache_buf, (void*)from_pos, remain);

            // erase sector
            hal_fc_erase_phy(FC_ERASE_MODE_SECTOR, write_addr, 1);

            // page program
            for(int i = 0; i < (FC_ONE_SECTOR_SIZE >> FC_PAGE_SIZE_POW_2); i++)
            {
                _hal_fc_program_page(write_addr - flash_mmp_base,
                                     (uint8_t*)pCache_buf + (i * (0x1 << FC_PAGE_SIZE_POW_2)),
                                     (0x1 << FC_PAGE_SIZE_POW_2), is_quad_mode, pAttr);
                to_pos += (0x1 << FC_PAGE_SIZE_POW_2);

                write_addr = to_pos;

                if( cb_mmap && cb_mmap(to_pos, &write_addr) )
                {
                    rval = FC_ERR_WRONG_ADDERSS;
                    break;
                }
            }
        }
    } while(0);

    FC_CACHE_START(pCSR);

    // write disable
    _hal_fc_write_disable();

    return rval;
}

fc_err_t
hal_fc_write(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         length,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr),
    uint32_t    *pCache_buf)
{
    uintptr_t flash_phy_addr = flash_vir_addr + GET_FLASH_BOOT_START_POS();

    return hal_fc_write_phy(buf_addr, flash_phy_addr, length, cb_mmap, pCache_buf);
}

fc_err_t
hal_fc_write_raw_phy(
    uintptr_t   buf_addr,
    uintptr_t   flash_phy_addr,
    int         bytes,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr))
{
    fc_err_t    rval = FC_ERR_OK;
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;

    do {
        flash_attr_t    *pAttr = 0;
        uintptr_t       flash_mmp_base = 0;
        uintptr_t       write_addr = 0;
        uintptr_t       from_pos = 0;
        uintptr_t       to_pos = 0;
        uint16_t        is_quad_mode = false;

        rval = _hal_fc_verify(flash_phy_addr, &pAttr);
        if( rval ) break;

        FC_CACHE_STOP(pCSR);

        if( ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD &&
              ((fc_sr_gd_t*)&pCSR->SR)->b.BP) ||
            ((pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND &&
              ((fc_sr1_winbond_t*)&pCSR->SR)->b.BP) )
        {
            rval = FC_ERR_DATA_PROTECTED;
            break;
        }

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD ||
            (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            is_quad_mode = (hal_fc_get_sr1() & (FC_QUAD_EN >> 8));
        }

        flash_mmp_base  = hal_fc_get_flash_base_addr();
        to_pos          = flash_phy_addr;
        from_pos        = buf_addr;
        write_addr        = to_pos;

        if( cb_mmap && cb_mmap(to_pos, &write_addr) )
        {
            rval = FC_ERR_WRONG_ADDERSS;
            break;
        }

        for(int i = 0; i < (bytes >> FC_PAGE_SIZE_POW_2); i++)
        {
            _hal_fc_program_page(write_addr - flash_mmp_base, (uint8_t*)from_pos,
                                 (uint16_t)(0x1 << FC_PAGE_SIZE_POW_2), is_quad_mode, pAttr);

            from_pos += sizeof(pCSR->BUF);
            to_pos   += sizeof(pCSR->BUF);
            write_addr  = to_pos;

            if( cb_mmap && cb_mmap(to_pos, &write_addr) )
            {
                rval = FC_ERR_WRONG_ADDERSS;
                break;
            }
        }

        if( rval ) break;

        bytes = bytes & ((0x1 << FC_PAGE_SIZE_POW_2) - 1);
        if( bytes )
        {
            _hal_fc_program_page(write_addr - flash_mmp_base, (uint8_t*)from_pos,
                                 (uint16_t)bytes, is_quad_mode, pAttr);

            from_pos += bytes;
            to_pos   += bytes;
            write_addr  = to_pos;

            if( cb_mmap && cb_mmap(to_pos, &write_addr) )
            {
                rval = FC_ERR_WRONG_ADDERSS;
                break;
            }
        }
    } while(0);

    FC_CACHE_START(pCSR);

    // write disable
    _hal_fc_write_disable();

    return rval;
}

fc_err_t
hal_fc_write_raw(
    uintptr_t   buf_addr,
    uintptr_t   flash_vir_addr,
    int         bytes,
    int (*cb_mmap)(uintptr_t phy_addr, uintptr_t *pVir_addr))
{
    uintptr_t flash_phy_addr = flash_vir_addr + GET_FLASH_BOOT_START_POS();

    return hal_fc_write_raw_phy(buf_addr, flash_phy_addr, bytes, cb_mmap);
}

uint8_t
hal_fc_get_sr(void)
{
    uint32_t    status = 0;
    fc_csr_t    *pCSR = (fc_csr_t*)FC_CSR_BASE;

    // when read SR register, the H/w module will automatically send '05h'
    status = pCSR->SR;
    return (uint8_t)(status & 0xFF);
}

uint8_t
hal_fc_get_sr1(void)
{
    uint32_t    status = 0;
    do {
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        flash_attr_t    *pAttr = 0;
        uint32_t        value = 0ul;

        if( _hal_fc_verify((uintptr_t)(-1), &pAttr) ) break;

        ((fc_csr_cmd_t*)&value)->b.CMDID    = QSPI_CMD_READ_EX_STATUS_REG;
        ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_WITH_DATA;
        ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_NO_ADDR;
        ((fc_csr_cmd_t*)&value)->b.LENGTH   = 0;
        ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_READ;
        ((fc_csr_cmd_t*)&value)->b.CMDADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                            ? FC_CMD_ADDR_4BYTES : FC_CMD_ADDR_3BYTES;
        ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
        ((fc_csr_cmd_t*)&value)->b.POLL     = 0;
        ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

        pCSR->CMD = value;

        while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}

        // data order: [31:24]=Byte5, [23:16]=Byte4, [15:8]=Byte3, [7:0]=Byte2
        status = pCSR->BUF[0];

    } while(0);

    return (status & 0xFF);
}

/**
 *  @brief  hal_fc_set_sr (Non thread-safe)
 *
 *  @param [in] value                   the value to set to status register of the flash
 *  @param [in] has_use_volatile        use volatile status register or not
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_set_sr(
    uint16_t    value,
    uint32_t    has_use_volatile)
{
    fc_err_t    rval = FC_ERR_OK;

    do {
        flash_attr_t    *pAttr = 0;

        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD )
        {
            _hal_fc_set_sr(QSPI_CMD_WRITE_STATUS_REG, has_use_volatile, (uint32_t)(value & 0xFFFF), 2, pAttr);
        }
        else if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            _hal_fc_set_sr(QSPI_CMD_WRITE_STATUS_REG, has_use_volatile, value & 0xFF, 1, pAttr);
            _hal_fc_set_sr(QSPI_CMD_WINBOND_WR_SR2, has_use_volatile, (value >> 8) & 0xFF, 1, pAttr);
        }

        // write disable
        _hal_fc_write_disable();
    } while(0);
    return rval;
}

fc_err_t
hal_fc_set_protection(
    fc_protect_level_t      level,
    uint32_t                has_use_volatile)
{
    fc_err_t    rval = FC_ERR_OK;
    do {
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        flash_attr_t    *pAttr = 0;
        uint32_t        flags = FC_QUAD_EN;

        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        flags |= (pCSR->SR & 0xFF);

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD )
        {
            if( level > pAttr->protect.max_level )
                level = FC_PROTECT_LEVEL_FULL_PROTECT;

            level = FC_GD_PROTECTION(level);

            if( (flags & MSK_FC_GD_PROTECTION) == level )
                break;

            flags &= ~MSK_FC_GD_PROTECTION;
            flags |= level;

            _hal_fc_set_sr(QSPI_CMD_WRITE_STATUS_REG, has_use_volatile, flags, 2, pAttr);
        }
        else if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            if( level > pAttr->protect.max_level )
                level = FC_PROTECT_LEVEL_FULL_PROTECT;

            level = FC_WINBOND_PROTECTION(level);

            if( (flags & MSK_FC_WINBOND_PROTECTION) == level )
                break;

            flags &= ~MSK_FC_WINBOND_PROTECTION;
            flags |= level;

            _hal_fc_set_sr(QSPI_CMD_WRITE_STATUS_REG, has_use_volatile, flags & 0xFF, 1, pAttr);
        }

        // write disable
        _hal_fc_write_disable();
    } while(0);

    return rval;
}

/**
 *  @brief  hal_fc_set_quad_mode (Non thread-safe)
 *
 *  @param [in] is_enable       enable quad mode or not
 *  @return
 *      error number, reference enum fc_err
 */
fc_err_t
hal_fc_set_quad_mode(
    uint32_t    is_enable)
{
    fc_err_t    rval = FC_ERR_OK;
    do {
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        flash_attr_t    *pAttr = 0;
        uint32_t        flags = 0ul;

        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        flags = (hal_fc_get_sr1() << 8) | (pCSR->SR & 0xFF);
        if( !!is_enable == !!(flags & FC_QUAD_EN) )
            break;

        if( is_enable )
            flags &= ~FC_QUAD_EN;
        else
            flags |= FC_QUAD_EN;

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD )
        {
            _hal_fc_set_sr(QSPI_CMD_WRITE_STATUS_REG, false, flags, 2, pAttr);
        }
        else if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
        {
            _hal_fc_set_sr(QSPI_CMD_WINBOND_WR_SR2, false, (flags >> 8) & 0xFF, 1, pAttr);
        }

        // write disable
        _hal_fc_write_disable();
    } while(0);

    return rval;
}

uintptr_t
hal_fc_get_flash_base_addr(void)
{
    return (GET_REMAP_TYPE()) ? (uintptr_t)FC_MMP_BASE : 0;
}

fc_err_t
hal_fc_get_flash_attr(
    fc_attr_t   *pFc_attr)
{
    fc_err_t        rval = FC_ERR_OK;
    do {
        flash_attr_t    *pAttr = 0;
        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        if( !pFc_attr )
        {
            rval = FC_ERR_NULL_POINTER;
            break;
        }

        pFc_attr->space_bytes               = pAttr->space_bytes;
        pFc_attr->protection_max_level      = pAttr->protect.max_level;
        pFc_attr->one_block_bytes           = pAttr->one_block_kbytes << 10;
        pFc_attr->one_protected_unit_bytes  = pAttr->protect.one_protected_unit_kbytes << 10;
    } while(0);

    return rval;
}

fc_err_t
hal_fc_ctrl_sec_reg(
    fc_sec_reg_ctrl_t   *pCtrl)
{
    fc_err_t        rval = FC_ERR_OK;
    do {
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        flash_attr_t    *pAttr = 0;
        uint32_t        value = 0ul;

        if( !pCtrl )
        {
            rval = FC_ERR_NULL_POINTER;
            break;
        }

        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        if( !pCtrl->buf_addr || !pCtrl->buf_bytes || (pCtrl->offset & 0x3) ||
            pCtrl->offset >= pAttr->sec_reg.one_page_bytes )
        {
            rval = FC_ERR_WRONG_PARAM;
            break;
        }

        if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_GD )
            value = (pCtrl->idx & 0x3) << 8;          // GD start from 0
        else if( (pAttr->factory_id & 0x00FF0000) == FC_MANUFACTURER_ID_WINBOND )
            value = ((pCtrl->idx + 1) & 0x3) << 12;   // WINBOND start from 1

        pCSR->ADDR = value + pCtrl->offset;

        ((fc_csr_cmd_t*)&value)->b.CMDADDR  = FC_CMD_WITH_ADDR;
        ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_WITH_DATA;
        ((fc_csr_cmd_t*)&value)->b.LENGTH   = pCtrl->buf_bytes - 1;
        ((fc_csr_cmd_t*)&value)->b.CMDADDR4 = (pAttr->capability & FC_4BYTES_ADDR)
                                            ? FC_CMD_ADDR_4BYTES : FC_CMD_ADDR_3BYTES;
        ((fc_csr_cmd_t*)&value)->b.CMDMODE  = FC_CMD_MODE_1I1O;
        ((fc_csr_cmd_t*)&value)->b.POLL     = 1;
        ((fc_csr_cmd_t*)&value)->b.ACT      = 1;

        if( pCtrl->opcode == FC_SEC_REG_OPCODE_READ )
        {
            ((fc_csr_cmd_t*)&value)->b.CMDID = QSPI_CMD_SEC_REG_READ;
            ((fc_csr_cmd_t*)&value)->b.CMDWR = FC_CMD_READ;
        }
        else if( pCtrl->opcode == FC_SEC_REG_OPCODE_WRITE )
        {
            _hal_fc_write_enable(pAttr, false);

            ((fc_csr_cmd_t*)&value)->b.CMDID = QSPI_CMD_SEC_REG_PROG;
            ((fc_csr_cmd_t*)&value)->b.CMDWR = FC_CMD_WRITE;

            _hal_fc_memcpy_ram((void*)pCSR->BUF, (void*)pCtrl->buf_addr,
                   (pCtrl->buf_bytes < pAttr->sec_reg.one_page_bytes)
                   ? pCtrl->buf_bytes : pAttr->sec_reg.one_page_bytes);
        }
        else if( pCtrl->opcode == FC_SEC_REG_OPCODE_ERASE )
        {
            _hal_fc_write_enable(pAttr, false);

            ((fc_csr_cmd_t*)&value)->b.CMDID    = QSPI_CMD_SEC_REG_ERASE;
            ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_NO_DATA;
            ((fc_csr_cmd_t*)&value)->b.CMDWR    = FC_CMD_WRITE;
            ((fc_csr_cmd_t*)&value)->b.LENGTH   = 0;
        }

        pCSR->CMD = value;

        if( (pAttr->factory_id & 0x00FF0000ul) == FC_MANUFACTURER_ID_GD )
            FC_DELAY(FC_GD_DELAY_TICKS);

        while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}


        if( pCtrl->opcode == FC_SEC_REG_OPCODE_READ )
        {
            _hal_fc_memcpy_ram((void*)pCtrl->buf_addr, (void*)&pCSR->BUF[pCtrl->offset >> 2],
                   (pCtrl->buf_bytes < pAttr->sec_reg.one_page_bytes)
                   ? pCtrl->buf_bytes : pAttr->sec_reg.one_page_bytes);
        }

        // write disable
        _hal_fc_write_disable();
    } while(0);
    return rval;
}

fc_err_t 
hal_fc_get_unique_id_get(
    uint8_t *puid)
{
    fc_err_t        rval = FC_ERR_OK;
    do
    {
        if(!puid)
        {
            rval = FC_ERR_NULL_POINTER;
            break;
        }
        fc_csr_t        *pCSR = (fc_csr_t*)FC_CSR_BASE;
        flash_attr_t    *pAttr = 0;
        uint32_t        value = 0ul;


        rval = _hal_fc_verify((uintptr_t)(-1), &pAttr);
        if( rval ) break;

        ((fc_csr_cmd_t*)&value)->b.CMDDATA  = FC_CMD_WITH_DATA;
        ((fc_csr_cmd_t*)&value)->b.LENGTH   = 11;
        ((fc_csr_cmd_t*)&value)->b.POLL     = 1;
        ((fc_csr_cmd_t*)&value)->b.ACT      = 1;
        ((fc_csr_cmd_t*)&value)->b.CMDWR    = 0;
        ((fc_csr_cmd_t*)&value)->b.CMDID    = 0x4B; // Read Unique ID Number

        pCSR->CMD = value;

        while( IS_FC_IN_STATUS(pCSR, MSK_FC_SR_BUSY) ) {}
        
        /* shift 4 bytes dummy */
        _hal_fc_memcpy_ram(puid, (uint8_t *)&pCSR->BUF[1], 8);
    } while (0);

    return rval;
}