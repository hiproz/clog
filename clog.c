#include <stdlib.h>
#include <stdarg.h>
#include "clog.h"
#include "sys/stat.h"
#include <time.h>
#ifdef _WIN32
#include "direct.h"
#else
#include "sys/types.h"
#endif

int   clog_level = LOG_LEVEL;
char* clog_buf = NULL;
int   clog_file_enable = 0;  // 默认只打印在console，如果开了开关，打印的内容会同步存储日志文件
char  clog_file_path[128] = {0};
int   clog_file_num = 10;
int   clog_file_size = 10;
FILE* current_file = NULL;
int   current_file_size = 0;
int   current_file_num = 0;
int   current_file_index = 0;

typedef struct {
    struct stat status;
    char        file_name[64];  // onley file name
} file_stru;

file_stru* file_list = NULL;
//////////////////////////////////////////////
// init the clog, put some initialize or resource option in here
int clog_init(void)
{
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
void clog_set_level(int level)
{
    clog_level = level;
}

int clog_get_level(void)
{
    return clog_level;
}

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
    //printf("start scan the dir:%s\n", directoryPath);
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        //printf("file name :%s\n", entry->d_name);

        sprintf(filePath, "%s\\%s", directoryPath, entry->d_name);
        //printf("file path :%s\n", filePath);

        if (file_count >= clog_file_num) {
            if (remove(filePath) == 0) {
                //printf(" remove the exceed file success\n");
            } else {
                printf("remove failed\n");
            }
            continue;
        } else {
            struct stat file_stat;
            time_t      modified_time;

            if (stat(filePath, &file_stat) == -1) {
                printf("get file info failed\n");
                break;
            }

            modified_time = file_stat.st_mtime;
            //printf("file time:%ld\n", modified_time);
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

void clog_set_file_para(int enable, char* save_dir_path, int file_num, int file_size)
{
    clog_file_enable = enable;
    if (clog_file_enable != 0) {
        //printf("dir:%s\n", save_dir_path);
        strcpy(clog_file_path, save_dir_path);
#ifdef _WIN32
        strcat(clog_file_path, "\\logs");
#else
        strcat(clog_file_path, "/logs");
#endif
        int result = _mkdir(clog_file_path);
        if (result == 0) {
            //printf("create dir:%s success!\n", clog_file_path);
        } else {
            //printf("dir exist:%!\n", clog_file_path);
        }

        clog_file_num = file_num;
        clog_file_size = file_size;

        file_list = (file_stru*)malloc(sizeof(file_stru) * file_num);
        if (!file_list) {
            printf("create file list failed\n");
        }

        printFilesByModifiedTime(clog_file_path);
    }
}

void write_log_file(const char* log)
{
    if (current_file) {
        //printf("current file size:%d %d\n", current_file_size, clog_file_size);
        if (current_file_size < clog_file_size) {
            fprintf(current_file, "%s", log);
            current_file_size += strlen(log);
            fflush(current_file);
            return;
        } else {
            //printf("close new file\n");
            fclose(current_file);
            current_file_size = 0;
            current_file = NULL;
        }
    }

    if (!current_file) {
        if (current_file_num >= clog_file_num) {
            char tmp_path[128] = {0};
            snprintf(tmp_path, sizeof(tmp_path), "%s\\%s", clog_file_path, file_list[current_file_index].file_name);
            if (remove(tmp_path) == 0) {
                //printf("remove the file success:%s\n", tmp_path);
            } else {
                printf("remove failed:%s\n", tmp_path);
            }
            current_file_num--;
        }
        time_t    t = time(NULL);
        struct tm tm = *localtime(&t);
        char      file_path[128];
        char      filename[64] = {0};
        sprintf(filename, "%d_%02d_%02d_%02d_%02d_%02d-%03d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, rand() % 1000);
#ifdef _WIN32
        sprintf(file_path, "%s\\%s", clog_file_path, filename);
#else
        sprintf(file_path, "%s/%ds", clog_file_path, filename);
#endif
        current_file = fopen(file_path, "w");
        if (current_file) {
            //printf("create new file sucess:%s\n", file_path);
            current_file_size = 0;
            fprintf(current_file, "%s", log);
            current_file_size += strlen(log);
            fflush(current_file);

            strcpy(file_list[current_file_index].file_name, filename);
            current_file_index = (current_file_index + 1) % clog_file_num;
            current_file_num++;
        }
    }
}

void clog_process(char* log)
{
    if (clog_file_enable) {
        write_log_file(log);
    }
    clog_printf("%s", log);
}

void _clog(int log_level, char* fmt, ...)
{
    if (fmt == 0 || strlen(fmt) == 0) {
        return;
    }
    if (log_level >= clog_level) {
        if (log_level == LL_DBG) {
            strcat(clog_buf, "[DBG] ");
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
        vsnprintf(clog_buf + strlen(clog_buf), CLOG_BUF_SIZE - 1 - strlen(clog_buf), fmt, args);
        strcat(clog_buf, "\n");
        va_end(args);

        clog_process(clog_buf);
    }
}

void _clog_hex(int level, uint8_t* data, uint16_t len)
{
    if (level >= clog_level) {
        if (level == LL_DBG) {
            strcat(clog_buf, "[DBG]:\n");
        } else if (level == LL_WAR) {
            strcat(clog_buf, "\033[43m[WAR]\033[0m:\n");
        } else if (level == LL_ERR) {
            strcat(clog_buf, "\033[41m[ERR]\033[0m:\n");
        } else if (level == LL_RUN) {
            strcat(clog_buf, "[RUN]:\n");
        } else {
            //do nothing
        }
    }
    clog_process(clog_buf);

    memset(clog_buf, 0, CLOG_BUF_SIZE);

    if (level >= clog_level) {
        int i;
        int hex_bytes = CLOG_BUF_SIZE * 1000 / 3125;

        for (i = 0; i < len && i < hex_bytes; i++) {
            sprintf(clog_buf + strlen(clog_buf), "%02X ", data[i]);
            if (i % 16 == 15) {
                sprintf(clog_buf + strlen(clog_buf), "%s", "\n");
            }
        }

        clog_process(clog_buf);
    }
}

void _clog_mix(int level, uint8_t* data, uint16_t len)
{
    if (level >= clog_level) {
        if (level == LL_DBG) {
            strcat(clog_buf, "[DBG]:\n");
        } else if (level == LL_WAR) {
            strcat(clog_buf, "\033[43m[WAR]\033[0m:\n");
        } else if (level == LL_ERR) {
            strcat(clog_buf, "\033[41m[ERR]\033[0m:\n");
        } else if (level == LL_RUN) {
            strcat(clog_buf, "[RUN]:\n");
        } else {
            //do nothing
        }
    }
    clog_process(clog_buf);

    memset(clog_buf, 0, CLOG_BUF_SIZE);

    if (level >= clog_level) {
        int i;
        int hex_bytes = CLOG_BUF_SIZE * 1000 / 3125;

        for (i = 0; i < len && i < hex_bytes; i++) {
            if (data[i] >= 0x20 && data[i] <= 0x7e) {
                sprintf(clog_buf + strlen(clog_buf), "%c ", data[i]);
            } else {
                sprintf(clog_buf + strlen(clog_buf), "%02X ", data[i]);
            }
            if (i % 16 == 15) {
                sprintf(clog_buf + strlen(clog_buf), "%s", "\n");
            }
        }
        clog_process(clog_buf);
    }
}
