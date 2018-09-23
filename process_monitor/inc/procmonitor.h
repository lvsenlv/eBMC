/*************************************************************************
	> File Name: common.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 23/09/2018
 ************************************************************************/

#ifndef __PROCMONITOR_H
#define __PROCMONITOR_H

#include "common.h"
#include <sys/types.h>

typedef enum {
    PROC_TM_ACT_STOP = 0,
    PROC_TM_ACT_RESTART,
}PROC_TM_ACT;

typedef struct ProcInfoStruct {
    pid_t pid;
    int MonitorTime;
    PROC_TM_ACT TimeoutAction;
    char status;
    char ProcName[64];
    char StartCmd[CMD_LINE_MAX_LEN];
    char StopCmd[CMD_LINE_MAX_LEN];
}ProcInfo_t;

#endif
