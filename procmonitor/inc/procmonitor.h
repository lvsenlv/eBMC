/*************************************************************************
	> File Name: procmonitor.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 23/09/2018
 ************************************************************************/

#ifndef __PROCMONITOR_H
#define __PROCMONITOR_H

#include "common.h"
#include <sys/types.h>

#define PROC_PIPE_NAME                          BASE_PATH_PIPE"/procmonitor"
#define PROC_MONITOR_MAX_CNT                    32
#define PROC_CMD_LINE_MAX_LEN                   128
#define PROC_NAME_MAX_LEN                       64
#define PROC_TIMEOUT_MAX_VAL                    300 //Unit: second

typedef enum {
    PROC_CMD_REGISTER = 1,
    PROC_CMD_KEEP_ALIVE,
    PROC_CMD_UPDATE_PID,
    PROC_CMD_EXIT,
}PROC_CMD;

typedef struct proc_cmd_struct {
    int cmd;
    int pid;
    int timeout;
    char proc_name[PROC_NAME_MAX_LEN];
    char start_cmd[PROC_CMD_LINE_MAX_LEN];
}PACKED proc_cmd_t;

int proc_register(proc_cmd_t *proc_cmd_pt, void *p_arg);
int proc_keep_alive(proc_cmd_t *proc_cmd_pt, void *p_arg);
int proc_update_pid(proc_cmd_t *proc_cmd_pt, void *p_arg);
int proc_cancle(proc_cmd_t *proc_cmd_pt, void *p_arg);

#endif
