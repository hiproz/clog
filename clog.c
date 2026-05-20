#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "clog.h"
#include <time.h>

#if (SUPPORT_LOC_SAVE)
#ifdef _WIN32
#include "direct.h"
#else
#if defined(__GNUC__) && defined(__linux__)
#include "sys/stat.h"
#include "sys/types.h"
#endif
#endif
#endif

typedef void (*custom_log_func_t)(const char* log_buf);

clog_inf_t default_log = {0};
clog_inf_t target_log = {0};

#if ENABLE_DATETIME
#if (SDK_PLATFORM == PLATFORM_COMMON)
void clog_get_datetime(unsigned char* datetime)
{
    time_t     now;
    struct tm* tm_ptr;
    struct tm  tm_local;

    if (datetime == NULL) {
        return;
    }

    now = time(NULL);
    if (now == (time_t)-1) {
        memset(datetime, 0, 6);
        return;
    }

    tm_ptr = localtime(&now);
    if (tm_ptr == NULL) {
        memset(datetime, 0, 6);
        return;
    }
    tm_local = *tm_ptr;

    datetime[0] = (unsigned char)tm_local.tm_sec;
    datetime[1] = (unsigned char)tm_local.tm_min;
    datetime[2] = (unsigned char)tm_local.tm_hour;
    datetime[3] = (unsigned char)tm_local.tm_mday;
    datetime[4] = (unsigned char)(tm_local.tm_mon + 1);
    datetime[5] = (unsigned char)(((tm_local.tm_year + 1900) % 100));
}
#endif /* PLATFORM_COMMON */
#endif /* ENABLE_DATETIME */

static int clog_fmt_valid(const char* fmt)
{
    return fmt != NULL && fmt[0] != '\0';
}

static size_t clog_buf_used(const clog_inf_t* log)
{
    size_t i;

    if (log == NULL || log->clog_buf == NULL) {
        return 0U;
    }
    for (i = 0U; i < (size_t)CLOG_BUF_SIZE; ++i) {
        if (log->clog_buf[i] == '\0') {
            return i;
        }
    }
    return (size_t)CLOG_BUF_SIZE;
}

static size_t clog_buf_left(const clog_inf_t* log)
{
    size_t used = clog_buf_used(log);
    if (used >= (size_t)CLOG_BUF_SIZE) {
        return 0U;
    }
    return (size_t)CLOG_BUF_SIZE - used - 1U;
}

static void clog_append_literal(clog_inf_t* log, const char* text)
{
    size_t used;
    size_t left;
    size_t copy_len;

    if (log == NULL || log->clog_buf == NULL || text == NULL) {
        return;
    }

    used = clog_buf_used(log);
    left = clog_buf_left(log);
    if (left == 0U) {
        return;
    }

    copy_len = 0U;
    while (copy_len < left && text[copy_len] != '\0') {
        copy_len++;
    }
    memcpy(log->clog_buf + used, text, copy_len);
    log->clog_buf[used + copy_len] = '\0';
}

static void clog_appendf(clog_inf_t* log, const char* fmt, ...)
{
    va_list args;
    size_t  used;
    size_t  left;

    if (log == NULL || log->clog_buf == NULL || fmt == NULL) {
        return;
    }

    used = clog_buf_used(log);
    left = clog_buf_left(log);
    if (left == 0U) {
        return;
    }

    va_start(args, fmt);
    (void)vsnprintf(log->clog_buf + used, left + 1U, fmt, args);
    va_end(args);
}

static void clog_append_level_tag(clog_inf_t* log, int level, const char* suffix)
{
    if (level == LL_DBG) {
        clog_append_literal(log, "[DBG]");
    } else if (level == LL_WAR) {
        clog_append_literal(log, "\033[43m[WAR]\033[0m");
    } else if (level == LL_ERR) {
        clog_append_literal(log, "\033[41m[ERR]\033[0m");
    } else if (level == LL_RUN) {
        clog_append_literal(log, "[RUN]");
    } else {
        return;
    }
    if (suffix != NULL) {
        clog_append_literal(log, suffix);
    }
}

#if (SUPPORT_LOC_SAVE)
typedef struct {
    struct stat status;
    char        file_name[64];
} file_stru;

file_stru* file_list = NULL;
#endif

