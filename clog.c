// https://github.com/hiproz/clog.git
#include <stdlib.h>
#include <stdarg.h>
#include "clog.h"

int   clog_level = LOG_LEVEL;
char* clog_buf   = NULL;
//////////////////////////////////////////////
// init the clog, put some initialize or resource option in here
int clog_init(void) {
  if (clog_buf != NULL) {
    return 0;
  }

  clog_buf = (char*)malloc(CLOG_BUF_SIZE);
  if (NULL == clog_buf) {
    return -1;
  } else {
    return 0;
  }
}

// set the log level filter
void clog_set_level(int level) {
  ;
  clog_level = level;
}

int clog_get_level(void) {
  ;
  return clog_level;
}

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
    vsnprintf(clog_buf + strlen(clog_buf), CLOG_BUF_SIZE - 3 - strlen(clog_buf), fmt, args);
    strcat(clog_buf, "\r\n");
    va_end(args);

    clog_printf(clog_buf);
  }
}

void _clog_hex(int level, uint8_t* data, uint16_t len) {

  if (level >= clog_level) {
    if (level == LL_INF) {
      strcat(clog_buf, "[INF]:\r\n");
    } else if (level == LL_WAR) {
      strcat(clog_buf, "\033[43m[WAR]\033[0m:\r\n");
    } else if (level == LL_ERR) {
      strcat(clog_buf, "\033[41m[ERR]\033[0m:\r\n");
    } else if (level == LL_RUN) {
      strcat(clog_buf, "[RUN]:\r\n");
    } else {
      //do nothing
    }
  }
  clog_printf(clog_buf);

  memset(clog_buf, 0, CLOG_BUF_SIZE);

  if (level >= clog_level) {
    int i;
    int hex_bytes = CLOG_BUF_SIZE * 1000 / 3125;  // 每16个增加2个回车换行

    for (i = 0; i < len && i < hex_bytes; i++) {
      sprintf(clog_buf + strlen(clog_buf), "%02X ", data[i]);
      if (i % 16 == 15) {
        sprintf(clog_buf + strlen(clog_buf), "%s", "\r\n");
      }
    }

    clog_printf(clog_buf);
  }
}

void _clog_mix(int level, uint8_t* data, uint16_t len) {
  if (level >= clog_level) {
    if (level == LL_INF) {
      strcat(clog_buf, "[INF]:\r\n");
    } else if (level == LL_WAR) {
      strcat(clog_buf, "\033[43m[WAR]\033[0m:\r\n");
    } else if (level == LL_ERR) {
      strcat(clog_buf, "\033[41m[ERR]\033[0m:\r\n");
    } else if (level == LL_RUN) {
      strcat(clog_buf, "[RUN]:\r\n");
    } else {
      //do nothing
    }
  }
  clog_printf(clog_buf);

  memset(clog_buf, 0, CLOG_BUF_SIZE);

  if (level >= clog_level) {
    int i;
    int hex_bytes = CLOG_BUF_SIZE * 1000 / 3125;  // 每16个增加2个回车换行

    for (i = 0; i < len && i < hex_bytes; i++) {
      if (data[i] >= 0x20 && data[i] <= 0x7e) {
        sprintf(clog_buf + strlen(clog_buf), "%c ", data[i]);
      } else {
        sprintf(clog_buf + strlen(clog_buf), "%02X ", data[i]);
      }
      if (i % 16 == 15) {
        sprintf(clog_buf + strlen(clog_buf), "%s", "\r\n");
      }
    }
    clog_printf(clog_buf);
  }
}
