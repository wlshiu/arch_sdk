/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_gpio.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include "hal_gpio.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of GPIO port 0
 */
#define GPIO0_CSR_BASE  0x54050000ul

/**
 *  Pins of a GPIO port
 */
#define GPIO_PIN_MAX        GPIO_MAX_PIN_NUM

typedef enum gpio_port
{
    GPIO_PORT_0     = 0,
    GPIO_PORT_MAX
} gpio_port_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  Control/Status Register (CSR) of GPIO
 */
typedef struct gpio_csr
{
    volatile uint32_t DataOut;          // DOUT;     // 0x00 (rw) data output
    volatile uint32_t DataIn;           // DIN;      // 0x04 (rw) data input
    volatile uint32_t PinDir;           // PDIR;     // 0x08 (rw) direction
    volatile uint32_t PinBypass;        // BYPASS;   // 0x0c (rw) bypass
    volatile uint32_t DataSet;          // DSET;     // 0x10 (rw) data bit set
    volatile uint32_t DataClear;        // DCLR;     // 0x14 (rw) data bit clear
    volatile uint32_t PinPullEnable;    // PUEN;     // 0x18 (rw) pull up
    volatile uint32_t PinPullType;      // PTYP;     // 0x1c (rw) pull high/low
    volatile uint32_t IntrEnable;       // IER;      // 0x20 (rw) interrupt enable
    volatile uint32_t IntrRawState;     // SR;       // 0x24 (rw) interrupt status (raw)
    volatile uint32_t IntrMaskedState;  // IMSR;     // 0x28 (rw) interrupt status (masked)
    volatile uint32_t IntrMask;         // IMR;      // 0x2c (rw) interrupt mask
    volatile uint32_t IntrClear;        // ICR;      // 0x30 (rw) interrupt clear
    volatile uint32_t IntrTrigger;      // ITR;      // 0x34 (rw) interrupt trigger method
    volatile uint32_t IntrBoth;         // IBOTH;    // 0x38 (rw) interrupt trigger edge (single or both)
    volatile uint32_t IntrRiseNeg;      // IRN;      // 0x3c (rw) interrupt trigger edge (rising or falling)
    volatile uint32_t BounceEn;         // BER;      // 0x40 (rw) de-bouncing clock prescaling enable
    volatile uint32_t BouncePreScale;   // BPSR;     // 0x44 (rw) de-bouncing clock pre-scaler
} gpio_csr_t;

typedef struct gpio_dev
{
    gpio_handle_t   handle;
} gpio_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static const gpio_dev_t     g_gpio_dev[GPIO_PORT_MAX] =
{
    [GPIO_PORT_0] = { .handle = { .instance = GPIO0_CSR_BASE, .port_id = GPIO_PORT_0, .irq_id = GPIO_IRQn, .total_pins = GPIO_PIN_MAX, }, },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
gpio_err_t
hal_gpio_init(
    uint32_t        port_id,
    uint32_t        pins_pull_en,
    uint32_t        pins_pull_data,
    gpio_handle_t   **ppHGpio)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        gpio_csr_t      *pCSR = 0;

        if( !ppHGpio )
        {
            rval = GPIO_ERR_WRONG_PARAM;
            break;
        }

        *ppHGpio = 0;

        if( port_id > (GPIO_PORT_MAX - 1) )
        {
            rval = GPIO_ERR_NO_INSTANCE;
            break;
        }

        pCSR = (gpio_csr_t*)g_gpio_dev[port_id].handle.instance;

        pCSR->PinDir        = 0;  // default set to input mode
        pCSR->PinPullType   = pins_pull_data;
        pCSR->PinPullEnable = pins_pull_en;

        *ppHGpio = (gpio_handle_t*)&g_gpio_dev[port_id].handle;
    } while(0);
    return rval;
}

