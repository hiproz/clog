#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "clog.h"

int clog_level = INF;
char clog_buf[CLOG_BUF_SIZE] = {0};

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
int clog_get_level(void) { return clog_level; }
void clog_hex(uint8_t *data, uint16_t len) {
  int i;
  for (i = 0; i < len; i++) {
    clog_printf("%02X ", data[i]);
    if (i % 16 == 15) {
      clog_printf("\r\n");
    }
  }
  // 非整行
  if (i % 16 != 0) {
    clog_printf("\r\n");
  }
}
void clog_str_hex(uint8_t *data, uint16_t len) {
  int i;
  for (i = 0; i < len; i++) {
    if (data[i] >= 0x20 && data[i] <= 0x7e) {
      clog_printf("%c", data[i]);
    } else {
      clog_printf("%02X ", data[i]);
    }
    if (i % 16 == 15) {
      clog_printf("\r\n");
    }
  }
  // 非整行
  if (i % 16 != 0) {
    clog_printf("\r\n");
  }
}
