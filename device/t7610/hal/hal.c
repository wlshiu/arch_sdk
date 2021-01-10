/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

/* *INDENT-OFF* */

#include "system_ARMCM4.h"
#include "hal_scu.h"
#include "hal_fc.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_ssp.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "hal_wdt.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum slot_usr_isr
{
    SLOT_USER_ISR_SYS_TICK = IRQ_Total,
    SLOT_USER_ISR_PENDSV,
    SLOT_USER_ISR_SVC,
    SLOT_USER_ISR_HARD_FAULT,

    SLOT_USER_ISR_ALL
} slot_usr_isr_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define NOTIFY_USER_IRQ(a)          do { if(g_usr_isr_table[a]) g_usr_isr_table[a](); } while(0)

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static cb_usr_isr_t             g_usr_isr_table[SLOT_USER_ISR_ALL] = {0};
static uint32_t volatile        g_msTicks = 0xFFFFFFFFul;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  ISR Definition
//=============================================================================
void __attribute__((weak, section("isr_func"))) HardFault_Handler(void)
{
    NOTIFY_USER_IRQ(SLOT_USER_ISR_HARD_FAULT);
}

void __attribute__((weak, section("isr_func"))) SVC_Handler(void)
{
    NOTIFY_USER_IRQ(SLOT_USER_ISR_SVC);
}

void __attribute__((weak, section("isr_func"))) PendSV_Handler(void)
{
    NOTIFY_USER_IRQ(SLOT_USER_ISR_PENDSV);
}

void __isr_func_h SysTick_Handler(void)
{
    g_msTicks++;

    NOTIFY_USER_IRQ(SLOT_USER_ISR_SYS_TICK);

    return;
}

void __isr_func_h SYS_IRQHandler(void)
{
    NOTIFY_USER_IRQ(SYS_IRQn);
}

void __isr_func_h CoreRxEv_IRQHandler(void)
{
    NOTIFY_USER_IRQ(CoreRxEv_IRQn);
}

void __isr_func_h ZCD_IRQHandler(void)
{
    NOTIFY_USER_IRQ(ZCD_IRQn);
}

void __isr_func_h WAKEUP_IRQHandler(void)
{
    NOTIFY_USER_IRQ(WAKEUP_IRQn);
}

void __isr_func_h GPIO_IRQHandler(void)
{
    NOTIFY_USER_IRQ(GPIO_IRQn);
}

void __isr_func_h CRC_IRQHandler(void)
{
    NOTIFY_USER_IRQ(CRC_IRQn);
}

void __isr_func_h SEC_IRQHandler(void)
{
    NOTIFY_USER_IRQ(SEC_IRQn);
}

void __isr_func_h WDT_IRQHandler(void)
{
    NOTIFY_USER_IRQ(WDT_IRQn);
}

void __attribute__((weak, section("isr_func"))) GMAC_IRQHandler(void)
{
    NOTIFY_USER_IRQ(GMAC_IRQn);
}

void __attribute__((weak, section("isr_func"))) IPC_RxCh0_IRQHandler(void)
{
    NOTIFY_USER_IRQ(IPC_RxCh0_IRQn);
}

void __isr_func_h SSP1_IRQHandler(void)
{
    NOTIFY_USER_IRQ(SSP1_IRQn);
}

void __isr_func_h DMA_IRQHandler(void)
{
    NOTIFY_USER_IRQ(DMA_IRQn);
}

void __isr_func_h SSP2_IRQHandler(void)
{
    NOTIFY_USER_IRQ(SSP2_IRQn);
}

void __isr_func_h SSP0_IRQHandler(void)
{
    NOTIFY_USER_IRQ(SSP0_IRQn);
}

void __attribute__((weak, section("isr_func")))  IPC_RxCh1_IRQHandler(void)
{
    NOTIFY_USER_IRQ(IPC_RxCh1_IRQn);
}

void __isr_func_h TIM0_IRQHandler(void)
{
    NOTIFY_USER_IRQ(TIM0_IRQn);
}

void __isr_func_h TIM1_IRQHandler(void)
{
    NOTIFY_USER_IRQ(TIM1_IRQn);
}

