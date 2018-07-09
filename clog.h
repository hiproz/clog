#ifndef __CLOG_H__
#define __CLOG_H__

// clang-format off

// clang-format on

#ifndef ERROR
#define ERROR 1
#endif
#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef U8
typedef unsigned char U8;
#endif
#ifndef S8
typedef signed char S8;
#endif
#ifndef U16
typedef unsigned short U16;
#endif
#ifndef S16
typedef signed short S16;
#endif
#ifndef U32
typedef unsigned int U32;
#endif
#ifndef S32
typedef signed int S32;
#endif

#define FREE_RTOS 1

#if FREE_RTOS
#define s ((xTaskGetTickCount() / configTICK_RATE_HZ))
#define CLOG_YEAR 1
#define CLOG_MONTH 1
#define CLOG_DAY 1
#define CLOG_WEEK 1
#define CLOG_HOUR ((s % (3600 * 24)) / 3600)
#define CLOG_MINUTE ((s % (3600)) / 60)
#define CLOG_SECOND (s % 60)
#endif

// adapting the snprintf method
#if APOLLO_SDK
#define snprintf _snprintf
#endif
#define CLOG_BUF_SIZE 768

#define clog                                                                                                           \
  {                                                                                                                    \
    memset(clog_buf, 0, sizeof(clog_buf));                                                                             \
    char *p = strrchr(__FILE__, '\\');                                                                                 \
    if (p != NULL) {                                                                                                   \
      snprintf(clog_buf, sizeof(clog_buf), "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", CLOG_YEAR, CLOG_MONTH, CLOG_DAY,    \
               CLOG_HOUR, CLOG_MINUTE, CLOG_SECOND, strrchr(__FILE__, '\\') + 1, __LINE__);                            \
    } else {                                                                                                           \
      snprintf(clog_buf, sizeof(clog_buf), "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", CLOG_YEAR, CLOG_MONTH, CLOG_DAY,    \
               CLOG_HOUR, CLOG_MINUTE, CLOG_SECOND, strrchr(__FILE__, '/') + 1, __LINE__);                             \
    }                                                                                                                  \
  }                                                                                                                    \
  _clog
/*******************************************************

 *********************************************************/
typedef enum {

  INF,
  WAR,
  ERR,
  RUN,
  NONE
} CLEVEL_EN;

extern char clog_buf[CLOG_BUF_SIZE];

void _clog(int log_level, char *fmt, ...);
void clog_init(void);
void clog_set_level(int level);

#endif
