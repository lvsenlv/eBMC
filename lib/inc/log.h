/*************************************************************************
	> File Name: log.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 28th,2018 Saturday 16:20:14
 ************************************************************************/

#ifndef __LOG_H
#define __LOG_H

#include "common.h"
#include "file.h"
#include <pthread.h>

#define LOG_DEFAULT_SIZE                        (4*1024)
#define LOG_DEFAULT_ROTATE                      0
#define LOG_DEFAULT_MUTEX                       0
#define LOG(format, args...) \
        do \
        { \
            log_print(&g_log, format, ##args); \
        }while(0)

#define LOG_INFO(format, args...) \
        do \
        { \
            log_print(&i_log, format, ##args); \
        }while(0)

#define LOG_WARN(format, args...) \
        do \
        { \
            log_print(&w_log, format, ##args); \
        }while(0)

#define LOG_ERR(format, args...) \
        do \
        { \
            log_print(&e_log, format, ##args); \
        }while(0)

#define LOG_DEBUG(format, args...) \
        do \
        { \
            log_print(&d_log, format, ##args); \
        }while(0)

typedef struct logcfg_struct {
    void *cfg_p;
}logcfg_t;

typedef struct log_struct {
    char name[FILE_NAME_MAX_LENGTH];
    FILE *fp;
    uint32_t size;
    uint8_t rotate;
    uint8_t prev_rotate;
    uint8_t mutex;
    pthread_mutex_t lock;
}log_t;

int log_load(logcfg_t *logcfg_pt, const char *conf_p);
void log_unload(logcfg_t *logcfg_pt);
int log_get(logcfg_t *log_conf_pt, log_t *log_pt, int level);
void log_print(log_t *log_pt, const char *format, ...)  __attribute__((__format__ (__printf__, 2, 3)));

#endif