void __isr_func_h TIM2_IRQHandler(void)
{
    NOTIFY_USER_IRQ(TIM2_IRQn);
}

void __isr_func_h TIM3_IRQHandler(void)
{
    NOTIFY_USER_IRQ(TIM3_IRQn);
}

void __isr_func_h I2C_IRQHandler(void)
{
    NOTIFY_USER_IRQ(I2C_IRQn);
}

void __isr_func_h UART4_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART4_IRQn);
}

void __isr_func_h UART5_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART5_IRQn);
}

void __isr_func_h UART0_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART0_IRQn);
}

void __isr_func_h UART1_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART1_IRQn);
}

void __isr_func_h UART2_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART2_IRQn);
}

void __isr_func_h UART3_IRQHandler(void)
{
    NOTIFY_USER_IRQ(UART3_IRQn);
}

static uint32_t
_hal_get_package_type(void)
{
#if 0
    // TODO: H/w suppurt for reporting package type
    #define HAL_PACKAGE_TYPE_PIN    x
    uint32_t    *pEfuse_data = (uint32_t*)0x540E0000;

    return (IS_BIT_SET(*pEfuse_data, HAL_PACKAGE_TYPE_PIN))
           ? HAL_PACKAGE_LQFP128 : HAL_PACKAGE_QFN68;
#else
    return HAL_PACKAGE_LQFP128;
#endif
}

#if defined(CONFIG_ENABLE_EXCEPTION_INFO)
#include <stdarg.h>
#include <string.h>

// BFSR flags
#define IBUSERR         (0x1ul << 0)
#define PRECISERR       (0x1ul << 1)
#define IMPRECISERR     (0x1ul << 2)
#define UNSTKERR        (0x1ul << 3)
#define STKERR          (0x1ul << 4)
#define BFARVALID       (0x1ul << 7)

// UFSR flags
#define UNDEFINSTR      (0x1ul << 0)
#define INVSTATE        (0x1ul << 1)
#define INVPC           (0x1ul << 2)
#define NOCP            (0x1ul << 3)
#define UNALIGNED       (0x1ul << 8)
#define DIVBYZERO       (0x1ul << 9)

// MMFSR flags
#define IACCVIOL        (0x1ul << 0)
#define DACCVIOL        (0x1ul << 1)
#define MUNSTKERR       (0x1ul << 3)
#define MSTKERR         (0x1ul << 4)
#define MMARVALID       (0x1ul << 7)

#define UART0_CSR_BASE          0x54010000ul

/**
 *  Control/Status Register (CSR) of uart
 */
typedef struct uart_csr
{
    union {
        volatile uint32_t RBR;      /* Receiver Buffer Register (RBR, Offset: 0x00 for Read) */
        volatile uint32_t THR;      /* Transmitter Holding Register (THR, Offset: 0x00 for Write) */
        volatile uint32_t DLL;      /* Baud Rate Divisor Latch LSB (DLL, Offset: 0x00 when DLAB = 1) */
    };

    union {
        volatile uint32_t IER;      /* Interrupt Enable Register (IER, Offset: 0x04) */
        volatile uint32_t DLM;      /* Baud Rate Divisor Latch MSB (DLM, Offset: 0x04 when DLAB = 1) */
    };

    union {
        volatile uint32_t IIR;      /* Interrupt Identification Register (IIR, Offset: 0x08) */
        volatile uint32_t FCR;      /* FIFO Control Register (FCR, Offset: 0x08 for Write) */
        volatile uint32_t PSR;      /* Prescaler Register (PSR, Offset: 0x08 when DLAB = 1) */
    };

    volatile uint32_t LCR;          /* Line Control Register (LCR, Offset: 0x0C) */
    volatile uint32_t MCR;          /* Modem Control Register (MCR, Offset: 0x10) */

    union {
        volatile uint32_t LSR;      /* Line Status Register (LSR, Offset: 0x14 for Read) */
        volatile uint32_t TST;      /* Testing Register (TST, Offset: 0x14 for Write) */
    };

    volatile uint32_t MSR;          /* Modem Status Register (MSR, Offset: 0x18) */
    volatile uint32_t SPR;          /* Scratch Pad Register (SPR, Offset: 0x1C) */
} uart_csr_t;

