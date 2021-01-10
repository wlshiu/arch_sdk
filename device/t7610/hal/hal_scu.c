/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_scu.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/13
 * @license
 * @description
 */


#include "hal_scu.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  System control unit
 */
#define SCU_CSR_BASE        0x54000000ul
#define SCUEX_CSR_BASE      0x54000800ul

/**
 *  reboot system
 *  ps. it will catch the bootstrap configuration when reboot
 */
#define POS_PWR_MOD__REBOOT         7
#define MSK_PWR_MOD__REBOOT         0x1

/**
 *  S/w catch booting status (USER IP)
 */
#define POS_SW__CLR_REBOOT_STATUS   29
#define MSK_SW__CLR_REBOOT_STATUS   0x1ul
#define REG_SW__REBOOT_STATUS       (SCU_CSR_BASE + 0x0884ul) // 0x54000884
#define MSK_SW__REBOOT_FLAG         0x1

/**
 *  software reset adjust when reboot
 *  ps. this is faraday IP to enable S/w catch booting status when trigger reboot
 */
#define POS_PWR_MOD__SW_RST         10
#define MSK_PWR_MOD__SW_RST         0x1

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct scuex_csr_reboot_mask1
{
    union {
        volatile uint32_t   reboot_mask1;
        struct {
            volatile uint32_t   CPU_CM4             : 1;
            volatile uint32_t   USER_BRESET         : 1;
            volatile uint32_t   AHB2ERAM_HRST       : 1;
            volatile uint32_t   AHB2ERAM_1_HRST     : 1;
            volatile uint32_t   AHB2ERAM_ROM_HRST   : 1;
            volatile uint32_t   ATE_SPI_FLASH_HRST  : 1;
            volatile uint32_t   BP010_AHB_7X9_HRST  : 1;
            volatile uint32_t   FTDMAC020_HRST      : 1;
            volatile uint32_t   FTH2P020_HRST       : 1;
            volatile uint32_t   FTGMAC030_HRST      : 1;
            volatile uint32_t   USER_IP_TOP_HRST    : 1;
            volatile uint32_t   EFUSE_CTRL_PRST     : 1;
            volatile uint32_t   FTH2P020_PRST       : 1;
            volatile uint32_t   FTGPIO010_PRST      : 1;
            volatile uint32_t   FTIIC010_PRST       : 1;
            volatile uint32_t   FTPWMTMR010_PRST    : 1;
            volatile uint32_t   FTUART010_PRST      : 1;
            volatile uint32_t   FTUART010_1_PRST    : 1;
            volatile uint32_t   FTUART010_2_PRST    : 1;
            volatile uint32_t   FTUART010_3_PRST    : 1;
            volatile uint32_t   FTUART010_4_PRST    : 1;
            volatile uint32_t   FTUART010_5_PRST    : 1;
            volatile uint32_t   FTUART010_6_PRST    : 1;
            volatile uint32_t   FTSSP010_SPI_PRST   : 1;
            volatile uint32_t   FTSSP010_SSP1_PRST  : 1;
            volatile uint32_t   FTSSP010_SSP2_PRST  : 1;
            volatile uint32_t   FTWDT010_PRST       : 1;
        } b;
    };
} scuex_csr_reboot_mask1_t;

typedef struct scuex_csr_swrst_msk1
{
    union {
        volatile uint32_t   swrst_msk1;
        struct {
            volatile uint32_t   CPU_CM4             : 1;
            volatile uint32_t   USER_BRESET         : 1;
            volatile uint32_t   AHB2ERAM_HRST       : 1;
            volatile uint32_t   AHB2ERAM_1_HRST     : 1;
            volatile uint32_t   AHB2ERAM_ROM_HRST   : 1;
            volatile uint32_t   ATE_SPI_FLASH_HRST  : 1;
            volatile uint32_t   BP010_AHB_7X9_HRST  : 1;
            volatile uint32_t   FTDMAC020_HRST      : 1;
            volatile uint32_t   FTH2P020_HRST       : 1;
            volatile uint32_t   FTGMAC030_HRST      : 1;
            volatile uint32_t   USER_IP_TOP_HRST    : 1;
            volatile uint32_t   EFUSE_CTRL_PRST     : 1;
            volatile uint32_t   FTH2P020_PRST       : 1;
            volatile uint32_t   FTGPIO010_PRST      : 1;
            volatile uint32_t   FTIIC010_PRST       : 1;
            volatile uint32_t   FTPWMTMR010_PRST    : 1;
            volatile uint32_t   FTUART010_PRST      : 1;
            volatile uint32_t   FTUART010_1_PRST    : 1;
            volatile uint32_t   FTUART010_2_PRST    : 1;
            volatile uint32_t   FTUART010_3_PRST    : 1;
            volatile uint32_t   FTUART010_4_PRST    : 1;
            volatile uint32_t   FTUART010_5_PRST    : 1;
            volatile uint32_t   FTUART010_6_PRST    : 1;
            volatile uint32_t   FTSSP010_SPI_PRST   : 1;
            volatile uint32_t   FTSSP010_SSP1_PRST  : 1;
            volatile uint32_t   FTSSP010_SSP2_PRST  : 1;
            volatile uint32_t   FTWDT010_PRST       : 1;
        } b;
    };
} scuex_csr_swrst_msk1_t;

