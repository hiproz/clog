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

////////////depend on the platform 需要根据不同的平台适配/////////////
#define ENABLE_DATETIME
#ifdef ENABLE_DATETIME
// it is variaty that every platform get the rtc time。
// 每个平台获取时间的实现都有所不同。
// datetime :为长度为6的字节数组，返回年月日时分秒
extern void clog_get_datetime(unsigned char *datetime);
#endif
////////////////////////////////////////////////////////////////////

//todo 要根据目标uart的发送bufer长度实际调整
#define CLOG_BUF_SIZE 512
extern char clog_buf[CLOG_BUF_SIZE];

// clang-format off
#ifdef ENABLE_DATETIME
#define clog { \
    unsigned char datetime[6] = {0}; \
    clog_get_datetime(datetime); \
    memset(clog_buf, 0, sizeof(clog_buf)); \
    char *p = strrchr(__FILE__, '\\');  \
    if (p != NULL) { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", \
      datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0], strrchr(__FILE__, '\\') + 1, __LINE__); \
    } else { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", \
      datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0], strrchr(__FILE__, '/') + 1, __LINE__); \
    } \
  } _clog
#else
#define clog { \
    memset(clog_buf, 0, sizeof(clog_buf)); \
    char *p = strrchr(__FILE__, '\\');  \
    if (p != NULL) { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%s:%d ", \
      strrchr(__FILE__, '\\') + 1, __LINE__); \
    } else { \
      snprintf(clog_buf, CLOG_BUF_SIZE - 1, "%s:%d ", \
      strrchr(__FILE__, '/') + 1, __LINE__); \
    } \
  } _clog
#endif
// clamg-format on

typedef enum {
  INF,
  WAR,
  ERR,
  RUN,
  NONE
} LOG_LEVEL_EN;

void _clog(int log_level, char *fmt, ...);
void clog_hex(uint8_t * data, uint16_t len);
void clog_init(void);
void clog_set_level(int level);
int clog_get_level(void);
void clog_str_hex(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif 