int clog_init(clog_inf_t* log, int level)
{
    if (log == NULL) {
        return -1;
    }

    log->clog_level = level;
    log->clog_file_num = 10;
    log->clog_file_size = 1;

    if (log->clog_buf != NULL) {
        free(log->clog_buf);
        log->clog_buf = NULL;
    }

    log->clog_buf = (char*)malloc(CLOG_BUF_SIZE);
    if (log->clog_buf == NULL) {
        return -1;
    }
    memset(log->clog_buf, 0, CLOG_BUF_SIZE);

    return 0;
}

void clog_deinit(clog_inf_t* log)
{
    if (log == NULL) {
        return;
    }
    if (log->clog_buf != NULL) {
        free(log->clog_buf);
        log->clog_buf = NULL;
    }
#if (SUPPORT_LOC_SAVE)
    if (log->current_file != NULL) {
        fclose(log->current_file);
        log->current_file = NULL;
    }
#endif
}

int clog_get_level(clog_inf_t* log)
{
    if (log == NULL) {
        return LL_NONE;
    }
    return log->clog_level;
}

#if (SUPPORT_LOC_SAVE)
int compare(const void* a, const void* b)
{
    struct stat* stat_a = &((file_stru*)a)->status;
    struct stat* stat_b = &((file_stru*)b)->status;
    return (int)(stat_a->st_mtime - stat_b->st_mtime);
}

void printFilesByModifiedTime(const char* directoryPath)
{
    DIR*           dir;
    struct dirent* entry;
    char           filePath[MAX_PATH];
    int            file_count = 0;

    dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("can't open dir:%s\n", directoryPath);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        sprintf(filePath, "%s\\%s", directoryPath, entry->d_name);

        if (file_count >= clog_file_num) {
            if (remove(filePath) == 0) {
            } else {
                printf("remove failed\n");
            }
            continue;
        } else {
            struct stat file_stat;

            if (stat(filePath, &file_stat) == -1) {
                printf("get file info failed\n");
                break;
            }

            file_list[file_count].status = file_stat;
            strcpy(file_list[file_count].file_name, entry->d_name);
            file_count++;
        }
    }

    closedir(dir);

    qsort(file_list, file_count, sizeof(file_stru), compare);

    current_file_num = file_count;
    current_file_index %= clog_file_num;
}

/** Configure log directory and rotating file limits (SUPPORT_LOC_SAVE only). */
void clog_set_file_para(int enable, char* save_dir_path, int file_num, int file_size)
{
    if (enable != 0) {
        strcpy(clog_file_path, save_dir_path);
#ifdef _WIN32
        strcat(clog_file_path, "\\logs");
#else
        strcat(clog_file_path, "/logs");
#endif
        int result = _mkdir(clog_file_path);
        (void)result;

        clog_file_num = file_num;
        clog_file_size = file_size;

        file_list = (file_stru*)malloc(sizeof(file_stru) * file_num);
        if (!file_list) {
            printf("create file list failed\n");
        }

        printFilesByModifiedTime(clog_file_path);
    }
}

