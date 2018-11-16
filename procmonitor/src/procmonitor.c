/*************************************************************************
	> File Name: procmonitor.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 23/09/2018
 ************************************************************************/

#include "procmonitor.h"
#include "errno_rw.h"
#include "file.h"
#include "queue.h"
#include "hash.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/resource.h>
#include <time.h>

typedef int (*cmd_hndlr_pf) (proc_cmd_t *, void *);
typedef struct cmd_hndlr_map_struct {
    char cmd;
    char enable;
    cmd_hndlr_pf cmd_hndlr;
}cmd_hndlr_map_t;

typedef struct proc_info_struct {
    int pid;
    uint32_t timeout;
    uint32_t time_cnt;
    uint32_t fail_cnt;
    time_t   last_fail_tm;
    uint32_t name_hash;
    uint32_t cmd_hash;
    char proc_name[PROC_NAME_MAX_LEN];
    char start_cmd[PROC_CMD_LINE_MAX_LEN];
    char wait_cancle;
    struct proc_info_struct *next_pt;
}proc_info_t;

log_t g_log;
proc_info_t g_proc_info;

const cmd_hndlr_map_t cmd_tbl[] = {
    {PROC_CMD_REGISTER,         1, proc_register},
    {PROC_CMD_KEEP_ALIVE,       1, proc_keep_alive},
    {PROC_CMD_UPDATE_PID,       1, proc_update_pid},
    {PROC_CMD_EXIT,             1, proc_cancle},
};

static int proc_get_name(int pid, char *buf_p, int size)
{
    char proc_name_path[PROC_NAME_MAX_LEN];
    char buf[PROC_NAME_MAX_LEN];
    int fd;
    int res;

    snprintf(proc_name_path, sizeof(proc_name_path), "/proc/%d/status", pid);
    fd = open(proc_name_path, O_RDONLY);
    if(0 > fd)
        return -1;
        
    res = read(fd, buf, sizeof(buf));
    if(0 >= res)
    {
        close(fd);
        return -1;
    }

    if(res > size)
        return -1;

    sscanf(buf, "%*s %s", buf_p);

    return 0;
}
    
static int proc_get_pid(const char *proc_name_p)
{
    int pid;
    DIR *dir_p;
    struct dirent *entry_p;
    int fd;
    int res;
    char f_name[FILE_NAME_MAX_LENGTH];
    char cur_proc_name[FILE_NAME_MAX_LENGTH];
    char buf[128];

    if((NULL == proc_name_p) || (0 == proc_name_p[0]))
    {
        errno = EINVAL;
        return -1;
    }

    dir_p = opendir("/proc");
    if(NULL == dir_p)
        return -1;

    pid = -1;
    while(1)
    {
        entry_p = readdir(dir_p);
        if(NULL == entry_p)
            break;
        
        //Ignore ".", ".."
        if('.' == entry_p->d_name[0])
        {
            if('\0' == entry_p->d_name[1])
                continue;
            else if(('.' == entry_p->d_name[1]) && ('\0' == entry_p->d_name[2]))
                continue;
        }
        
        if(DT_DIR != entry_p->d_type)
            continue;

        snprintf(f_name, sizeof(f_name), "/proc/%s/status", entry_p->d_name);
        fd = open(f_name, O_RDONLY);
        if(0 > fd)
            continue;
        
        res = read(fd, buf, sizeof(buf));
        if(0 >= res)
        {
            close(fd);
            continue;
        }

        sscanf(buf, "%*s %s", cur_proc_name);

        if(0 != strcmp(proc_name_p, cur_proc_name))
        {
            close(fd);
            continue;
        }

        pid = atoi(entry_p->d_name);
        close(fd);
        break;
    }
    
    closedir(dir_p);
    
    return pid;
}

static int proc_check_cmd(proc_cmd_t *proc_cmd_pt)
{
    if(NULL == proc_cmd_pt)
        return -1;
    
    if(
        (1 >= proc_cmd_pt->pid) || 
        (0 == proc_cmd_pt->proc_name[0]) || 
        (0 != proc_cmd_pt->proc_name[PROC_NAME_MAX_LEN-1]) || 
        (0 == proc_cmd_pt->start_cmd[0]) || 
        (0 != proc_cmd_pt->start_cmd[PROC_CMD_LINE_MAX_LEN-1]) || 
        (0 == proc_cmd_pt->timeout) || 
        (PROC_TIMEOUT_MAX_VAL < proc_cmd_pt->timeout)
      )
        return -1;

    return 0;
}

static int proc_check_pid(int pid, const char *proc_name_p)
{
    char proc_name[PROC_NAME_MAX_LEN];
    
    if(NULL == proc_name_p)
        return -1;

    if(0 != proc_get_name(pid, proc_name, sizeof(proc_name)))
        return -1;

    if(0 != strcmp(proc_name, proc_name_p))
        return -1;

    return 0;
}