static char                 g_msg[64] __attribute__ ((aligned (4))) = {0};

static void
_hal_uint2strhex(char *pStr, unsigned int number, const char nibbles_to_print)
{
#define MAX_NIBBLES     (8)
    int     nibble = 0;
    char    nibbles = nibbles_to_print;

    if( (unsigned)nibbles > MAX_NIBBLES )
        nibbles = MAX_NIBBLES;

    while( nibbles > 0 )
    {
        nibbles--;
        nibble = (int)(number >> (nibbles * 4)) & 0x0F;
        if( nibble <= 9 )
            pStr[strlen(pStr)] = (char)('0' + nibble);
        else
            pStr[strlen(pStr)] = (char)('A' - 10 + nibble);
    }
    return;
}

static int
_hal_exp_dump_init(void)
{
    uint32_t    div = 0ul;
    uart_csr_t  *pCSR = (uart_csr_t*)UART0_CSR_BASE;

	div = SystemGetCoreClock() / (115200 * 16);
	pCSR->LCR |= (0x1 << 7);
	pCSR->DLL = (div & 0xFF);
	pCSR->DLM = div >> 8;
	pCSR->PSR = 1;
	pCSR->LCR &= ~(0x1 << 7);
	pCSR->LCR = 0x3;
    return 0;
}

static int
_hal_exp_dump_out(char *pMsg, int len)
{
    uart_csr_t  *pCSR = (uart_csr_t*)UART0_CSR_BASE;

    while( len )
    {
        while( (pCSR->LSR & 0x20) == 0 ) {}

        pCSR->THR = *pMsg++;
        len--;
    }
    return 0;
}