void write_log_file(clog_inf_t* log, const char* log_data)
{
    if (log->current_file) {
        if (log->current_file_size < log->clog_file_size) {
            fprintf(log->current_file, "%s", log_data);
            log->current_file_size += strlen(log_data);
            fflush(log->current_file);
            return;
        } else {
            fclose(log->current_file);
            log->current_file_size = 0;
            log->current_file = NULL;
        }
    }

    if (!log->current_file) {
        if (log->current_file_num >= log->clog_file_num) {
            char tmp_path[128] = {0};
            snprintf(tmp_path, sizeof(tmp_path), "%s\\%s", log->clog_file_path, log->file_list[log->current_file_index].file_name);
            if (remove(tmp_path) == 0) {
            } else {
                printf("remove failed:%s\n", tmp_path);
            }
            log->current_file_num--;
        }
        time_t    t = time(NULL);
        struct tm tm = *localtime(&t);
        char      file_path[128];
        char      filename[64] = {0};
        sprintf(filename, "%d_%02d_%02d_%02d_%02d_%02d-%03d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, rand() % 1000);
#ifdef _WIN32
        sprintf(file_path, "%s\\%s", log->clog_file_path, filename);
#else
        sprintf(file_path, "%s/%s", log->clog_file_path, filename);
#endif
        log->current_file = fopen(file_path, "w");
        if (log->current_file) {
            log->current_file_size = 0;
            fprintf(log->current_file, "%s", log_data);
            log->current_file_size += strlen(log_data);
            fflush(log->current_file);

            strcpy(log->file_list[log->current_file_index].file_name, filename);
            log->current_file_index = (log->current_file_index + 1) % log->clog_file_num;
            log->current_file_num++;
        }
    }
}
#endif

void clog_process(clog_inf_t* log, char* log_data)
{
#if (SUPPORT_LOC_SAVE)
    write_log_file(log, log_data);
#else
    (void)log;
#endif
    clog_printf("%s", log_data);
}

#if ENABLE_DATETIME
static void clog_append_datetime_prefix(clog_inf_t* log)
{
    unsigned char datetime[6] = {0};

    clog_get_datetime(datetime);
    clog_appendf(log, "[%02u/%02u/%02u %02u:%02u:%02u] ", datetime[5], datetime[4], datetime[3], datetime[2], datetime[1], datetime[0]);
}
#endif

static void clog_append_file_line_prefix(clog_inf_t* log, const char* file, int line)
{
    const char* file_name = file;
    const char* win_sep;
    const char* unix_sep;

    win_sep = strrchr(file, '\\');
    unix_sep = strrchr(file, '/');
    if (win_sep != NULL) {
        file_name = win_sep + 1;
    } else if (unix_sep != NULL) {
        file_name = unix_sep + 1;
    }

    clog_appendf(log, "[%s:%d] ", file_name, line);
}

void clog_format_prefix(clog_inf_t* log, const char* file, int line)
{
    if (log == NULL || log->clog_buf == NULL || file == NULL) {
        return;
    }

    memset(log->clog_buf, 0, CLOG_BUF_SIZE);

#if ENABLE_DATETIME
    clog_append_datetime_prefix(log);
#endif
    clog_append_file_line_prefix(log, file, line);
}

void _clog(clog_inf_t* log, int level, const char* fmt, ...)
{
    va_list args;

    if (log == NULL || log->clog_buf == NULL || !clog_fmt_valid(fmt)) {
        return;
    }
    if (level < log->clog_level) {
        return;
    }

    clog_append_level_tag(log, level, " ");
    va_start(args, fmt);
    (void)vsnprintf(log->clog_buf + clog_buf_used(log), clog_buf_left(log) + 1U, fmt, args);
    va_end(args);
    clog_append_literal(log, "\n");
    clog_process(log, log->clog_buf);
}

void _clog_hex(clog_inf_t* log, int level, const uint8_t* data, uint16_t len)
{
    int i;

    if (log == NULL || log->clog_buf == NULL || data == NULL) {
        return;
    }
    if (level < log->clog_level) {
        return;
    }

    clog_append_level_tag(log, level, ":");
    for (i = 0; i < len && i < (int)CLOG_HEX_MAX_BYTES; i++) {
        clog_appendf(log, "%02X ", data[i]);
    }
    clog_append_literal(log, "\n");
    clog_process(log, log->clog_buf);
}

void _clog_mix(clog_inf_t* log, int level, const uint8_t* data, uint16_t len)
{
    int i;

    if (log == NULL || log->clog_buf == NULL || data == NULL) {
        return;
    }
    if (level < log->clog_level) {
        return;
    }

    clog_append_level_tag(log, level, ":\n");
    for (i = 0; i < len && i < (int)CLOG_HEX_MAX_BYTES; i++) {
        if (data[i] >= 0x20 && data[i] <= 0x7e) {
            clog_appendf(log, "%c ", data[i]);
        } else {
            clog_appendf(log, "%02X ", data[i]);
        }
    }
    clog_append_literal(log, "\n");
    clog_process(log, log->clog_buf);
}

void _clog_custom(clog_inf_t* log, void* func, int level, const char* fmt, ...)
{
    va_list args;

    if (log == NULL || log->clog_buf == NULL || func == NULL || !clog_fmt_valid(fmt)) {
        return;
    }
    if (level < log->clog_level) {
        return;
    }

    va_start(args, fmt);
    (void)vsnprintf(log->clog_buf + clog_buf_used(log), clog_buf_left(log) + 1U, fmt, args);
    va_end(args);
    clog_append_literal(log, "\n");
    ((custom_log_func_t)func)(log->clog_buf);
}

void _clog_hex_custom(clog_inf_t* log, void* func, int level, const uint8_t* data, uint16_t len)
{
    int i;

    if (log == NULL || log->clog_buf == NULL || func == NULL || data == NULL) {
        return;
    }
    if (level < log->clog_level) {
        return;
    }

    clog_append_level_tag(log, level, ":");
    for (i = 0; i < len && i < (int)CLOG_HEX_MAX_BYTES; i++) {
        clog_appendf(log, "%02X ", data[i]);
    }
    clog_append_literal(log, "\n");
    ((custom_log_func_t)func)(log->clog_buf);
}
