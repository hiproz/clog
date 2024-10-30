#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "dirent.h"
#include <direct.h>
#include "stdlib.h"

#define PLATFORM_COMMON                0
#define PLATFORM_RDA8910_CSDK_OPENLUAT 1
#define PLATFORM_RDA8910_CSDK_NEOWAY   2
#define PLATFORM_RTT                   3

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

#ifndef snprintf
#define snprintf sprintf_s
#endif

// log level defination
// LL:clog level
#define LL_DBG  0  // debug
#define LL_WAR  1  // warrnings
#define LL_ERR  2  // errors
#define LL_RUN  3  // some important running logs
#define LL_NONE 4  // disable all the logs

///////////////////////// setting before build /////////////////////////
#define CLOG_BUF_SIZE   1024
#define LOG_LEVEL       LL_DBG
#define BUILD_LOG_LEVEL LL_DBG
#define ENABLE_DATETIME 0
////////////////////////////////////////////////////////
#if ENABLE_DATETIME
extern void clog_get_datetime(unsigned char* datetime);
#endif

// clang-format off
#if ENABLE_DATETIME
#define CLOG_FORMAT_WITH_TIME { \
	unsigned char datetime[6] = { 0 }; \
	clog_get_datetime(datetime); \
	snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%02u/%02u/%02u %02u:%02u:%02u] ", \
	datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0]); \
	}
#else 
#define CLOG_FORMAT_WITH_TIME
#endif

#define CLOG_FORMAT { \
	if (clog_init()) break; \
	memset(clog_buf, 0, CLOG_BUF_SIZE); \
	CLOG_FORMAT_WITH_TIME \
	const char *p = strrchr(__FILE__, '\\'); \
	if (p != NULL) { \
	snprintf(clog_buf + strlen(clog_buf), CLOG_BUF_SIZE - 1, "[%s:%d] ", strrchr(__FILE__, '\\') + 1, __LINE__); \
	} else { \
		const char *p_linux = strrchr(__FILE__, '/'); \
		if (p_linux != NULL) { \
			snprintf(clog_buf + strlen(clog_buf), CLOG_BUF_SIZE - 1, "[%s:%d] ", p_linux + 1, __LINE__); \
		} \
		else { \
				snprintf(clog_buf + strlen(clog_buf), CLOG_BUF_SIZE - 1, "[%s:%d] ", __FILE__, __LINE__); \
			} \
	} \
}

#define MACRO_SPLICE(x,y) x##y

// debug log
#if (LL_DBG >= BUILD_LOG_LEVEL)
#define clog_dbg(fmt,...)         do{CLOG_FORMAT _clog(LL_DBG, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_dbg(data, len)   do{CLOG_FORMAT _clog_hex(LL_DBG, data, len);}while(0)
#define clog_mix_dbg(data, len)   do{CLOG_FORMAT _clog_mix(LL_DBG, data, len);}while(0)
#else
#define clog_dbg(...)
#endif

#if (LL_WAR >= BUILD_LOG_LEVEL)
#define clog_war(fmt,...)         do{CLOG_FORMAT _clog(LL_WAR, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_war(data, len)   do{CLOG_FORMAT _clog_hex(LL_WAR, data, len);}while(0)
#define clog_mix_war(data, len)   do{CLOG_FORMAT _clog_mix(LL_WAR, data, len);}while(0)
#else
#define clog_war(...)
#endif

// error log
#if (LL_ERR >= BUILD_LOG_LEVEL)
#define clog_err(fmt,...)         do{CLOG_FORMAT _clog(LL_ERR, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_err(data, len)   do{CLOG_FORMAT _clog_hex(LL_ERR, data, len);}while(0)
#define clog_mix_err(data, len)   do{CLOG_FORMAT _clog_mix(LL_ERR, data, len);}while(0)
#else
#define clog_err(...)
#define clog_hex_err(...)
#define clog_mix_err(...)
#endif

// running log
#if (LL_RUN >= BUILD_LOG_LEVEL)
#define clog_run(fmt,...)		do{CLOG_FORMAT _clog(LL_RUN, fmt, ##__VA_ARGS__);}while(0)
#define clog_hex_run(data, len) do{CLOG_FORMAT _clog_hex(LL_RUN, data, len);}while(0)
#define clog_mix_run(data, len) do{CLOG_FORMAT _clog_mix(LL_RUN, data, len);}while(0)
#else
#define clog_run(...)
#endif
// clang-format on

void _clog(int log_level, char* fmt, ...);
void _clog_hex(int level, uint8_t* data, uint16_t len);
void _clog_mix(int level, uint8_t* data, uint16_t len);
///////////////////////////////////////////////////////////////////////
int  clog_init(void);
void clog_set_level(int level);
int  clog_get_level(void);
void clog_set_file_para(int enable, char* save_dir_path, int file_num, int file_size);
////////////////////////////////////////////
extern char* clog_buf;

#ifdef __cplusplus
}
#endif

#endif
