/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file device_hal.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/01
 * @license
 * @description
 */

#ifndef __device_hal_H_wPqRAa38_lSmA_HjFO_sAaC_u93Labujmc7X__
#define __device_hal_H_wPqRAa38_lSmA_HjFO_sAaC_u93Labujmc7X__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hal_errno.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  Configuration of the Cortex-M4 Processor and Core Peripherals
 */
#define __CM4_REV                       0x0001  /*!< Cortex-M4 Core Revision */
#define __MPU_PRESENT                   0       /*!< MPU present or not */
#define __NVIC_PRIO_BITS                3       /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig          0       /*!< Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT                   0       /*!< FPU present or not */

typedef enum IRQn
{
    /**
     *  Cortex-M4 Processor Exceptions Numbers
     */
    NonMaskableInt_IRQn             = -14,  /*!<   2  Non maskable Interrupt, cannot be stopped or preempted    */
    HardFault_IRQn                  = -13,  /*!<   3  Hard Fault, all classes of Fault                          */
    MemoryManagement_IRQn           = -12,  /*!<   4  Memory Management                                         */
    BusFault_IRQn                   = -11,  /*!<   5  Bus Fault                                                 */
    UsageFault_IRQn                 = -10,  /*!<   6  Usage Fault                                               */
    SVCall_IRQn                     =  -5,  /*!<  11  System Service Call via SVC instruction                   */
    DebugMonitor_IRQn               =  -4,  /*!<  12  Debug Monitor                                             */
    PendSV_IRQn                     =  -2,  /*!<  14  Pendable request for system service                       */
    SysTick_IRQn                    =  -1,  /*!<  15  System Tick Timer                                         */

    /**
     *  Phoenix/M4 Specific Interrupt Numbers
     */
    SYS_IRQn                        = 0,    /*!< Brown-Out Low Voltage Detected Interrupt  */
    CoreRxEv_IRQn                   = 1,    /*!< M0 send to M4 Core Event Interrupt        */
    ZCD_IRQn                        = 2,    /*!< ZCD Interrupt                             */
    WAKEUP_IRQn                     = 3,    /*!< wakeup Interrupt                          */
    GPIO_IRQn                       = 4,    /*!< GPIO_P0 Interrupt                         */

    CRC_IRQn                        = 7,    /*!< CRC Interrupt                             */
    SEC_IRQn                        = 8,    /*!< SEC Interrupt                             */

    WDT_IRQn                        = 10,   /*!< WDT Interrupt                             */
    GMAC_IRQn                       = 11,   /*!< MAC Interrupt                             */
    IPC_RxCh0_IRQn                  = 12,   /*!< IPC_RxCh0 Interrupt                       */
    SSP1_IRQn                       = 13,   /*!< SSP1 Interrupt                            */
    DMA_IRQn                        = 14,   /*!< DMA Interrupt                             */
    SSP2_IRQn                       = 15,   /*!< SSP1 Interrupt                            */
    SSP0_IRQn                       = 16,   /*!< SSP0 Interrupt                            */
    IPC_RxCh1_IRQn                  = 17,   /*!< IPC_RxCh1 Interrupt                       */
    TIM0_IRQn                       = 20,   /*!< TIMER0 Interrupt                          */
    TIM1_IRQn                       = 21,   /*!< TIMER1 Interrupt                          */
    TIM2_IRQn                       = 22,   /*!< TIMER2 Interrupt                          */
    TIM3_IRQn                       = 23,   /*!< TIMER3 Interrupt                          */
    I2C_IRQn                        = 24,   /*!< I2C Interrupt                             */
    UART4_IRQn                      = 26,   /*!< UART4 Interrupt                           */
    UART5_IRQn                      = 27,   /*!< UART5 Interrupt                           */
    UART0_IRQn                      = 28,   /*!< UART0 Interrupt                           */
    UART1_IRQn                      = 29,   /*!< UART1 Interrupt                           */
    UART2_IRQn                      = 30,   /*!< UART2 Interrupt                           */
    UART3_IRQn                      = 31,   /*!< UART3 Interrupt                           */

    IRQ_Total
} IRQn_Type;

#define SysCore_IRQn                IRQ_Total

/**
 *  package type
 */
#define HAL_PACKAGE_LQFP128         0x128
#define HAL_PACKAGE_QFN68           0x68

#include "core_cm4.h"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define REG_READ(pReg)                              (*(pReg))
#define REG_WRITE(pReg, data)                       (*(pReg) = (data))
#define REG_MASK_READ(pReg, mask)                   (*(pReg) & (mask))
#define REG_MASK_WRITE(pReg, data, mask)            do { unsigned long __val = REG_MASK_READ(pReg, ~(mask)); \
                                                         __val |= ((data) & (mask));                         \
                                                         *(pReg) = __val;                                    \
                                                    } while(0)

