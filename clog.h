// https://github.com/hiproz/clog.git

#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "string.h"

//#define ENABLE_RTT // use rtt for clog
#ifdef ENABLE_RTT
#include "SEGGER_RTT.h"
#define clog_printf(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#endif

#define RDA8910_CSDK  // used in rda8910_csdk
#ifdef RDA8910_CSDK
#include "iot_debug.h"
#include "am_openat.h"
#define clog_printf IVTBL(print)
#endif

// log level defination
// LL:clog level
#define LL_INF  0  // some infos
#define LL_WAR  1  // warrnings
#define LL_ERR  2  // errors
#define LL_RUN  3  // some important running logs
#define LL_NONE 4  // disable all the logs

///////////////////////// setting before build /////////////////////////
// 发送bufer长度实际调整
#define CLOG_BUF_SIZE 512

// 日志开关
#define LOG_LEVEL LL_INF  // 日志级别，此级别下的log不打印

// 日志编译级别
#define BUILD_LOG_LEVEL LL_INF  // 编译级别，此级别下的log打印代码不参与编译,默认设置为RUN

// 日志是否支持显示RTC时间
#define ENABLE_DATETIME 0
////////////////////////////////////////////////////////
#if ENABLE_DATETIME  //depend on the platform 需要根据不同的平台适配
// it is variaty that every platform get the rtc time。
// 每个平台获取时间的实现都有所不同。
// datetime :为长度为6的字节数组，返回年月日时分秒
extern void clog_get_datetime(unsigned char* datetime);
#endif

// clang-format off
#define CLOG_FORMAT_WITH_TIME { \
  if(clog_init()) break; \
  unsigned char datetime[6]={0}; \
  clog_get_datetime(datetime); \
  memset(clog_buf, 0, CLOG_BUF_SIZE); \
  char *p = strrchr(__FILE__, '\\');  \
  if (p != NULL) { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%02u/%02u/%02u %02u:%02u:%02u] [%s:%d] ", \
    datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0], strrchr(__FILE__, '\\') + 1, __LINE__); \
  } else { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%02u/%02u/%02u %02u:%02u:%02u %s:%d] ", \
    datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0], strrchr(__FILE__, '/') + 1, __LINE__); \
  } \
}

#define CLOG_FORMAT { \
  if(clog_init()) break; \
  memset(clog_buf, 0, CLOG_BUF_SIZE); \
  char *p = strrchr(__FILE__, '\\'); \
  if (p != NULL) { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%s:%d] ", \
    strrchr(__FILE__, '\\') + 1, __LINE__); \
  } else { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%s:%d] ", \
    strrchr(__FILE__, '/') + 1, __LINE__); \
  } \
}



#ifdef RDA8910_CSDK
#define _MACRO_SPLICE(x) x
#define MACRO_SPLICE(x,y) _MACRO_SPLICE(x)_MACRO_SPLICE(y)
#else
#define _MACRO_SPLICE(x,y) x##y
#define MACRO_SPLICE(x,y) _MACRO_SPLICE(x,y)
#endif

// info log
#if (LL_INF >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_inf(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(LL_INF,fmt,##__VA_ARGS__))
#else
#define clog_inf(fmt,...)  do{MACRO_SPLICE(CLOG_FORMAT, _clog(LL_INF, fmt, ##__VA_ARGS__));}while(0)
#endif
#else
#define clog_inf(...)
#endif


// warning log
#if (LL_WAR >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_war(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(LL_WAR,fmt,##__VA_ARGS__))
#else
#define clog_war(fmt,...)  do{MACRO_SPLICE(CLOG_FORMAT, _clog(LL_WAR, fmt, ##__VA_ARGS__));}while(0)
#endif
#else
#define clog_war(...)
#endif

// error log
#if (LL_ERR >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_err(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(LL_ERR,fmt,##__VA_ARGS__))
#else
#define clog_err(fmt,...)  do{MACRO_SPLICE(CLOG_FORMAT, _clog(LL_ERR, fmt, ##__VA_ARGS__));}while(0)
#endif
#else
#define clog_err(...)
#endif

// running log
#if (LL_RUN >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_run(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(LL_RUN,fmt,##__VA_ARGS__))
#else
#define clog_run(fmt,...)  do{MACRO_SPLICE(CLOG_FORMAT, _clog(LL_RUN, fmt, ##__VA_ARGS__));}while(0)
#endif
#else
#define clog_run(...)
#endif
// clang-format on

void _clog(int log_level, char* fmt, ...);

///////////////////////////////////////////////////////////////////////
/**
 * @brief externed fuctions
 *
 */

/**
 * clog_inf();
 * clog_war();
 * clog_err();
 * clog_run();
 */

/**
 * @brief ini the clog resource
 *
 * @return int:0 success; other:failed
 */
int clog_init(void);

/**
 * @brief set the log level
 *
 * @param level reference to the macros :LL_INF...
 */
void clog_set_level(int level);

/**
 * @brief get the log level
 *
 * @return int
 */
int clog_get_level(void);
/**
 * @brief  show all the data to hex format
 *
 * @param level log levels,reference to the macros :LL_INF...
 * @param data: the data to show
 * @param len: the data's length
 */
void clog_hex(int level, uint8_t* data, uint16_t len);
/**
 * @brief: if the byte is char, show charactor, if not, show to hex
 *
* @param level log levels,reference to the macros :LL_INF...
 * @param data: the data to show
 * @param len: the data's length
 */
void clog_str_hex(int level, uint8_t* data, uint16_t len);

////////////////////////////////////////////
extern char* clog_buf;

#ifdef __cplusplus
}
#endif

#endif