/**
 *  external (customer) register
 */
typedef struct scuex_csr
{
    union{
        volatile uint32_t EXREG_0800;
        volatile uint32_t REBOOT_MASK1;
    };

    union {
        volatile uint32_t EXREG_0804;
        volatile uint32_t REBOOT_MASK2;
    };

    union {
        volatile uint32_t EXREG_0808;
        volatile uint32_t SWRST_MSK1;
    };

    union {
        volatile uint32_t EXREG_080C;
        volatile uint32_t SW_RESET_MASK2;
    };

    union {
        volatile uint32_t EXREG_0810;
        volatile uint32_t IO_DCSR1;
    };

    union {
        volatile uint32_t EXREG_0814;
        volatile uint32_t IO_DCSR2;
    };

    union {
        volatile uint32_t EXREG_0818;
        volatile uint32_t IO_DCSR3;
    };

    union {
        volatile uint32_t EXREG_081C;
        volatile uint32_t WKUP_ENABLE;
    };

    union {
        volatile uint32_t EXREG_0820;
        volatile uint32_t PIN_MUX1;
    };

    union {
        volatile uint32_t EXREG_0824;
        volatile uint32_t PIN_MUX2;
    };

    union {
        volatile uint32_t EXREG_0828;
        volatile uint32_t SIDE_BAND_CTRL;
    };

    union {
        volatile uint32_t EXREG_082C;
        volatile uint32_t TS_VALUE1;
    };

    union {
        volatile uint32_t EXREG_0830;
        volatile uint32_t TS_VALUE2;
    };

    union {
        volatile uint32_t EXREG_0834;
        volatile uint32_t AUX_FAULT;
    };

    union {
        volatile uint32_t EXREG_0838;
        volatile uint32_t CLK_DIV;
    };

    union {
        volatile uint32_t EXREG_083C;
        volatile uint32_t REG_83C;
    };

    union {
        volatile uint32_t EXREG_0840;
        volatile uint32_t STCALIB;
    };

    union {
        volatile uint32_t EXREG_0844;
        volatile uint32_t STCLK_DIV;
    };

    union {
        volatile uint32_t EXREG_0848;
        volatile uint32_t TX_POLL_P_SYS;
    };

    union {
        volatile uint32_t EXREG_084C;
        volatile uint32_t CLK_RESET_CTRL;
    };

    union {
        volatile uint32_t EXREG_0850;
        volatile uint32_t STRAP_U;
    };

    union {
        volatile uint32_t EXREG_0854;
        volatile uint32_t STRAP_INIT;
    };

    union {
        volatile uint32_t EXREG_0858;
        volatile uint32_t GLB_COUNT;
    };

    union {
        volatile uint32_t EXREG_085C;
        volatile uint32_t ZCDPH_MUX_SEL;
    };

    union {
        volatile uint32_t EXREG_0860;
        volatile uint32_t REG_860;
    };

    union {
        volatile uint32_t EXREG_0864;
        volatile uint32_t PIN_MUX3;
    };

    union {
        volatile uint32_t EXREG_0868;
        volatile uint32_t UART_CLK;
    };

    union {
        volatile uint32_t EXREG_086C;
        volatile uint32_t SDR_IO_CTRL1;
    };

    union {
        volatile uint32_t EXREG_0870;
        volatile uint32_t SDR_IO_CTRL2;
    };

    union {
        volatile uint32_t EXREG_0874;
        volatile uint32_t SDR_IO_CTRL3;
    };

    union {
        volatile uint32_t EXREG_0878;
        volatile uint32_t IO_OD1;
    };

    union {
        volatile uint32_t EXREG_087C;
        volatile uint32_t IO_OD2;
    };

    union {
        volatile uint32_t EXREG_0880;
        volatile uint32_t PIN_MUX4;
    };

    union {
        volatile uint32_t EXREG_0884;
        volatile uint32_t SW_RST_STATUS;
    };

    union {
        volatile uint32_t EXREG_0888;
        volatile uint32_t CUSTOMIZED;
    };
} scuex_csr_t;

