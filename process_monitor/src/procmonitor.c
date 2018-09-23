/*************************************************************************
	> File Name: common.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 23/09/2018
 ************************************************************************/

#include "procmonitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    umask(0);

    if(0 != access(BASE_PATH_VAR, F_OK))
    {
        if(0 != mkdir(BASE_PATH_VAR, 0764))
        {
            fprintf(stderr, "mkdir(%s): %s\n", BASE_PATH_VAR, strerror(errno));
            exit(0);
        }
    }

    if(0 != access(BASE_PATH_PIPE, F_OK))
    {
        if(0 != mkdir(BASE_PATH_PIPE, 0764))
        {
            fprintf(stderr, "mkdir(%s): %s\n", BASE_PATH_PIPE, strerror(errno));
            exit(0);
        }
    }
    
    if(0 == access(PIPE_PROC_MONITOR, F_OK))
    {
        if(0 != unlink(PIPE_PROC_MONITOR))
        {
            fprintf(stderr, "unlink(%s): %s\n", PIPE_PROC_MONITOR, strerror(errno));
            exit(0);
        }
    }

    if(0 != mkfifo(PIPE_PROC_MONITOR, 0664))
    {
        fprintf(stderr, "mkfifo(%s): %s\n", PIPE_PROC_MONITOR, strerror(errno));
        exit(0);
    }
    
    return 0;
}