static void proc_restart(proc_info_t *proc_info_pt)
{
    int pid;
    struct rlimit rlim;
    int i;
    int fd;
    proc_cmd_t proc_cmd;
    time_t tm;
    
    if(NULL == proc_info_pt)
        return;

    if(proc_info_pt->wait_cancle)
        return;

    time(&tm);
    if(60 > (tm-proc_info_pt->last_fail_tm))
    {
        proc_info_pt->fail_cnt++;
        proc_info_pt->last_fail_tm = tm;
        LOG("pid %d fail %d %ld\n", proc_info_pt->pid, proc_info_pt->fail_cnt, proc_info_pt->last_fail_tm);
    }
    else
    {
        proc_info_pt->fail_cnt = 1;
        proc_info_pt->last_fail_tm = tm;
    }

    if(3 < proc_info_pt->fail_cnt)
    {
        proc_info_pt->time_cnt = 0;
        proc_info_pt->wait_cancle = 1;
        return;
    }

    if(0 == kill(proc_info_pt->pid, 0))
    {
        if(0 != kill(proc_info_pt->pid, SIGKILL))
        {
            proc_info_pt->time_cnt = 0;
            LOG("kill(%d): %s\n", proc_info_pt->pid, strerror_rw(errno));
            return;
        }
    }

    proc_info_pt->time_cnt = 0;
    proc_info_pt->timeout = 5;

    pid = fork();
    if(0 > pid)
    {
        LOG("fork(%s): %s\n", proc_info_pt->proc_name, strerror_rw(errno));
        return;
    }

    if(0 < pid)
        return;

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    setsid();
    chdir("/");

    if(0 != getrlimit(RLIMIT_NOFILE, &rlim))
    {
        LOG("getrlimit(): %s\n", strerror_rw(errno));
        exit(0);
    }

    for(i = 3; i < rlim.rlim_max; i++)
    {
        close(i);
    }

    pid = fork();
    if(0 != pid)
    {
        exit(0);
    }

    fd = open(PROC_PIPE_NAME, O_RDWR);
    if(0 >= fd)
    {
        STDERR("open(%s): %s\n", PROC_PIPE_NAME, strerror_rw(errno));
    }
    else
    {
        memset(&proc_cmd, 0, sizeof(proc_cmd));
        proc_cmd.cmd = PROC_CMD_UPDATE_PID;
        proc_cmd.pid = getpid();
        proc_cmd.timeout = proc_info_pt->timeout;
        memcpy(proc_cmd.proc_name, proc_info_pt->proc_name, PROC_NAME_MAX_LEN-1);
        memcpy(proc_cmd.start_cmd, proc_info_pt->start_cmd, PROC_NAME_MAX_LEN-1);
        queue_set(fd, &proc_cmd, sizeof(proc_cmd));
        close(fd);
    }

    execl("/bin/bash", "bash", "-c", proc_info_pt->start_cmd, NULL);
    exit(0);
}

static void proc_monitor(void)
{
    proc_info_t *proc_info_pt;
    proc_info_t *prev_proc_info_pt;

    prev_proc_info_pt = &g_proc_info;
    proc_info_pt = g_proc_info.next_pt;
    while(NULL != proc_info_pt)
    {
        if(proc_info_pt->wait_cancle)
        {
            proc_info_pt->time_cnt++;
            if(60 < proc_info_pt->time_cnt)
            {
                prev_proc_info_pt->next_pt = proc_info_pt->next_pt;
                LOG("Cancel pid %d\n", proc_info_pt->pid);
                free(proc_info_pt);
                proc_info_pt = prev_proc_info_pt->next_pt;
                continue;
            }
            
            prev_proc_info_pt = proc_info_pt;
            proc_info_pt = proc_info_pt->next_pt;
            continue;
        }
        
        if(proc_info_pt->time_cnt >= proc_info_pt->timeout)
        {
            LOG("Detect pid %d timeout\n", proc_info_pt->pid);
            proc_restart(proc_info_pt);
            prev_proc_info_pt = proc_info_pt;
            proc_info_pt = proc_info_pt->next_pt;
            continue;
        }

        proc_info_pt->time_cnt++;
        prev_proc_info_pt = proc_info_pt;
        proc_info_pt = proc_info_pt->next_pt;
    }
}

