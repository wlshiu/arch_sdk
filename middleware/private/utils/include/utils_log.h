/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file utils_log.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/02
 * @license
 * @description
 */

#ifndef __utils_log_H_wD93UcML_ldGs_HuJ5_s4iI_uifDkyuLDSwC__
#define __utils_log_H_wD93UcML_ldGs_HuJ5_s4iI_uifDkyuLDSwC__

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup system log api
 *  @{
 */

#include "utils.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  color of text
 */
#define LOG_BLACK               "\033[30m"
#define LOG_RED                 "\033[31m"
#define LOG_GREEN               "\033[32m"
#define LOG_YELLOW              "\033[33m"
#define LOG_BLUE                "\033[34m"
#define LOG_MAGENTA             "\033[35m"
#define LOG_CYAN                "\033[36m"
#define LOG_WHITE               "\033[37m"
#define LOG_DEFAULT             "\033[39m"
#define LOG_RESET               "\033[m"

typedef enum utils_log_type
{
    UTILS_LOG_ERR         = 0,
    UTILS_LOG_INFO,


    UTILS_LOG_MAX

} utils_log_type_t;

/**
 *  callback for recording log by user
 */
typedef int (*cb_log_rec_t)(utils_log_type_t type, char *pStr, long length);
//=============================================================================
//                  Macro Definition
//=============================================================================
#if defined(CONFIG_ENABLE_LOG)
    #define msg(type, str, ...)                 utils_log(type, str, ##__VA_ARGS__)
    #define info(str, ...)                      utils_log(UTILS_LOG_INFO, str, ##__VA_ARGS__)
    #define log_color(type, color, str, ...)    utils_log(type, color str LOG_RESET, ##__VA_ARGS__)
    #define info_color(color, str, ...)         utils_log(UTILS_LOG_INFO, color str LOG_RESET, ##__VA_ARGS__)
    #define err(str, ...)                       utils_log(UTILS_LOG_ERR, LOG_RED str LOG_RESET, ##__VA_ARGS__)
#else
    #define msg(type, str, ...)
    #define info(str, ...)
    #define log_color(type, color, str, ...)
    #define info_color(color, str, ...)
    #define err(str, ...)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

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
 *  @brief  initialize log system
 *              This function MUST be executed before the other log API
 *  @return
 *      none
 */
void
utils_log_init(void);

/**
 *  @brief  register recorder to save log message if necessary
 *
 *  @param [in] cb_recorder
 *  @return
 *      none
 */
void
utils_log_register_recorder(
    cb_log_rec_t    cb_recorder);

/**
 *  @brief  log message
 *
 *  @param [in] type    log type, reference enum utils_log_type
 *  @param [in] fmt     C string that contains the text to be written to stdout.
 *  @param [in] ...     Depending on the format string
 *  @return
 *      none
 */
void
utils_log(
    utils_log_type_t   type,
    const char          *fmt,
    ...);

/**
 *  @brief  enable log with type
 *
 *  @param [in] type        log type, reference enum utils_log_type
 *  @return
 *      none
 */
void
utils_log_on(
    utils_log_type_t   type);

/**
 *  @brief  disable log with type
 *
 *  @param [in] type        log type, reference enum utils_log_type
 *  @return
 *      none
 */
void
utils_log_off(
    utils_log_type_t   type);

/**
 *  @brief  show all log types are enable or not
 *
 *  @return
 *      none
 */
void
utils_log_status(void);

/**
 *  @brief  log memory data
 *
 *  @param [in] type            log type, reference enum utils_log_type
 *  @param [in] prefix          the prefix of log message
 *  @param [in] pAddr           start memory address
 *  @param [in] bytes           byte length
 *  @param [in] has_out_u32le   display with uint32 little endian or byte by byte
 *  @return
 *      none
 *
 */
void
utils_log_mem(
    utils_log_type_t   type,
    char                *prefix,
    uint8_t             *pAddr,
    int                 bytes,
    unsigned int        has_out_u32le);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
