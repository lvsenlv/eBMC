/*************************************************************************
	> File Name: log.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 28th,2018 Saturday 16:20:09
 ************************************************************************/

#include "log.h"
#include "errno_rw.h"
#include "parser.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

#define LOG_CONF_MAX_ENTRY                      256
#define LOG_KEY_NAME                            "name"
#define LOG_KEY_SIZE                            "size"
#define LOG_KEY_ROTATE                          "rotate"
#define LOG_KEY_MUTEX                           "mutex"

static int log_rotate(log_t *log_pt)
{
    char buf[FILE_NAME_MAX_LENGTH];

    if(0 == log_pt->rotate)
        return -1;

    if(log_pt->prev_rotate > log_pt->rotate)
    {
        log_pt->prev_rotate = 1;
    }
    
    snprintf(buf, sizeof(buf), "%s%d", log_pt->name, log_pt->prev_rotate);
    if(0 != rename(log_pt->name, buf))
    {
        if(ENOENT == errno)
        {
            fclose(log_pt->fp);
            log_pt->fp = fopen(log_pt->name, "w+");
        }
        
        return -1;
    }

    log_pt->prev_rotate++;

    return 0;
}

int log_load(logcfg_t *logcfg_pt, const char *conf_p)
{
    parser_t *parser_pt;

    if((NULL == logcfg_pt) || (NULL == conf_p))
    {
        errno = EINVAL;
        return -1;
    }
    
    if(0 != access(conf_p, F_OK))
        return -1;
    
    parser_pt = par_load(conf_p);
    if(NULL == parser_pt)
        return -1;
        
    logcfg_pt->cfg_p = (void *)parser_pt;
    
    return 0;
}

void log_unload(logcfg_t *logcfg_pt)
{
    if(NULL == logcfg_pt)
        return;

    par_unload((parser_t *)logcfg_pt->cfg_p);
}

int log_get(logcfg_t *logcfg_pt, log_t *log_pt, int level)
{
    char sec[PAR_ONE_LINE_MAX_LEN];
    char *tmp_p;

    if((NULL == logcfg_pt) || (NULL == log_pt))
        return -1;

    memset(log_pt, 0, sizeof(log_t));
    
    snprintf(sec, sizeof(sec), "log%d", level);
    tmp_p = par_getstring(logcfg_pt->cfg_p, sec, LOG_KEY_NAME, NULL);
    if(NULL == tmp_p)
    {
        errno = ENODATA;
        return -1;
    }

    strncpy(log_pt->name, tmp_p, FILE_NAME_MAX_LENGTH);
    log_pt->name[FILE_NAME_MAX_LENGTH-1] = 0;
    log_pt->size        = par_getint(logcfg_pt->cfg_p, sec, LOG_KEY_SIZE, LOG_DEFAULT_SIZE);
    log_pt->rotate      = par_getint(logcfg_pt->cfg_p, sec, LOG_KEY_ROTATE, LOG_DEFAULT_ROTATE);
    log_pt->mutex       = par_getboolean(logcfg_pt->cfg_p, sec, LOG_KEY_MUTEX, LOG_DEFAULT_MUTEX);
    log_pt->prev_rotate = 1;
    log_pt->fp          = fopen(log_pt->name, "a+");
    if(NULL == log_pt->fp)
        return -1;

    if(log_pt->mutex)
    {
        if(0 != pthread_mutex_init(&log_pt->lock, NULL))
            return -1;
    }

    return 0;
}

void log_print(log_t *log_pt, const char *format, ...)
{
    va_list vl;
    char buf[2048];
    char *ptr;
    struct tm *tm_pt;
    time_t ti;
    char mutex;
    int size;
    
    if((NULL == log_pt) || (NULL == log_pt->fp) || (NULL == format))
        return;

    ptr = buf;
    ti = time(NULL);
    tm_pt = localtime(&ti);
    ptr += snprintf(buf, sizeof(buf), "[%04d-%02d-%02d %02d:%02d:%02d] ", 
        tm_pt->tm_year+1900, tm_pt->tm_mon+1, tm_pt->tm_mday, tm_pt->tm_hour, tm_pt->tm_min, tm_pt->tm_sec);
    
    va_start(vl, format);
    ptr += vsnprintf(ptr, buf+sizeof(buf)-ptr, format, vl);
    va_end(vl);

    mutex = log_pt->mutex;
    if(mutex)
    {
        pthread_mutex_lock(&log_pt->lock);
    }
    
    if((stdout != log_pt->fp) && (stderr != log_pt->fp))
    {
        size = ftell(log_pt->fp);
        if(size > log_pt->size)
        {
            if(0 == log_rotate(log_pt))
            {
                fclose(log_pt->fp);
                log_pt->fp = fopen(log_pt->name, "a+");
                if(NULL == log_pt->fp)
                    goto EXIT;
            }
            else
            {
                if(0 != ftruncate(fileno(log_pt->fp), 0))
                    goto EXIT;
            }
        }
    }
    
    fwrite(buf, 1, ptr-buf, log_pt->fp);
    fflush(log_pt->fp);

EXIT:
    if(mutex)
    {
        pthread_mutex_unlock(&log_pt->lock);
    }
}

int log_main(int argc, char **argv)
{
    logcfg_t logcfg;
    log_t log;
    int i;
    
    if(0 != log_load(&logcfg, "./mtlog.conf"))
    {
        STDOUT("log_load(): %s\n", strerror_rw(errno));
        return -1;
    }

    if(0 != log_get(&logcfg, &log, 1))
    {
        STDOUT("log_get(): %s\n", strerror_rw(errno));
        return -1;
    }

    for(i = 0; i < 1024; i++)
    {
        log_print(&log, "[%s][%d]: i=%d\n", __func__, __LINE__, i);
    }
    
    log_unload(&logcfg);
    
    return 0;
}