static void
_hal_exp_log_out(const char* format, ...)
{
    char    *pch = (char*)format;
    va_list va;
    va_start(va, format);

    do {
        if( !pch ) break;

        while (*pch)
        {
            /* format identification character */
            if( *pch == '%' )
            {
                pch++;

                if( pch )
                {
                    switch( *pch )
                    {
                        case 'x':
                            {
                                const unsigned int number = va_arg(va, unsigned int);

                                for(int j = 0; j < sizeof(g_msg); j += 4)
                                    *(uint32_t*)&g_msg[j] = 0;

                                strcpy((void*)&g_msg[strlen(g_msg)], (void*)"0x");
                                _hal_uint2strhex(g_msg, number, 8);
                                _hal_exp_dump_out(g_msg, strlen(g_msg));
                            }
                            break;
                        case 's':
                            {
                                char *string = va_arg(va, char *);
                                _hal_exp_dump_out(string, strlen(string));
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                _hal_exp_dump_out(pch, 1);
            }

            pch++;
        }
    } while(0);

    va_end(va);
    return;
}

static void
_usage_err(uint32_t CFSR_Value)
{
    uint32_t    UFSR = ((CFSR_Value & SCB_CFSR_USGFAULTSR_Msk) >> SCB_CFSR_USGFAULTSR_Pos);

    _hal_exp_log_out("\n>>> Usage fault: (CFSR= %x, UFSR= %x)\n", CFSR_Value, UFSR);
    _hal_exp_log_out("%s%s%s%s%s%s",
                    (UFSR & DIVBYZERO)  ? "  Divide by zero UsageFault\n" : "",
                    (UFSR & UNALIGNED)  ? "  Un-aligned access UsageFault\n" : "",
                    (UFSR & NOCP)       ? "  No coprocessor UsageFault\n" : "",
                    (UFSR & INVPC)      ? "  Invalid PC load UsageFault\n" : "",
                    (UFSR & INVSTATE)   ? "  Invalid state UsageFault\n" : "",
                    (UFSR & UNDEFINSTR) ? "  Undefined instruction UsageFault\n" : "");
    return;
}

static void
_bus_fault_err(uint32_t CFSR_Value)
{
    uint32_t    BFSR = ((CFSR_Value & SCB_CFSR_BUSFAULTSR_Msk) >> SCB_CFSR_BUSFAULTSR_Pos);

    _hal_exp_log_out("\n>>> Bus fault: (CFSR = %x, BFSR= %x)\n", CFSR_Value, BFSR);
    _hal_exp_log_out("%s%s%s%s%s%s",
                    (BFSR & IBUSERR)     ? "  IBUSERR\n" : "",
                    (BFSR & PRECISERR)   ? "  Precise Data Bus Error\n" : "",
                    (BFSR & IMPRECISERR) ? "  Imprecise Data Bus Error\n" : "",
                    (BFSR & UNSTKERR)    ? "  Unstacking Error\n" : "",
                    (BFSR & STKERR)      ? "  Stacking error\n" : "",
                    (BFSR & BFARVALID)   ? "  Bus Fault Address Register Valid\n" : "");
    return;
}

static void
_mem_mgt_err(uint32_t CFSR_Value)
{
    uint32_t    MMFSR = (CFSR_Value & SCB_CFSR_MEMFAULTSR_Msk);

    _hal_exp_log_out("\n>>> Mem Mgt fault: (CFSR = %x, MMFSR= %x)\n", CFSR_Value, MMFSR);
    _hal_exp_log_out("%s%s%s%s%s",
                    (MMFSR & IACCVIOL)  ? "  Instruction Access Violation\n" : "",
                    (MMFSR & DACCVIOL)  ? "  Data Access Violation\n" : "",
                    (MMFSR & MUNSTKERR) ? "  Memory Unstacking Error\n" : "",
                    (MMFSR & MSTKERR)   ? "  Memory Stacking Error\n" : "",
                    (MMFSR & MMARVALID) ? "  MMARVALID\n" : "");
    return;
}

static void
_dbg_fault(uint32_t DFSR_Value)
{
    _hal_exp_log_out("\n>>> Debug Fault: (DFSR = %x)\n", DFSR_Value);
    _hal_exp_log_out("%s%s%s%s%s",
                    (DFSR_Value & SCB_DFSR_EXTERNAL_Msk) ? "  EXTERNAL\n" : "",
                    (DFSR_Value & SCB_DFSR_VCATCH_Msk)   ? "  VCATCH\n" : "",
                    (DFSR_Value & SCB_DFSR_DWTTRAP_Msk)  ? "  DWTTRAP\n" : "",
                    (DFSR_Value & SCB_DFSR_BKPT_Msk)     ? "  BKPT\n" : "",
                    (DFSR_Value & SCB_DFSR_HALTED_Msk)   ? "  HALTED\n" : "");
    return;
}

void __attribute__((used))
_hal_hard_fault_handler(uint32_t *sp)
{
    /**
     *  These are volatile to try and prevent the compiler/linker optimising them
     *  away as the variables never actually get used.  If the debugger won't show the
     *  values of the variables, make them global my moving their declaration outside of this function.
     */
    volatile uint32_t   r0, r1, r2, r3;
    volatile uint32_t   r12;
    volatile uint32_t   lr; /* Link register. */
    volatile uint32_t   pc; /* Program counter. */
    volatile uint32_t   psr;/* Program status register. */

    volatile uint32_t cfsr  = SCB->CFSR;
    volatile uint32_t hfsr  = SCB->HFSR;
    volatile uint32_t mmfar = SCB->MMFAR;
    volatile uint32_t bfar  = SCB->BFAR;
    volatile uint32_t dfsr  = SCB->DFSR;

    _hal_exp_log_out("\n=========\nHard Fault:\n"
                      "SCB->CFSR  = %x\n",
                      "SCB->HFSR  = %x\n",
                      "SCB->MMFAR = %x\n",
                      "SCB->BFAR  = %x\n\n",
                      cfsr, hfsr, mmfar, bfar);

    if( (hfsr & SCB_HFSR_FORCED_Msk) )
    {
        if( (cfsr & SCB_CFSR_USGFAULTSR_Msk) )
            _usage_err(cfsr);

        if( (cfsr & SCB_CFSR_BUSFAULTSR_Msk) )
            _bus_fault_err(cfsr);

        if( (cfsr & SCB_CFSR_MEMFAULTSR_Msk) )
            _mem_mgt_err(cfsr);
    }

    _dbg_fault(dfsr);

    r0  = sp[0];
    r1  = sp[1];
    r2  = sp[2];
    r3  = sp[3];
    r12 = sp[4];
    lr  = sp[5];
    pc  = sp[6];
    psr = sp[7];

    _hal_exp_log_out("=================== Registers information ====================\n"
                     "  R0 : %x  R1 : %x  R2 : %x  R3 : %x\n"
                     "  R12: %x  LR : %x  PC : %x  PSR: %x\n"
                     "==============================================================\n",
                     r0, r1, r2, r3, r12, lr, pc, psr);

    while(1);
    return;
}

void exception_dump(void) __attribute__((naked, weak));
void exception_dump(void)
{
    _hal_exp_dump_init();

    __asm volatile
    (
        " tst lr, #4                \n"
        " ite eq                    \n"
        " mrseq r0, msp             \n"
        " mrsne r0, psp             \n"
        " b _hal_hard_fault_handler \n"
    );
}
#endif
//=============================================================================
//                  Public Function Definition
//=============================================================================
hal_errno_t
hal_init(void)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {

    } while(0);
    return rval;
}


hal_errno_t
hal_irq_register_isr(
    int             irq_id,
    cb_usr_isr_t    usr_callback)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    if( irq_id >= (int)SYS_IRQn && irq_id <= (int)UART3_IRQn )
        g_usr_isr_table[irq_id] = usr_callback;
    else if( irq_id == SysTick_IRQn )
        g_usr_isr_table[SLOT_USER_ISR_SYS_TICK] = usr_callback;
    else if( irq_id == PendSV_IRQn )
        g_usr_isr_table[SLOT_USER_ISR_PENDSV] = usr_callback;
    else if( irq_id == HardFault_IRQn )
        g_usr_isr_table[SLOT_USER_ISR_HARD_FAULT] = usr_callback;
    else if( irq_id == SVCall_IRQn )
        g_usr_isr_table[SLOT_USER_ISR_SVC] = usr_callback;
    else        rval = HAL_ERRNO_WRONG_PARAM;

    return rval;
}


hal_errno_t
hal_irq_enable(
    IRQn_Type   irq_id)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        if( irq_id >= IRQ_Total )
        {
            rval = HAL_ERRNO_WRONG_PARAM;
            break;
        }

        // Skip system IRQ of ARM Cortex M
        if( (int)irq_id < (int)SYS_IRQn )
            break;

        NVIC_EnableIRQ(irq_id);
        NVIC_ClearPendingIRQ(irq_id);

    } while(0);
    return rval;
}

