#ifndef __CLOG_H__
#define __CLOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdio.h"
#include "string.h"
//////////////////////////////depend on the platform 需要根据不同的平台适配////////////////
#define log clog

#define ENABLE_RTT
#ifdef ENABLE_RTT
#include "SEGGER_RTT.h"
#define printf(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#endif

// it is variaty that every platform get the rtc time。
// 每个平台获取时间的实现都有所不同。
// datetime :为长度为6的字节数组，返回年月日时分秒
extern void clog_get_datetime(unsigned char *datetime);
////////////////////////////////////////////////////////////////////

#define CLOG_BUF_SIZE 256
extern char clog_buf[CLOG_BUF_SIZE];

// clang-format off
#define clog { \
    unsigned char datetime[6] = {0}; \
    clog_get_datetime(datetime); \
    memset(clog_buf, 0, sizeof(clog_buf)); \
    char *p = strrchr(__FILE__, '\\');  \
    if (p != NULL) { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", datetime[0], datetime[1], datetime[2], datetime[3], datetime[4], datetime[5], strrchr(__FILE__, '\\') + 1, __LINE__); \
    } else { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", datetime[0], datetime[1], datetime[2], datetime[3], datetime[4], datetime[5], strrchr(__FILE__, '/') + 1, __LINE__); \
    } \
  } _clog
// clamg-format on

typedef enum {
  INF,
	DBG,
  WAR,
  ERR,
  RUN,
  NONE
} CLEVEL_EN;

void _clog(int log_level, char *fmt, ...);
void clog_init(void);
void clog_set_level(int level);

#ifdef __cplusplus
}
#endif

#endif 