gpio_err_t
hal_gpio_deinit(
    gpio_handle_t   **ppHGpio)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        gpio_dev_t  *pDev = 0;
        gpio_csr_t  *pCSR = 0;

        if( !ppHGpio || !(*ppHGpio) ) break;

        pDev = DOWN_CAST(gpio_dev_t, *ppHGpio, handle);
        pCSR = (gpio_csr_t*)pDev->handle.instance;

        pCSR->PinPullEnable = 0;
        pCSR->IntrEnable    = 0;
        pCSR->IntrClear     = 0xFFFFFFFF;

        hal_irq_disable(pDev->handle.irq_id);
    } while(0);
    return rval;
}

gpio_err_t
hal_gpio_set_pin(
    gpio_handle_t   *pHGpio,
    uint32_t        pin,
    gpio_mode_t     mode,
    gpio_trig_t     trig_mode,
    gpio_data_t     data)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        gpio_csr_t      *pCSR = 0;
        uint32_t        is_enable_irq = 0;

        if( !pHGpio )
        {
            rval = GPIO_ERR_NULL_POINTER;
            break;
        }

        if( pin > (GPIO_PIN_MAX - 1) )
        {
            rval = GPIO_ERR_NO_INSTANCE;
            break;
        }

        pCSR = (gpio_csr_t*)pHGpio->instance;

        is_enable_irq = IS_BIT_SET(pCSR->IntrEnable, pin);

        CLR_BIT(pCSR->IntrEnable, pin);
        SET_BIT(pCSR->IntrClear, pin);

        // set direction
        if( mode != GPIO_MODE_IGNORE )
        {
            if( mode == GPIO_MODE_OUTPUT )
                SET_BIT(pCSR->PinDir, pin);
            else
                CLR_BIT(pCSR->PinDir, pin);
        }

        if( data != GPIO_DATA_IGNORE )
        {
            if( data == GPIO_DATA_HIGH )
                SET_BIT(pCSR->DataSet, pin);
            else
                SET_BIT(pCSR->DataClear, pin);
        }

        // set trigger mode
        if( trig_mode != GPIO_TRIG_IGNORE )
        {
            if( trig_mode == GPIO_TRIG_HIGH_LEVEL ||
                trig_mode == GPIO_TRIG_LOW_LEVEL )
                SET_BIT(pCSR->IntrTrigger, pin);
            else
            {
                CLR_BIT(pCSR->IntrTrigger, pin);

                if( trig_mode == GPIO_TRIG_BOTH )
                    SET_BIT(pCSR->IntrBoth, pin);
                else
                {
                    CLR_BIT(pCSR->IntrBoth, pin);

                    if( trig_mode == GPIO_TRIG_RISING )
                        CLR_BIT(pCSR->IntrRiseNeg, pin);
                    else
                        SET_BIT(pCSR->IntrRiseNeg, pin);
                }
            }

            is_enable_irq = 1;
        }

        // enable IRQ
        if( is_enable_irq )
            SET_BIT(pCSR->IntrEnable, pin);

    } while(0);
    return rval;
}