#define SET_BIT(u32_data, bit_order)                ((u32_data) |= (0x1 << (bit_order)))
#define CLR_BIT(u32_data, bit_order)                ((u32_data) &= ~(0x1 << (bit_order)))
#define IS_BIT_SET(u32_data, bit_order)             ((u32_data) & (0x1 << (bit_order)))


#if defined(__GNUC__)
#define __pause()                   do{ __asm volatile("BKPT #01"); }while(0)
#endif

#ifndef MEMBER_OFFSET
#define MEMBER_OFFSET(type, member)     (uintptr_t)&(((type*)0)->member)
#endif

#ifndef DOWN_CAST
#define DOWN_CAST(type, ptr, member)    (type*)((uintptr_t)ptr - MEMBER_OFFSET(type, member))
#endif

#ifndef ALIGN4
#define ALIGN4(x)                       (((uintptr_t)(x) + 0x3) & ~0x3)
#endif

#ifndef FOUR_CC
#define FOUR_CC(a, b, c, d)             (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))
#endif

#define __reloc                 __attribute__((used, section("reloc_text")))
#define __relocdata             __attribute__((used, section("reloc_data")))

/**
 *  __isr_func_h: high priority ISR
 *  __isr_func_n: normal priority ISR
 *  __isr_func_l: low priority ISR
 */
#define __isr_func_h            __attribute__((section("isr_func")))
#define __isr_func_n
#define __isr_func_l

/**
 *  commands section of console
 */
#define __clicmd_pool           __attribute__((used, section("cli_cmd_pool")))

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void (*cb_usr_isr_t)(void);

typedef struct hal_argv
{
    union {
        uint8_t             *ptr;
        uint32_t            int_value;
        float               float_value;
        // unsigned long long  reserved; // 64-bits platform
    };
} hal_argv_t;
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
 *  @brief  hal_init (Non thread-safe)
 *
 *  @return
 *      error code
 */
hal_errno_t
hal_init(void);

/**
 *  @brief  hal_irq_register_isr (Non thread-safe)
 *
 *  @param [in] irq_id          reference enum IRQn
 *  @param [in] usr_callback    user ISR
 *  @return
 *      error code
 */
hal_errno_t
hal_irq_register_isr(
    int             irq_id,
    cb_usr_isr_t    usr_callback);

/**
 *  @brief  hal_irq_enable (Non thread-safe)
 *
 *  @param [in] irq_id          reference enum IRQn
 *  @return
 *      error code
 */
hal_errno_t
hal_irq_enable(
    IRQn_Type   irq_id);

/**
 *  @brief  hal_irq_disable (Non thread-safe)
 *
 *  @param [in] irq_id          reference enum IRQn
 *  @return
 *      error code
 */
hal_errno_t
hal_irq_disable(
    IRQn_Type   irq_id);

/**
 *  @brief  hal_irq_clear (Non thread-safe)
 *
 *  @param [in] irq_id          reference enum IRQn
 *  @param [in] pHandle         the handle of the hal of the target module
 *  @return
 *      error code
 */
hal_errno_t
hal_irq_clear(
    IRQn_Type   irq_id,
    void        *pHandle);

/**
 *  @brief  hal_verify_ic_package (Non thread-safe)
 *
 *  @param [in] irq_id          reference enum IRQn
 *  @return
 *      error code
 */
hal_errno_t
hal_verify_ic_package(
    IRQn_Type   irq_id);


/**
 *  @brief  hal_phy2vir (Non thread-safe)
 *
 *  @param [in] phy_flash_addr  the physical address of flash
 *                              - if access image volume 1, user MUST add the image volume offset 0x100000
 *  @param [in] pVir_addr       return the virtual address of flash after booting offset
 *  @return
 *      error code
 */
hal_errno_t
hal_phy2vir(
    uintptr_t   phy_flash_addr,
    uintptr_t   *pVir_addr);


/**
 *  @brief  hal_vir2phy (Non thread-safe)
 *
 *  @param [in] vir_flash_addr      the virtual address of flash after booting offset
 *  @param [in] pPhy_flash_addr     return the physical address of flash
 *  @return
 *      error code
 */
hal_errno_t
hal_vir2phy(
    uintptr_t   vir_flash_addr,
    uintptr_t   *pPhy_flash_addr);


/**
 *  @brief  hal_sys_get_core_clk
 *
 *  @return
 *      system core clock
 *
 */
uint32_t
hal_sys_get_core_clk(void);

/**
 *  @brief  hal_sys_get_cpu_clk
 *
 *  @return
 *      CPU or AHB bus clock
 *
 */
uint32_t
hal_sys_get_cpu_clk(void);

/**
 *  @brief  hal_sys_get_pclk
 *
 *  @return
 *      PCLK clock
 */
uint32_t
hal_sys_get_pclk(void);

/**
 *  @brief  hal_get_tick
 *              Get current core tick
 *
 *  @return
 *      0xFFFFFFFF: No supported
 *      others    : The current core tick
 */
uint32_t
hal_get_tick(void);


#ifdef __cplusplus
}
#endif

#endif