#define __PROC_CMD
int proc_register(proc_cmd_t *proc_cmd_pt, void *p_arg)
{
    proc_info_t *proc_info_pt;
    proc_info_t *tmp_proc_info_pt;
    int pid;
    uint32_t name_hash;
    uint32_t cmd_hash;
    char flag;

    if(0 != proc_check_cmd(proc_cmd_pt))
        return -1;

    if(0 != proc_check_pid(proc_cmd_pt->pid, proc_cmd_pt->proc_name))
    {
        LOG("Pid %d does not match \"%s\"\n", proc_cmd_pt->pid, proc_cmd_pt->proc_name);
        return -1;
    }

    pid = proc_cmd_pt->pid;
    proc_info_pt = &g_proc_info;
    name_hash = hash_calculate(proc_cmd_pt->proc_name, 0);
    cmd_hash = hash_calculate(proc_cmd_pt->start_cmd, 0);
    
    while(NULL != proc_info_pt)
    {
        flag = 0;

        if(name_hash == proc_info_pt->name_hash)
        {
            flag++;
        }

        if(cmd_hash == proc_info_pt->cmd_hash)
        {
            flag++;
        }

        if(pid == proc_info_pt->pid)
        {
            flag++;
        }
        
        if(3 == flag)
        {
            if((0 == strcmp(proc_cmd_pt->proc_name, proc_info_pt->proc_name)) && 
                (0 == strcmp(proc_cmd_pt->start_cmd, proc_info_pt->start_cmd)))
            {
                LOG("Pid %d has been register before\n", pid);
                return 0;
            }
        }
        else if(flag)
        {
            if(proc_info_pt->wait_cancle)
            {
                LOG("Proc \"%s\" wait to cancle\n", proc_info_pt->proc_name);
                return -1;
            }
            
            if(pid != proc_info_pt->pid)
            {
                LOG("Update pid %d as %d\n", proc_info_pt->pid, pid);
                proc_info_pt->pid = pid;
            }
            
            if(name_hash != proc_info_pt->name_hash)
            {
                LOG("Update proc name \"%s\" as \"%s\"\n", proc_info_pt->proc_name, proc_cmd_pt->proc_name);
                memcpy(proc_info_pt->proc_name, proc_cmd_pt->proc_name, PROC_NAME_MAX_LEN-1);
                proc_info_pt->name_hash = hash_calculate(proc_info_pt->proc_name, 0);
            }
                
            if(cmd_hash != proc_info_pt->cmd_hash)
            {
                LOG("Update pid %d start cmd \"%s\" as \"%s\"\n", 
                    proc_info_pt->pid, proc_info_pt->start_cmd, proc_cmd_pt->start_cmd);
                memcpy(proc_info_pt->start_cmd, proc_cmd_pt->start_cmd, PROC_CMD_LINE_MAX_LEN-1);
                proc_info_pt->cmd_hash = hash_calculate(proc_info_pt->start_cmd, 0);
            }
            
            return 0;
        }
        
        if(NULL == proc_info_pt->next_pt)
            break;
        
        proc_info_pt = proc_info_pt->next_pt;
    }

    tmp_proc_info_pt = (proc_info_t *)calloc(1, sizeof(proc_info_t));
    if(NULL == tmp_proc_info_pt)
        return -1;

    proc_info_pt->next_pt = tmp_proc_info_pt;
    proc_info_pt = tmp_proc_info_pt;
    proc_info_pt->pid = pid;
    proc_info_pt->timeout = proc_cmd_pt->timeout;
    proc_info_pt->name_hash = hash_calculate(proc_cmd_pt->proc_name, 0);
    proc_info_pt->cmd_hash = hash_calculate(proc_cmd_pt->start_cmd, 0);
    memcpy(proc_info_pt->proc_name, proc_cmd_pt->proc_name, PROC_NAME_MAX_LEN-1);
    memcpy(proc_info_pt->start_cmd, proc_cmd_pt->start_cmd, PROC_CMD_LINE_MAX_LEN-1);

    LOG("%s pid=%d timeout=%d cmd=\"%s\"\n", 
        proc_info_pt->proc_name, proc_info_pt->pid, proc_info_pt->timeout, proc_info_pt->start_cmd);
    
    return 0;
}

int proc_keep_alive(proc_cmd_t *proc_cmd_pt, void *p_arg)
{
    proc_info_t *proc_info_pt;
    register int pid;

    pid = proc_cmd_pt->pid;
    if(1 >= pid)
        return -1;
    
    proc_info_pt = g_proc_info.next_pt;
    while(NULL != proc_info_pt)
    {
        if(proc_info_pt->pid == pid)
            break;
        
        proc_info_pt = proc_info_pt->next_pt;
    }

    if(NULL == proc_info_pt)
        return -1;

    proc_info_pt->time_cnt = 0;
    return 0;
}