hal_errno_t
hal_irq_disable(
    IRQn_Type   irq_id)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        if( irq_id < SYS_IRQn || irq_id >= IRQ_Total )
        {
            rval = HAL_ERRNO_WRONG_PARAM;
            break;
        }

        // Skip system IRQ of ARM Cortex M
        if( (int)irq_id < (int)SYS_IRQn )
            break;

        NVIC_DisableIRQ(irq_id);
        NVIC_ClearPendingIRQ(irq_id);

    } while(0);
    return rval;
}

hal_errno_t
hal_irq_clear(
    IRQn_Type   irq_id,
    void        *pHandle)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        if( irq_id < SYS_IRQn || irq_id >= IRQ_Total )
        {
            rval = HAL_ERRNO_WRONG_PARAM;
            break;
        }

        // Skip system IRQ of ARM Cortex M
        if( (int)irq_id < (int)SYS_IRQn )
            break;

        switch(irq_id)
        {
            case GPIO_IRQn:
                rval = hal_gpio_clear_irq((gpio_handle_t*)pHandle);
                break;
            case CRC_IRQn:
                break;
            case SEC_IRQn:
                break;
            case WDT_IRQn:
                rval = hal_wdt_clear_irq();
                break;
            case GMAC_IRQn:
                break;
            case IPC_RxCh0_IRQn:
            case IPC_RxCh1_IRQn:
                break;
            case DMA_IRQn:
                break;
            case TIM0_IRQn:
            case TIM1_IRQn:
            case TIM2_IRQn:
            case TIM3_IRQn:
                rval = hal_tmr_clear_irq((tmr_handle_t*)pHandle);
                break;
            case I2C_IRQn:
                break;
            case UART0_IRQn:
            case UART1_IRQn:
            case UART2_IRQn:
            case UART3_IRQn:
            case UART4_IRQn:
            case UART5_IRQn:
                rval = hal_uart_clear_irq((uart_handle_t*)pHandle);
                break;
            case SSP0_IRQn:
            case SSP1_IRQn:
            case SSP2_IRQn:
            default: break;
        }

    } while(0);
    return rval;
}