/**
 *  0x20 (rw) power mode
 */
typedef struct scu_csr_pwr_mode
{
    union {
        struct {
            volatile uint32_t PWR_OFF       : 1;  /*!< (rw) power-off state (1) */
            volatile uint32_t SOFT_OFF      : 1;  /*!< (rw) soft-off state (1) */
            volatile uint32_t DORMANT_SAVE  : 1;  /*!< (rw) dormant mode with PLL restore */
            volatile uint32_t SLEEP         : 1;  /*!< (rw) sleep mode (1) (auto cleared after been woke up) */
            volatile uint32_t CPU_SPEED     : 1;  /*!< (rw) CPU frequency change indicator
                                                        (auto cleared after frequency change is done) */
            volatile uint32_t BUS_SPEED     : 1;  /*!< (rw) BUS frequency change indicator
                                                        (auto cleared after frequency change is done) */
            volatile uint32_t FCS           : 1;  /*!< (rw) frequency change indicator
                                                        (auto cleared after frequency change is done) */
            volatile uint32_t REBOOT        : 1;  /*!< (rw) reboot command */
            volatile uint32_t PLL_UPDATE    : 1;  /*!< (rw) PLL update command */
            volatile uint32_t CURR_RESTORE  : 1;  /*!< (rw) restore current frequency setting in the programming register */
            volatile uint32_t SW_RST        : 1;  /*!< (rw) software reset command */
            volatile uint32_t REMAP_CHG     : 1;  /*!< (rw) change remap setting immediately without reboot */
            volatile uint32_t CPUSPEED_MSK  : 4;  /*!< (rw) disable cpu speed function for multi-CPU */
            volatile uint32_t FCLK_MUX      : 4;  /*!< (rw) CPU frequency selection */
            volatile uint32_t BUS_MUX       : 4;  /*!< (rw) BUS frequency selection */
            volatile uint32_t REMAP         : 4;  /*!< (rw) memory space remap setting */
            volatile uint32_t FCS_PLL_RSTN  : 1;  /*!< (rw) reset PLL during FCS 0) reset 1) keep PLL active */
            volatile uint32_t FCS_DLL_RSTN  : 1;  /*!< (rw) reset DLL during FCS 0) reset 1) keep DLL active */
            volatile uint32_t FCS_PLL2_RSTN : 1;  /*!< (rw) reset PLL2 during FCS 0) reset 1) keep PLL2 active */
            volatile uint32_t SELFR_CMD_OFF : 1;  /*!< (rw) turn off DDR/SDR self-refresh command
                                                            during all power mode change sequences (1) */
        } b;

        volatile uint32_t pwr_mode;
    };
} scu_csr_pwr_mode_t;

typedef struct scu_dev
{
    scuex_handle_t      handle;
} scu_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static scu_dev_t      g_scu_dev = { .handle = { .instance = SCU_CSR_BASE, .instance_ex = SCUEX_CSR_BASE, } };
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
scu_err_t
hal_scu_init(
    scuex_handle_t  **ppHScu)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        scuex_csr_t     *pCSR = 0;

        if( !ppHScu )
        {
            rval = SCU_ERR_WRONG_PARAM;
            break;
        }

        *ppHScu = 0;

        pCSR = (scuex_csr_t*)g_scu_dev.handle.instance_ex;

        // clock dividing
        pCSR->EXREG_0838 &= ~0x3FF;

        *ppHScu = (scuex_handle_t*)&g_scu_dev.handle;
    } while(0);
    return rval;
}

scu_err_t
hal_scu_exec_script(
    scu_call_t  *pCalls,
    uint32_t    call_array_size)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        if( !pCalls || !call_array_size )
        {
            rval = SCU_ERR_WRONG_PARAM;
            break;
        }

        while( call_array_size-- )
        {
            uint32_t   value = *(volatile uint32_t*)pCalls->reg;
            value &= ~pCalls->mask;
            value |= pCalls->value;
            *(volatile uint32_t*)pCalls->reg = value;
            pCalls++;
        }
    } while(0);
    return rval;
}