gpio_err_t
hal_gpio_set_port(
    gpio_handle_t       *pHGpio,
    gpio_port_config_t  *pConfig)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        gpio_csr_t      *pCSR = 0;
        uint32_t        irq_en_stat = 0;
        uint32_t        value_0 = 0;
        uint32_t        value_1 = 0;

        if( !pHGpio || !pConfig )
        {
            rval = GPIO_ERR_NULL_POINTER;
            break;
        }

        pCSR = (gpio_csr_t*)pHGpio->instance;

        irq_en_stat = pCSR->IntrEnable;
        pCSR->IntrEnable = 0; // disable all IRQ
        pCSR->IntrClear  = 0xFFFFFFFF;

        // set direction
        value_0 = pCSR->PinDir;
        for(int i = 0; i < GPIO_PIN_MAX; i++)
        {
            gpio_mode_t     mode = pConfig->mode[i];

            if( mode == GPIO_MODE_IGNORE )
                continue;

            if( mode == GPIO_MODE_OUTPUT )
                SET_BIT(value_0, i);
            else
                CLR_BIT(value_0, i);
        }

        pCSR->PinDir = value_0;

        // set data
        value_0 = 0;
        value_1 = 0;
        for(int i = 0; i < GPIO_PIN_MAX; i++)
        {
            char    data = pConfig->data[i];

            if( data == GPIO_DATA_IGNORE )
                continue;

            if( data == GPIO_DATA_HIGH )
                SET_BIT(value_0, i);
            else
                SET_BIT(value_1, i);
        }

        pCSR->DataSet   = value_0;
        pCSR->DataClear = value_1;

        // set trigger mode
        for(int i = 0; i < GPIO_PIN_MAX; i++)
        {
            gpio_trig_t     trig_mode = pConfig->trig_mode[i];

            if( trig_mode == GPIO_TRIG_IGNORE )
                continue;

            if( trig_mode == GPIO_TRIG_HIGH_LEVEL ||
                trig_mode == GPIO_TRIG_LOW_LEVEL )
                SET_BIT(pCSR->IntrTrigger, i);
            else
            {
                CLR_BIT(pCSR->IntrTrigger, i);

                if( trig_mode == GPIO_TRIG_BOTH )
                    SET_BIT(pCSR->IntrBoth, i);
                else
                {
                    CLR_BIT(pCSR->IntrBoth, i);

                    if( trig_mode == GPIO_TRIG_RISING )
                        CLR_BIT(pCSR->IntrRiseNeg, i);
                    else
                        SET_BIT(pCSR->IntrRiseNeg, i);
                }
            }

            // enable IRQ
            SET_BIT(irq_en_stat, i);
        }

        pCSR->IntrEnable = irq_en_stat;
    } while(0);
    return rval;
}

gpio_err_t
hal_gpio_disable_pin_irq(
    gpio_handle_t   *pHGpio,
    uint32_t        pin)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        gpio_csr_t      *pCSR = 0;

        if( !pHGpio )
        {
            rval = GPIO_ERR_NULL_POINTER;
            break;
        }

        if( pin > (GPIO_PIN_MAX - 1) )
        {
            rval = GPIO_ERR_NO_INSTANCE;
            break;
        }

        pCSR = (gpio_csr_t*)pHGpio->instance;

        CLR_BIT(pCSR->IntrEnable, pin);
        SET_BIT(pCSR->IntrClear, pin);
    } while(0);
    return rval;
}

gpio_err_t
hal_gpio_disable_port_irq(
    gpio_handle_t   *pHGpio,
    uint32_t        pin_mask)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    do {
        if( !pHGpio )
        {
            rval = GPIO_ERR_NULL_POINTER;
            break;
        }

        ((gpio_csr_t*)pHGpio->instance)->IntrEnable = ~pin_mask;
        ((gpio_csr_t*)pHGpio->instance)->IntrClear  = pin_mask;

    } while(0);
    return rval;
}

uint32_t
hal_gpio_get_input(
    gpio_handle_t   *pHGpio,
    uint32_t        pin_mask)
{
    uint32_t      rval = 0;
    do {
        if( !pHGpio ) break;

        rval = ((gpio_csr_t*)pHGpio->instance)->DataIn & pin_mask;
    } while(0);
    return rval;
}

uint32_t
hal_gpio_get_irq(
    gpio_handle_t   *pHGpio)
{
    uint32_t      rval = 0;
    do {
        if( !pHGpio ) break;

        rval = ((gpio_csr_t*)pHGpio->instance)->IntrRawState;
    } while(0);
    return rval;
}


gpio_err_t
hal_gpio_clear_irq(
    gpio_handle_t   *pHGpio)
{
    gpio_err_t      rval = GPIO_ERR_OK;
    if( pHGpio )
    {
        gpio_csr_t      *pCSR = (gpio_csr_t*)pHGpio->instance;
        uint32_t        value = pCSR->IntrClear;
        pCSR->IntrClear = value;
    }
    return rval;
}


