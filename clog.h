#ifndef __CLOG_H__
#define __CLOG_H__
#include "string.h"
//////////////////////////////depend on the platform 需要根据不同的平台适配////////////////
#include "SEGGER_RTT.h"

#define printf(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
////////////////////////////////////////////////////////////////////
#define CLOG_MAX_FILE_NAME_LEN 64
#define CLOG_BUF_SIZE 768

extern char clog_file_name[CLOG_MAX_FILE_NAME_LEN];
extern int clog_line_num;

// clang-format off
#define clog { \
    memset(clog_file_name, 0, sizeof(clog_file_name));  \
    char *p = strrchr(__FILE__, '\\');  \
    if (p != NULL) { \
      strncpy(clog_file_name, strrchr(__FILE__, '\\') + 1, CLOG_MAX_FILE_NAME_LEN-1); \
    } else { \
			strncpy(clog_file_name, strrchr(__FILE__, '/') + 1, CLOG_MAX_FILE_NAME_LEN-1); \
    } \
		clog_line_num = __LINE__; \
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

#endif
