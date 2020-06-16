#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdio.h"
#include "string.h"

#define ENABLE_RTT
#ifdef ENABLE_RTT
#include "SEGGER_RTT.h"
#define clog_printf(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#endif

// log level defination
#define INF  0
#define WAR  1
#define ERR  2
#define RUN  3
#define UL1  4
#define UL2  5
#define UL3  6
#define NONE 7
#define DIS  8

///////////////////////// setting before build /////////////////////////
// 日志开关
#define LOG_LEVEL INF  // 日志级别，此级别下的log不打印
// 日志编译级别
#define BUILD_LOG_LEVEL INF  // 编译级别，此级别下的log打印代码不参与编译,默认设置为RUN
// 日志是否支持显示RTC时间
#define ENABLE_DATETIME 0
////////////////////////////////////////////////////////
////////////depend on the platform 需要根据不同的平台适配/////////////
#if ENABLE_DATETIME
// it is variaty that every platform get the rtc time。
// 每个平台获取时间的实现都有所不同。
// datetime :为长度为6的字节数组，返回年月日时分秒
extern void clog_get_datetime(unsigned char* datetime);
#endif
// 发送bufer长度实际调整
#define CLOG_BUF_SIZE 512
extern char clog_buf[CLOG_BUF_SIZE];
/////////////////////////////////////////////////////////
// clang-format off
#define CLOG_FORMAT_WITH_TIME { \
  unsigned char datetime[6]={0}; \
  clog_get_datetime(datetime); \
  memset(clog_buf, 0, sizeof(clog_buf)); \
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
  memset(clog_buf, 0, sizeof(clog_buf)); \
  char *p = strrchr(__FILE__, '\\');  \
  if (p != NULL) { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%s:%d] ", \
    strrchr(__FILE__, '\\') + 1, __LINE__); \
  } else { \
    snprintf(clog_buf, CLOG_BUF_SIZE - 1, "[%s:%d] ", \
    strrchr(__FILE__, '/') + 1, __LINE__); \
  } \
}


#define _MACRO_SPLICE(x,y) x##y

#define MACRO_SPLICE(x,y) _MACRO_SPLICE(x,y)

// info log
#if (INF >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_inf(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(INF,fmt,##__VA_ARGS__))
#else
#define clog_inf(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(INF,fmt,##__VA_ARGS__))
#endif
#else
#define clog_inf(...)
#endif

// warning log
#if (WAR >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_war(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(WAR,fmt,##__VA_ARGS__))
#else
#define clog_war(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(WAR,fmt,##__VA_ARGS__))
#endif
#else
#define clog_war(...)
#endif

// error log
#if (ERR >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_err(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(ERR,fmt,##__VA_ARGS__))
#else
#define clog_err(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(ERR,fmt,##__VA_ARGS__))
#endif
#else
#define clog_err(...)
#endif

// running log
#if (RUN >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_run(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(RUN,fmt,##__VA_ARGS__))
#else
#define clog_run(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(RUN,fmt,##__VA_ARGS__))
#endif
#else
#define clog_run(...)
#endif

// user define level 1
#if (UL1 >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_ul1(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(UL1,fmt,##__VA_ARGS__))
#else
#define clog_ul1(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(UL1,fmt,##__VA_ARGS__))
#endif
#else
#define clog_ul1(...)
#endif

// user define level 2
#if (UL2 >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_ul2(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(UL2,fmt,##__VA_ARGS__))
#else
#define clog_ul2(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(UL2,fmt,##__VA_ARGS__))
#endif
#else
#define clog_ul2(...)
#endif

// user define level 3
#if (UL3 >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_ul3(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(UL3,fmt,##__VA_ARGS__))
#else
#define clog_ul3(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(UL3,fmt,##__VA_ARGS__))
#endif
#else
#define clog_ul3(...)
#endif

// display level
#if (DIS >= BUILD_LOG_LEVEL)
#if ENABLE_DATETIME
#define clog_dis(fmt,...)  MACRO_SPLICE(CLOG_FORMAT_WITH_TIME,_clog(DIS,fmt,##__VA_ARGS__))
#else
#define clog_dis(fmt,...)  MACRO_SPLICE(CLOG_FORMAT,_clog(DIS,fmt,##__VA_ARGS__))
#endif
#else
#define clog_dis(...)
#endif
// clang-format on

void _clog(int log_level, char* fmt, ...);
void clog_hex(int level, uint8_t* data, uint16_t len);
void clog_init(void);
void clog_set_level(int level);
int  clog_get_level(void);
void clog_str_hex(int level, uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
