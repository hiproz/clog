#include "stdio.h"
#include <stdarg.h>
#include "clog.h"

////////////////////////depend on the platform////////////////////////////
// need the user to impletation this function,it is variaty that every platform get the rtc time。
// 需要业务自己实现这个函数,每个平台获取时间的实现都有所不同。
// datetime :为长度为6的字节数组，返回年月日时分秒
extern void clog_get_datetime(unsigned char *datetime);
//////////////////////////////////////////////////////////////////////////

int clog_level = INF;
char clog_file_name[CLOG_MAX_FILE_NAME_LEN];
int clog_line_num = 0;
char clog_buf[CLOG_BUF_SIZE] = {0};

void _clog(int log_level, char *fmt, ...) {

  if (fmt == 0 || strlen(fmt) == 0)
    return;

  if (log_level >= clog_level) {
    memset(clog_buf, 0, sizeof(clog_buf));

    // date time and code line number.
    unsigned char datetime[6] = {0};
    clog_get_datetime(datetime);

    snprintf(clog_buf + strlen(clog_buf), sizeof(clog_buf) - 1 - strlen(clog_buf), "%02u/%02u/%02u %02u:%02u:%02u %s:%d ",
             datetime[0], datetime[1], datetime[2], datetime[3], datetime[4], datetime[5],
             clog_file_name, clog_line_num);

    // log level filter
    if (log_level == INF) {
      strcat(clog_buf, "[INF] ");
    } else if (log_level == DBG) {
      strcat(clog_buf, "[DBG] ");
    } else if (log_level == WAR) {
      strcat(clog_buf, "[WAR] ");
    } else if (log_level == RUN) {
      strcat(clog_buf, "[RUN] ");
    } else if (log_level == ERR) {
      strcat(clog_buf, "[ERR] ");
    } else {
      //do nothing
      return;
    }

    va_list args;

    va_start(args, fmt);
    vsnprintf(clog_buf + strlen(clog_buf), sizeof(clog_buf) - 1 - strlen(clog_buf), fmt, args);
    va_end(args);

    printf(clog_buf);
  }
}

// init the clog, put some initialize or resource option in here
void clog_init(void) {
  clog_level = INF;
}

// set the log level filter
void clog_set_level(int level) {
  clog_level = level;
}
