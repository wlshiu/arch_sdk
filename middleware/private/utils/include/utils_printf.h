/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file utils_printf.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/02
 * @license
 * @description
 */

#ifndef __utils_printf_H_wj1Lr25h_lRJQ_HWtk_sKpt_uERTXtDEivet__
#define __utils_printf_H_wj1Lr25h_lRJQ_HWtk_sKpt_uERTXtDEivet__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup print api
 *  @{
 */

#include <stdarg.h>
#include "hal_errno.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef hal_errno_t (*cb_putchar)(int ch);
typedef int         (*cb_puts)(char *str, int len);
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void utils_vsprintf(char *pBuf, int maxlen, const char *fmt, va_list args);

/**
 *  @brief  Composes a string with the same text that would be printed if format was used on printf, but instead of being printed,
 *          the content is stored as a C string in the buffer pointed by pbuf
 *
 *  @param [in] pbuf            Pointer to a buffer where the resulting C-string is stored.
 *                              The buffer should have a size of at least buf_size characters.
 *  @param [in] buf_size        Maximum number of bytes to be used in the buffer.
 *                              The generated string has a length of at most buf_size - 1,
 *                              leaving space for the additional terminating null character.
 *  @param [in] fmt             C string that contains a format string that follows the same specifications as format in printf
 *  @param [in] ...             Depending on the format string
 *  @return
 *
 */
int utils_snprintf(char *pbuf, int buf_size, const char *fmt, ...);

/**
 *  @brief  Writes the C string pointed by format to the standard output (stdout).
 *  @param [in] fmt             C string that contains the text to be written to stdout.
 *  @param [in] ...             Depending on the format string
 *  @return
 *
 */
void utils_printf(const char *fmt, ...);

/**
 *  @brief      Output the C string pointed by str to the stdout
 *
 *  @param [in] str     C string to be printed
 *  @return
 *      > 0: ok
 *      < 0: fail
 */
int utils_puts(char *str);

/**
 *  @brief  register the standard output functions
 *
 *  @param [in] pf_putc     the callback of output a character
 *  @param [in] pf_puts     the callback of output string
 *  @return
 *
 */
void
utils_register_stdout(
    cb_putchar  pf_putc,
    cb_puts     pf_puts);


/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif
