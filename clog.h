#ifndef __IDS_LOG_H__
#define __IDS_LOG_H__

// clang-format off
#include "usr_common.h"
#include "usr_uart.h"
// clang-format on
/*****************************************************************************
IDS: interface development suit
,本接口的模块名，为避免变量和宏冲突，采用模块前缀的命名方式。
 *****************************************************************************/
#define printf                                                                                                         \
  memset(log_buf, 0, sizeof(log_buf));                                                                                 \
  ids_printf

#define log                                                                                                            \
  \
{                                                                                                                   \
    memset(log_buf, 0, sizeof(log_buf));                                                                               \
    char* p = strrchr(__FILE__, '\\');                                                                                 \
    if (p != NULL) {                                                                                                   \
      _snprintf(log_buf, sizeof(log_buf), "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", hal_time.ui32Year,                   \
                hal_time.ui32Month, hal_time.ui32DayOfMonth, hal_time.ui32Hour, hal_time.ui32Minute,                   \
                hal_time.ui32Second, strrchr(__FILE__, '\\') + 1, __LINE__);                                           \
    } else {                                                                                                           \
      _snprintf(log_buf, sizeof(log_buf), "%02u/%02u/%02u %02u:%02u:%02u %s:%d ", hal_time.ui32Year,                   \
                hal_time.ui32Month, hal_time.ui32DayOfMonth, hal_time.ui32Hour, hal_time.ui32Minute,                   \
                hal_time.ui32Second, strrchr(__FILE__, '/') + 1, __LINE__);                                            \
    }                                                                                                                  \
  \
}                                                                                                                   \
  ids_log
/*******************************************************

 *********************************************************/
typedef enum { INF, WAR, ERR, RUN, NONE } IDS_TRACE_LEVEL_EN;

extern char log_buf[UART_SEND_BUF_SIZE];

// void trace( level, module_type sth, kal_char *fmt, ...);
void ids_printf(int log_level, char* fmt, ...);
void ids_log(int log_level, char* fmt, ...);
void ids_log_init(void);
void ids_log_set_level(int level);

#endif