int proc_update_pid(proc_cmd_t *proc_cmd_pt, void *p_arg)
{
    proc_info_t *proc_info_pt;
    uint32_t name_hash;
    uint32_t cmd_hash;

    if(0 != proc_check_cmd(proc_cmd_pt))
        return -1;
    
    name_hash = hash_calculate(proc_cmd_pt->proc_name, 0);
    cmd_hash = hash_calculate(proc_cmd_pt->start_cmd, 0);

    proc_info_pt = g_proc_info.next_pt;
    while(NULL != proc_info_pt)
    {
        if((name_hash == proc_info_pt->name_hash) && (cmd_hash == proc_info_pt->cmd_hash))
        {
            if((0 == strcmp(proc_cmd_pt->proc_name, proc_info_pt->proc_name)) && 
                (0 == strcmp(proc_cmd_pt->start_cmd, proc_info_pt->start_cmd)))
            {
                LOG("Update pid %d as %d\n", proc_info_pt->pid, proc_cmd_pt->pid);
                proc_info_pt->pid = proc_cmd_pt->pid;
                return 0;
            }
        }
        
        proc_info_pt = proc_info_pt->next_pt;
    }

    return -1;
}

int proc_cancle(proc_cmd_t *proc_cmd_pt, void *p_arg)
{
    proc_info_t *proc_info_pt;
    proc_info_t *prev_proc_info_pt;
    int pid;
    uint32_t name_hash;
    uint32_t cmd_hash;

    if(0 != proc_check_cmd(proc_cmd_pt))
        return -1;

    pid = proc_cmd_pt->pid;
    name_hash = hash_calculate(proc_cmd_pt->proc_name, 0);
    cmd_hash = hash_calculate(proc_cmd_pt->start_cmd, 0);

    prev_proc_info_pt = &g_proc_info;
    proc_info_pt = g_proc_info.next_pt;
    while(NULL != proc_info_pt)
    {
        if((pid == proc_info_pt->pid) && (name_hash == proc_info_pt->name_hash) && 
            (cmd_hash == proc_info_pt->cmd_hash))
        {
            if((0 == strcmp(proc_cmd_pt->proc_name, proc_info_pt->proc_name)) && 
                (0 == strcmp(proc_cmd_pt->start_cmd, proc_info_pt->start_cmd)))
            {
                prev_proc_info_pt->next_pt = proc_info_pt->next_pt;
                free(proc_info_pt);
                LOG("Pid %d exit\n", pid);
                return 0;
            }
        }

        prev_proc_info_pt = proc_info_pt;
        proc_info_pt = proc_info_pt->next_pt;
    }

    return 0;
}

int main(int argc, char **argv)
{
    logcfg_t logcfg;
    int fd;
    int res;
    proc_cmd_t proc_cmd;
    int i;
    int cnt;
    struct timeval tv;

    memset(&g_proc_info, 0, sizeof(g_proc_info));
    cnt = 0;

    if(0 != log_load(&logcfg, "pm.conf"))
    {
        STDERR("log_load(): %s\n", strerror_rw(errno));
        return -1;
    }

    if(0 != log_get(&logcfg, &g_log, 1))
    {
        STDERR("log_get(): %s\n", strerror_rw(errno));
        log_unload(&logcfg);
        return -1;
    }

    log_unload(&logcfg);

    if(0 != dir_create(BASE_PATH_PIPE, 0755))
    {
        STDERR("dir_create(%s): %s\n", BASE_PATH_PIPE, strerror(errno));
        return -1;
    }

    if(0 != queue_create(PROC_PIPE_NAME, 0755))
    {
        STDERR("queue_create(%s): %s\n", PROC_PIPE_NAME, strerror(errno));
        return -1;
    }

    fd = open(PROC_PIPE_NAME, O_RDWR);
    if(0 > fd)
        return -1;

    LOG("Process monitor daemon ready\n");

    while(1)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        res = queue_get_tv(fd, &tv, &proc_cmd, sizeof(proc_cmd));
        if(0 == res)
        {
            cnt = 0;
            proc_monitor();
            continue;
        }
        
        cnt += 1000000 - tv.tv_usec;
        if(cnt >= 1000000)
        {
            cnt = 0;
            proc_monitor();
        }

        if(sizeof(proc_cmd) != res)
            continue;

        for(i = 0; i < ARRAY_SIZE(cmd_tbl); i++)
        {
            if(cmd_tbl[i].cmd == proc_cmd.cmd)
                break;
        }

        if((ARRAY_SIZE(cmd_tbl) == i) || (0 == cmd_tbl[i].enable))
        {
            LOG("Cmd not found: 0x%x\n", proc_cmd.cmd);
            continue;
        }

        (*cmd_tbl[i].cmd_hndlr)(&proc_cmd, NULL);
    }

    return 0;
}
