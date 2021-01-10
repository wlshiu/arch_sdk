/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_gpio.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_gpio_H_weThlowe_lEWT_H4O8_syCO_uKMla8ZnEugv__
#define __hal_gpio_H_weThlowe_lEWT_H4O8_syCO_uKMla8ZnEugv__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define GPIO_MAX_PIN_NUM    32

typedef enum gpio_err
{
    GPIO_ERR_OK             = 0,
    GPIO_ERR_NULL_POINTER   = (GPIO_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    GPIO_ERR_NO_INSTANCE    = (GPIO_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    GPIO_ERR_WRONG_PARAM    = (GPIO_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    GPIO_ERR_NOT_SUPPORT    = (GPIO_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),

} gpio_err_t;

/**
 *  GPIO direction mode
 */
typedef enum gpio_mode
{
    GPIO_MODE_IGNORE    = 0,
    GPIO_MODE_INPUT,            /*!< GPIO input direction */
    GPIO_MODE_OUTPUT,           /*!< GPIO output direction */

} gpio_mode_t;

/**
 *  GPIO IRQ trigger mode
 */
typedef enum gpio_trig
{
    GPIO_TRIG_IGNORE        = 0,
    GPIO_TRIG_HIGH_LEVEL,           /*!< high level trigger */
    GPIO_TRIG_LOW_LEVEL,            /*!< low level trigger */
    GPIO_TRIG_RISING,               /*!< rising edge trigger */
    GPIO_TRIG_FALLING,              /*!< falling edge trigger */
    GPIO_TRIG_BOTH,                 /*!< rising and falling edge trigger */

} gpio_trig_t;

/**
 *  GPIO data
 */
typedef enum gpio_data
{
    GPIO_DATA_IGNORE    = 0,
    GPIO_DATA_HIGH,
    GPIO_DATA_LOW,
} gpio_data_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  configuration of a GPIO port
 */
typedef struct gpio_port_config
{
    gpio_mode_t     mode[GPIO_MAX_PIN_NUM];
    gpio_trig_t     trig_mode[GPIO_MAX_PIN_NUM];
    gpio_data_t     data[GPIO_MAX_PIN_NUM];
} gpio_port_config_t;

/**
 *  GPIO handle
 */
typedef struct gpio_handle
{
    uint32_t    instance;   /*!< the instance of target module */
    uint16_t    port_id;    /*!< the port id of GPIO module */
    uint16_t    total_pins; /*!< the total pins of a GPIO port module */
    IRQn_Type   irq_id;     /*!< the IRQ id of target module */
} gpio_handle_t;
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
 *  @brief  hal_gpio_init (Non thread-safe)
 *
 *  @param [in] port_id             the port id of target gpio
 *  @param [in] pins_pull_en        enable ALL Pins pull up/down, normally it should be set to '0' (Don't use)
 *  @param [in] pins_pull_data      set ALL Pins to pull up (1) or pull down (0)
 *  @param [in] ppHGpio             the handler of a GPIO port
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_init(
    uint32_t        port_id,
    uint32_t        pins_pull_en,
    uint32_t        pins_pull_data,
    gpio_handle_t   **ppHGpio);


/**
 *  @brief  hal_gpio_deinit (Non thread-safe)
 *
 *  @param [in] ppHGpio         the handler of a GPIO port
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_deinit(
    gpio_handle_t   **ppHGpio);


/**
 *  @brief  hal_gpio_set_pin (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @param [in] pin             the pin id
 *  @param [in] mode            direction mode, reference enum gpio_mode
 *  @param [in] trig_mode       trigger mode, reference enum gpio_trig
 *  @param [in] data            data type, reference enum gpio_data
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_set_pin(
    gpio_handle_t   *pHGpio,
    uint32_t        pin,
    gpio_mode_t     mode,
    gpio_trig_t     trig_mode,
    gpio_data_t     data);


/**
 *  @brief  hal_gpio_set_port (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @param [in] pConfig         the configuration of a GPIO port,
 *                              reference struct gpio_port_config
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_set_port(
    gpio_handle_t       *pHGpio,
    gpio_port_config_t  *pConfig);


/**
 *  @brief  hal_gpio_disable_pin_irq (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @param [in] pin             pin id of a GPIO port
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_disable_pin_irq(
    gpio_handle_t   *pHGpio,
    uint32_t        pin);


/**
 *  @brief  hal_gpio_disable_port_irq (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @param [in] pin_mask        the mask of target pins of a GPIO port
 *                              ps. 1: the target pin to disable
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_disable_port_irq(
    gpio_handle_t   *pHGpio,
    uint32_t        pin_mask);


/**
 *  @brief  hal_gpio_get_input (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @param [in] pin_mask        the mask of target pins of a GPIO port
 *                              ps. 1: the target pin to detect input
 *  @return
 *      the input value form a GPIO port
 */
uint32_t
hal_gpio_get_input(
    gpio_handle_t   *pHGpio,
    uint32_t        pin_mask);


/**
 *  @brief  hal_gpio_get_irq (Non thread-safe)
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @return
 *      the IRQ flags of all pins of a GPIO port
 */
uint32_t
hal_gpio_get_irq(
    gpio_handle_t   *pHGpio);


/**
 *  @brief  hal_gpio_clear_irq (Non thread-safe)
 *              auto clear all IRQ flags of a GPIO port
 *
 *  @param [in] pHGpio          the handler of a GPIO port
 *  @return
 *      error code, reference enum gpio_err
 */
gpio_err_t
hal_gpio_clear_irq(
    gpio_handle_t   *pHGpio) __isr_func_n;


#ifdef __cplusplus
}
#endif

#endif
