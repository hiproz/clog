#include "stdio.h"
#include <stdarg.h>
#include "clog.h"

int clog_level = INF;
char clog_buf[CLOG_BUF_SIZE] = {0};
extern void clog_printf(char *);
void _clog(int log_level, char *fmt, ...) {
  if (log_level == NONE || fmt == 0 || strlen(fmt) == 0)
    return;

  if (log_level >= clog_level) {
    // log level filter
    if (log_level == INF) {
      strcat(clog_buf, "[INF] ");
    } else if (log_level == WAR) {
      strcat(clog_buf, "[WAR] ");
    } else if (log_level == RUN) {
      strcat(clog_buf, "[RUN] ");
    } else if (log_level == ERR) {
      strcat(clog_buf, "[ERR] ");
    } else {
      //do nothing
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(clog_buf + strlen(clog_buf), sizeof(clog_buf) - 1 - strlen(clog_buf), fmt, args);
    va_end(args);

    clog_printf(clog_buf);
  }
}

// init the clog, put some initialize or resource option in here
void clog_init(void) { clog_level = INF; }

// set the log level filter
void clog_set_level(int level) { clog_level = level; }

