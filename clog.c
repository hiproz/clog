#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "clog.h"

int  clog_level              = LOG_LEVEL;
char clog_buf[CLOG_BUF_SIZE] = {0};
//////////////////////////////////////////////

void _clog(int log_level, char* fmt, ...) {
  if (fmt == 0 || strlen(fmt) == 0) {
    return;
  }

  if (log_level >= clog_level) {
    if (log_level == LL_INF) {
      strcat(clog_buf, "[INF] ");
    } else if (log_level == LL_WAR) {
      strcat(clog_buf, "\033[43m[WAR]\033[0m ");
    } else if (log_level == LL_ERR) {
      strcat(clog_buf, "\033[41m[ERR]\033[0m ");
    } else if (log_level == LL_RUN) {
      strcat(clog_buf, "[RUN] ");
    } else {
      //do nothing
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(clog_buf + strlen(clog_buf), sizeof(clog_buf) - 3 - strlen(clog_buf), fmt, args);
    strcat(clog_buf, "\r\n");
    va_end(args);

    clog_printf(clog_buf);
  }
}

// init the clog, put some initialize or resource option in here
void clog_init(void) { clog_level = LL_INF; }

// set the log level filter
void clog_set_level(int level) { clog_level = level; }

int clog_get_level(void) { return clog_level; }

void clog_hex(int level, uint8_t* data, uint16_t len) {
  if (level >= clog_level) {
    int i;
    for (i = 0; i < len; i++) {
      clog_printf("%02X ", data[i]);
      if (i % 16 == 15) {
        clog_printf("\r\n");
      }
    }
    if (i % 16 != 0) {
      clog_printf("\r\n");
    }
  }
}

void clog_str_hex(int level, uint8_t* data, uint16_t len) {
  if (level >= clog_level) {
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
    if (i % 16 != 0) {
      clog_printf("\r\n");
    }
  }
}
