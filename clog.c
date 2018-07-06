
// clang-format off
#include "ids_log.h"
// clang-format on

char log_buf[UART_SEND_BUF_SIZE] = { 0 };
void ids_log_init(void) { ; }
int ids_log_level = ERR;

/*****************************************************************************

 *****************************************************************************/
void ids_log(int log_level, char* fmt, ...) {
  va_list args;
  char buf[UART_SEND_BUF_SIZE] = { 0 };

  if (fmt == NULL || strlen(fmt) == 0) return;

  if (log_level >= ids_log_level) {
    // 日志级别
    if (log_level == INF) { strcat(log_buf, "[INF] "); }
    if (log_level == RUN) { strcat(log_buf, "[RUN] "); }
    if (log_level == WAR) { strcat(log_buf, "[WAR] "); }
    if (log_level == ERR) { strcat(log_buf, "[ERR] "); }

    // 打印的内容
    va_start(args, fmt);
    vsnprintf(buf, UART_SEND_BUF_SIZE, fmt, args);
    va_end(args);
    strncat(log_buf, buf, UART_SEND_BUF_SIZE);

    // 如果原始内容，没有加回车换行，这里统一增加回车换行
    // strncat(log_buf, "\r\n", 2);
    // 打印输出
    am_hal_uart_string_transmit_buffered(DBG_UART, log_buf);
    uart_transmit_delay(DBG_UART);
  }
}
void ids_printf(int log_level, char* fmt, ...) {
  // if (ids_log_level == NONE) return;

  va_list args;
  char buf[UART_SEND_BUF_SIZE] = { 0 };

  if (fmt == NULL || strlen(fmt) == 0) return;
  if (log_level >= ids_log_level) {
    // 打印的内容
    va_start(args, fmt);
    vsnprintf(buf, UART_SEND_BUF_SIZE, fmt, args);
    va_end(args);

    // 追加日志前缀
    strncat(log_buf, buf, UART_SEND_BUF_SIZE);
    // strncat(log_buf, "\r\n", 2);

    // 打印输出

    am_hal_uart_string_transmit_buffered(DBG_UART, log_buf);
    uart_transmit_delay(DBG_UART);
  }
}
void ids_log_set_level(int level) { ids_log_level = level; }
