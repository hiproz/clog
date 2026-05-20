#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__) && defined(_WIN32)
#include "dirent.h"
#include <direct.h>
#endif

#define TRUE  1
#define FALSE 0

/** Supported output platforms */
#define PLATFORM_COMMON                0
#define PLATFORM_RDA8910_CSDK_OPENLUAT 1
#define PLATFORM_RDA8910_CSDK_NEOWAY   2
#define PLATFORM_RTT                   3

// When FALSE, logs go only to the console (no on-device file storage).
#define SUPPORT_LOC_SAVE FALSE

/** Platform-specific low-level print hook (clog_printf). */
#define SDK_PLATFORM PLATFORM_COMMON
#if (SDK_PLATFORM == PLATFORM_RDA8910_CSDK_OPENLUAT)
#include "iot_debug.h"
#include "am_openat.h"
#define clog_printf IVTBL(print)
#elif (SDK_PLATFORM == PLATFORM_RDA8910_CSDK_NEOWAY)
extern void osiTraceBasic(unsigned tag, unsigned nargs, const char* fmt, ...);
#define clog_printf(fmt, ...) osiTraceBasic(3, 0, fmt, ##__VA_ARGS__)
#elif (SDK_PLATFORM == PLATFORM_RTT)
#define clog_printf(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#else
#define clog_printf printf
#endif

enum CLOG_LEVEL
{
    LL_DBG = 0,
    LL_WAR = 1,
    LL_ERR = 2,
    LL_RUN = 3,  // important running logs, always displayed when enabled
    LL_NONE = 4
};

///////////////////////// setting before build /////////////////////////
#define CLOG_BUF_SIZE        256
#define CLOG_RESERVED_PREFIX 64
#define CLOG_HEX_MAX_BYTES   (((CLOG_BUF_SIZE) > (CLOG_RESERVED_PREFIX)) ? (((CLOG_BUF_SIZE) - (CLOG_RESERVED_PREFIX)) / 3) : 0)
#define BUILD_LOG_LEVEL      LL_DBG
#define ENABLE_DATETIME      FALSE
////////////////////////////////////////////////////////
#if ENABLE_DATETIME
/**
 * Fill datetime[6] for the log prefix [YY/MM/DD HH:MM:SS]:
 *   [0]=second, [1]=minute, [2]=hour, [3]=day, [4]=month, [5]=year (00-99).
 * Provided in clog.c when SDK_PLATFORM is PLATFORM_COMMON; implement on other platforms.
 */
void clog_get_datetime(unsigned char* datetime);
#endif

typedef struct {
    int   clog_level;
    char* clog_buf;
    char  clog_file_path[128];
    int   clog_file_num;
    int   clog_file_size;
    FILE* current_file;
    int   current_file_size;
    int   current_file_num;
    int   current_file_index;
} clog_inf_t;

// clang-format off
/** Prefix in log->clog_buf: [datetime] (if ENABLE_DATETIME) then [file:line]. Ends with ';' for use inside do{ ... }while(0). */
#define CLOG_FORMAT(log) clog_format_prefix((log), __FILE__, __LINE__);

#define MACRO_SPLICE(x,y) x##y

#if (LL_DBG >= BUILD_LOG_LEVEL)
#define clog_dbg(fmt,...)         do{CLOG_FORMAT((&default_log)); _clog((&default_log), LL_DBG, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_dbg(data, len)   do{CLOG_FORMAT((&default_log)); _clog_hex((&default_log), LL_DBG, data, len);}while(0)
#define clog_mix_dbg(data, len)   do{CLOG_FORMAT((&default_log)); _clog_mix((&default_log), LL_DBG, data, len);}while(0)
#else
#define clog_dbg(...)
#define clog_hex_dbg(...)
#define clog_mix_dbg(...)
#endif

#if (LL_WAR >= BUILD_LOG_LEVEL)
#define clog_war(fmt,...)         do{CLOG_FORMAT((&default_log)); _clog((&default_log), LL_WAR, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_war(data, len)   do{CLOG_FORMAT((&default_log)); _clog_hex((&default_log), LL_WAR, data, len);}while(0)
#define clog_mix_war(data, len)   do{CLOG_FORMAT((&default_log)); _clog_mix((&default_log), LL_WAR, data, len);}while(0)
#else
#define clog_war(...)
#define clog_hex_war(...)
#define clog_mix_war(...)
#endif

#if (LL_ERR >= BUILD_LOG_LEVEL)
#define clog_err(fmt,...)         do{CLOG_FORMAT((&default_log)); _clog((&default_log), LL_ERR, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_err(data, len)   do{CLOG_FORMAT((&default_log)); _clog_hex((&default_log), LL_ERR, data, len);}while(0)
#define clog_mix_err(data, len)   do{CLOG_FORMAT((&default_log)); _clog_mix((&default_log), LL_ERR, data, len);}while(0)
#else
#define clog_err(...)
#define clog_hex_err(...)
#define clog_mix_err(...)
#endif

#if (LL_RUN >= BUILD_LOG_LEVEL)
#define clog_run(fmt,...)         do{CLOG_FORMAT((&default_log)); _clog((&default_log), LL_RUN, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_run(data, len)   do{CLOG_FORMAT((&default_log)); _clog_hex((&default_log), LL_RUN, data, len);}while(0)
#define clog_mix_run(data, len)   do{CLOG_FORMAT((&default_log)); _clog_mix((&default_log), LL_RUN, data, len);}while(0)
#else
#define clog_run(...)
#define clog_hex_run(...)
#define clog_mix_run(...)
#endif

#define clog_non(...)
// clang-format on

void clog_format_prefix(clog_inf_t* log, const char* file, int line);

void _clog(clog_inf_t* log, int level, const char* fmt, ...);
void _clog_hex(clog_inf_t* log, int level, const uint8_t* data, uint16_t len);
void _clog_mix(clog_inf_t* log, int level, const uint8_t* data, uint16_t len);

void _clog_custom(clog_inf_t* log, void* func, int log_level, const char* fmt, ...);
void _clog_hex_custom(clog_inf_t* log, void* func, int level, const uint8_t* data, uint16_t len);

int  clog_init(clog_inf_t* log, int level);
void clog_deinit(clog_inf_t* log);
int  clog_get_level(clog_inf_t* log);
#if (SUPPORT_LOC_SAVE)
void clog_set_file_para(int enable, char* save_dir_path, int file_num, int file_size);
#endif

/** Global instances — not thread-safe; serialize or use per-task instances + locks. */
extern clog_inf_t default_log;
extern clog_inf_t target_log;

#ifdef __cplusplus
}
#endif

#endif