hal_errno_t
hal_verify_ic_package(
    IRQn_Type   irq_id)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        uint32_t    package_type = 0;

        if( irq_id < SYS_IRQn || irq_id >= IRQ_Total )
        {
            rval = HAL_ERRNO_NOT_SUPPORT;
            break;
        }

        package_type = _hal_get_package_type();
        if( package_type == HAL_PACKAGE_LQFP128 )
            break;

        switch(irq_id)
        {
        #if 0
            case CRC_IRQn:
                break;
            case SEC_IRQn:
                break;
            case WDT_IRQn:
                break;
            case DMA_IRQn:
                break;
            case IPC_RxCh0_IRQn:
            case IPC_RxCh1_IRQn:
                break;
            case GPIO_IRQn:
                break;
            case TIM0_IRQn:
            case TIM1_IRQn:
            case TIM2_IRQn:
            case TIM3_IRQn:
                break;
            case SSP0_IRQn:
            case SSP1_IRQn:
            case SSP2_IRQn:
                break;
            case UART0_IRQn:
            case UART1_IRQn:
            case UART2_IRQn:
                break;
        #endif
            case GMAC_IRQn:
            case I2C_IRQn:
            case UART3_IRQn:
            case UART4_IRQn:
            case UART5_IRQn:
                rval = HAL_ERRNO_NOT_SUPPORT;
                break;
            default: break;
        }
    } while(0);
    return rval;
}

hal_errno_t
hal_phy2vir(
    uintptr_t   phy_flash_addr,
    uintptr_t   *pVir_addr)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do{
        uint32_t    mmp_type = -1;

        hal_scu_get_mmp_type(&mmp_type);

        if( mmp_type == 0 )
        {
            long        value = 0ul;
            uint32_t    offset = 0;
            fc_attr_t   fc_attr = { .space_bytes = 0ul, };

            if( hal_fc_get_flash_attr(&fc_attr) )
            {
                rval = HAL_ERRNO_UNKNOWN_DEVICE;
                break;
            }

            hal_scu_get_boot_offset(&offset);

            value = (phy_flash_addr & ~0xF0000000ul) - offset;
            *(pVir_addr) = (uintptr_t)((value < 0) ? value + fc_attr.space_bytes : value);
        }
        else
        {
            *(pVir_addr) = ((phy_flash_addr) & ~0xF0000000ul) + 0x60000000ul;
        }
    }while(0);
    return rval;
}

hal_errno_t
hal_vir2phy(
    uintptr_t   vir_flash_addr,
    uintptr_t   *pPhy_flash_addr)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do{
        uint32_t    mmp_type = -1;

        hal_scu_get_mmp_type(&mmp_type);

        if( mmp_type == 0 )
        {
            long        value = 0ul;
            uint32_t    offset = 0;
            fc_attr_t   fc_attr = { .space_bytes = 0ul, };

            if( hal_fc_get_flash_attr(&fc_attr) )
            {
                rval = HAL_ERRNO_UNKNOWN_DEVICE;
                break;
            }

            hal_scu_get_boot_offset(&offset);

            value = ((vir_flash_addr) & ~0xF0000000ul) + offset;
            *(pPhy_flash_addr) = (uintptr_t)((value < fc_attr.space_bytes) ? value : value - fc_attr.space_bytes);
        }
        else
        {
            *(pPhy_flash_addr) = ((vir_flash_addr) & ~0xF0000000ul) + 0x60000000ul;
        }
    }while(0);
    return rval;
}

uint32_t
hal_sys_get_core_clk(void)
{
    return SystemGetCoreClock();
}

uint32_t
hal_sys_get_cpu_clk(void)
{
    return SystemGetCoreClock() >> 1;
}

uint32_t
hal_sys_get_pclk(void)
{
    return SystemGetCoreClock() >> 3;
}

uint32_t
hal_get_tick(void)
{
    return g_msTicks;
}

/* *INDENT-ON* */