scu_err_t
hal_scu_get_mmp_type(
    uint32_t    *pType)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        if( !pType )
        {
            rval = SCU_ERR_NULL_POINTER;
            break;
        }

        *pType = ((scu_csr_pwr_mode_t*)0x54000020ul)->b.REMAP;
    } while(0);
    return rval;
}

scu_err_t
hal_scu_set_mmp_type(
    uint32_t    type)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        ((scu_csr_pwr_mode_t*)0x54000020ul)->b.REMAP = type & 0xF;
    } while(0);
    return rval;
}

scu_err_t
hal_scu_set_boot_offset(
    uint32_t    offset)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        if( offset & 0xF )
        {
            rval = SCU_ERR_NOT_ALIGNMENT;
            break;
        }

        /**
         *  Flash boot-up will be started physically from BootStartPos
         *  BootStartPos = reg[0x60FFF020] << 4 (16-Bytes alignment)
         */
        *((volatile uint32_t*)0x60FFF020ul) = (offset >> 4);
    } while(0);

    return rval;
}

scu_err_t
hal_scu_get_boot_offset(
    uint32_t    *pOffset)
{
    scu_err_t   rval = SCU_ERR_OK;
    do {
        if( !pOffset )
        {
            rval = SCU_ERR_NULL_POINTER;
            break;
        }

        /**
         *  Flash boot-up will be started physically from BootStartPos
         *  BootStartPos = reg[0x60FFF020] << 4 (16-Bytes alignment)
         */
        *pOffset = *((volatile uint32_t*)0x60FFF020ul) << 4;
    } while(0);

    return rval;
}


scu_err_t
hal_scu_reboot(
    uint32_t    is_boot_record)
{
    scuex_csr_t     *pCSR_scuex = (scuex_csr_t*)SCUEX_CSR_BASE;
    uint32_t        pwr_mask = 0x80000000ul; // 0x80000000 is faraday sample code

    if( is_boot_record )
    {
        pCSR_scuex->EXREG_0878 &= ~(MSK_SW__CLR_REBOOT_STATUS << POS_SW__CLR_REBOOT_STATUS);

        pwr_mask |= (MSK_PWR_MOD__SW_RST << POS_PWR_MOD__SW_RST);
    }
    else
    {
        pCSR_scuex->EXREG_0878 |= (MSK_SW__CLR_REBOOT_STATUS << POS_SW__CLR_REBOOT_STATUS);
    }

    pwr_mask |= (MSK_PWR_MOD__REBOOT << POS_PWR_MOD__REBOOT);

    *((uint32_t*)0x54000020ul) |= pwr_mask;

    return SCU_ERR_OK;
}

scu_err_t
hal_scu_set_hw_reset(
    uint32_t    flags,
    bool        is_enable)
{
    scu_err_t       rval = SCU_ERR_OK;
    scuex_csr_t     *pCSR_scuex = (scuex_csr_t*)SCUEX_CSR_BASE;

    if( is_enable )
    {
        pCSR_scuex->REBOOT_MASK1 &= ~(flags);
        pCSR_scuex->REBOOT_MASK2 = 0x0;
    }
    else
    {
        pCSR_scuex->REBOOT_MASK1 |= flags;
        /**
         *  reset mask of customer
         *      ps. default: bit[0~15] always keep status, others always reset
         *      bit[0]: BootStartPos reset
         *      bit[1]: global R/W register of M0 (reserved)
         *      bit[2]: about DSP
         *      bit[15 ~ 3]: No defined
         */
        pCSR_scuex->REBOOT_MASK2 = 0xFFFF;
    }

    return rval;
}

scu_err_t
hal_scu_set_sw_strap(
    scu_sw_strap_t  *pSw_strap)
{
    scu_err_t   rval = SCU_ERR_OK;

    do {
        scuex_csr_t         *pCSR_scuex = (scuex_csr_t*)SCUEX_CSR_BASE;
        scu_sw_strap_t      *pStrap = 0;

        pStrap = (scu_sw_strap_t*)&pCSR_scuex->EXREG_0850;

        *pStrap = *pSw_strap;
    } while(0);

    return rval;
}

scu_err_t
hal_scu_get_sw_strap(
    scu_sw_strap_t  *pSw_strap)
{
    scu_err_t   rval = SCU_ERR_OK;

    do {
        scuex_csr_t         *pCSR_scuex = (scuex_csr_t*)SCUEX_CSR_BASE;
        scu_sw_strap_t      *pStrap = 0;

        pStrap = (scu_sw_strap_t*)&pCSR_scuex->EXREG_0850;

        *pSw_strap = *pStrap;
    } while(0);

    return rval;
}


