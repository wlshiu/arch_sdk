/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_uart.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */

#ifndef __hal_uart_H_widQ6vYI_lbQd_HI5b_sh3f_udxb9Euc2r8R__
#define __hal_uart_H_widQ6vYI_lbQd_HI5b_sh3f_udxb9Euc2r8R__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

typedef enum uart_err
{
    UART_ERR_OK             = 0,
    UART_ERR_NULL_POINTER   = (UART_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    UART_ERR_NO_INSTANCE    = (UART_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    UART_ERR_WRONG_PARAM    = (UART_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    UART_ERR_NOT_SUPPORT    = (UART_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),

} uart_err_t;

typedef enum uart_parity
{
    UART_PARITY_NONE     = 0,     // No parity
    UART_PARITY_EVEN     = 1,     // Even parity
    UART_PARITY_ODD      = 2,     // Odd parity
    UART_PARITY_1        = 3,     // 1
    UART_PARITY_0        = 4,     // 0
} uart_parity_t;

typedef enum uart_word_len
{
    UART_WORD_LEN_5BITS     = 0,    // 5-bits data word length
    UART_WORD_LEN_6BITS     = 1,    // 6-bits data word length
    UART_WORD_LEN_7BITS     = 2,    // 7-bits data word length
    UART_WORD_LEN_8BITS     = 3,    // 8-bits data word length
} uart_word_len_t;

typedef enum uart_stop_bits
{
    UART_STOP_BITS_1  = 0,  // 1 stop bit
    UART_STOP_BITS_2  = 1,  // 2 stop bit
} uart_stop_bits_t;

typedef enum uart_opcode
{
    UART_OPCODE_SET_BAUDRATE = 1,
} uart_opcode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  uart initial configuration
 */
typedef struct uart_config
{
    /**
     *  the current system clock
     */
    uint32_t            system_clock;

    /**
     *  This member configures the UART communication baud rate.
     */
    uint32_t            baud_rate;

    /**
     *  Specifies the number of data bits transmitted or received in a frame.
     */
    uart_word_len_t     word_length;

    /**
     *  Specifies the number of stop bits transmitted.
     */
    uart_stop_bits_t    stop_bits;

    /**
     *  Specifies the parity mode.
     *  @note When parity is enabled, the computed parity is inserted
     *  at the MSB position of the transmitted data (9th bit when
     *  the word length is set to 9 data bits; 8th bit when the
     *  word length is set to 8 data bits).
     */
    uart_parity_t       parity;

#if 0
    /**
     *  Specifies whether the Receive or Transmit mode is enabled or disabled.
     */
    uint32_t            mode;

    /**
     *  Specifies whether the hardware flow control mode is enabled or disabled.
     */
    uint32_t            hw_flow_ctl;
#endif

    uint32_t            irq_priority; // 0: ignore

} uart_config_t;

typedef struct uart_argv
{
    hal_argv_t  argv[3];
} uart_argv_t;

/**
 *  uart handle
 */
typedef struct uart_handle
{
    uint32_t    instance;   /*!< the instance of target module */
    uint32_t    port_id;    /*!< the port id of target module */
    IRQn_Type   irq_id;     /*!< the IRQ id of target module */

} uart_handle_t;
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
 *  @brief  hal_uart_init (Non thread-safe)
 *
 *  @param [in] target_id   target uart id
 *  @param [in] pConfig     the configuration of uart module
 *  @param [in] ppHUart     the uart handler
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_init(
    uint32_t        target_id,
    uart_config_t   *pConfig,
    uart_handle_t   **ppHUart);

/**
 *  @brief  hal_uart_deinit (Non thread-safe)
 *
 *  @param [in] ppHUart         the uart handler
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_deinit(uart_handle_t **ppHUart);

/**
 *  @brief  hal_uart_send (Non thread-safe)
 *
 *  @param [in] pHUart      the uart handler
 *  @param [in] pData       the sending data buffer
 *  @param [in] length      data length of sending
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_send(
    uart_handle_t   *pHUart,
    uint8_t         *pData,
    uint32_t        length);

/**
 *  @brief  hal_uart_recv (Non thread-safe)
 *
 *  @param [in] pHUart      the uart handler
 *  @param [in] pData       the receiving data buffer
 *  @param [in] pLength     the receiving length
 *                              It should be assigned receiving buffer length
 *                              and this API will report the real receiving data length
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_recv(
    uart_handle_t   *pHUart,
    uint8_t         *pData,
    uint32_t        *pLength);

/**
 *  @brief  hal_uart_clear_irq (Non thread-safe)
 *
 *  @param [in] pHUart      the uart handler
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_clear_irq(
    uart_handle_t    *pHUart) __isr_func_n;



#ifdef __cplusplus
}
#endif

#endif
