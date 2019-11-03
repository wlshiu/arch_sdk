/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp_stm32f429_439.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/02
 * @license
 * @description
 */


#include "bsp.h"
#include "stm32f4xx.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_UART_BAUD_RATE       115200

#ifndef __unused
    #define __unused                    __attribute__ ((unused))
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_uart_init(void)
{
    USART_InitTypeDef       USART_InitStructure = {0};

    /** USARTx configured as follow:
     *   - BaudRate = 115200 baud
     *   - Word Length = 8 Bits
     *   - One Stop Bit
     *   - No parity
     *   - Hardware flow control disabled (RTS and CTS signals)
     *   - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate            = CONFIG_UART_BAUD_RATE;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);

    // enable USART3
    USART_Cmd(USART3, ENABLE);

    return;
}

static int
stm32f4x9_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // enable GPIOC, RCC clock of USART3
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // link  PC10 and PC11 to USART3
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    // set PC10 to TX
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // set PC11 to RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    _uart_init();
    return 0;
}

static int
stm32f4x9_deinit(void)
{
    return 0;
}

static int
stm32f4x9_stdout_char(int ch)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {}
    
    USART_SendData(USART3, (unsigned short int) ch);
    return 0;
}


static int
stm32f4x9_stdout_string(char *str, int length)
{
    int     len = 0;
    while (length)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {}
        USART_SendData(USART3, (unsigned short int) (*str));
        str++, length--, len++;
    }
    return len;
}

static int
stm32f4x9_stdin_str(char *pBuf, int length)
{
    int         byte_cnt = 0;
    return byte_cnt;
}

static int
stm32f4x9_set_gpio(void *pExtra)
{
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
bsp_desc_t      g_bsp_stm32f429_439 =
{
    .pf_init           = stm32f4x9_init,
    .pf_deinit         = stm32f4x9_deinit,
    .pf_stdout_char    = stm32f4x9_stdout_char,
    .pf_stdout_string  = stm32f4x9_stdout_string,
    .pf_stdin_str      = stm32f4x9_stdin_str,
    .pf_set_gpio       = stm32f4x9_set_gpio,
};
