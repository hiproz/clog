
// clang-format off
#include <stdarg.h>
#include "clog.h"
// clang-format on

char clog_buf[CLOG_BUF_SIZE] = {0};
int clog_level = INF;

/*****************************************************************************

 *****************************************************************************/
void _clog(int log_level, char *fmt, ...) {
  va_list args;
  char buf[CLOG_BUF_SIZE] = {0};

  if (fmt == NULL || strlen(fmt) == 0)
    return;

  if (log_level >= clog_level) {
    // 日志级别
    if (log_level == INF) {
      strcat(clog_buf, "[INF] ");
    }
    if (log_level == RUN) {
      strcat(clog_buf, "[RUN] ");
    }
    if (log_level == WAR) {
      strcat(clog_buf, "[WAR] ");
    }
    if (log_level == ERR) {
      strcat(clog_buf, "[ERR] ");
    }

    // 打印的内容
    va_start(args, fmt);
    vsnprintf(buf, CLOG_BUF_SIZE, fmt, args);
    va_end(args);
    strncat(clog_buf, buf, CLOG_BUF_SIZE);

    // 如果原始内容，没有加回车换行，这里统一增加回车换行
    // strncat(clog_buf, "\r\n", 2);
    // 打印输出
    printf("%s", clog_buf);
  }
}
void clog_init(void) { ; }
void clog_set_level(int level) { clog_level = level; }
