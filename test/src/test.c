/*************************************************************************
	> File Name: test.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 17/10/2018
 ************************************************************************/

#include "errno_rw.h"
#include "procmonitor.h"
#include "queue.h"
#include "file.h"
#include "parser.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

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
    {
        buf[size] = 0;
    }

    sscanf(buf, "%*s %s", buf_p);

    return 0;
}

int main(void)
{
    int fd;
    proc_cmd_t proc_cmd;
    stat_t f_info;

    printf("%ld\n", sizeof(f_info.st_size));

    fd = open(PROC_PIPE_NAME, O_RDWR);
    //fd = open("../procmonitor/eBMC/pipe/procmonitor", O_RDWR);
    if(0 > fd)
    {
        STDERR("open(%s): %s\n", PROC_PIPE_NAME, strerror_rw(errno));
        return -1;
    }

    memset(&proc_cmd, 0, sizeof(proc_cmd));
    proc_cmd.cmd = PROC_CMD_REGISTER;
    proc_cmd.pid = getpid();
    proc_cmd.timeout = 5;
    proc_get_name(proc_cmd.pid, proc_cmd.proc_name, PROC_NAME_MAX_LEN);
    sprintf(proc_cmd.start_cmd, "/lvsenlv/code/eBMC/test/test");
    queue_set(fd, &proc_cmd, sizeof(proc_cmd));

    proc_cmd.cmd = PROC_CMD_KEEP_ALIVE;
    int i;
    
    for(i = 0; i < 70; i++)
    {
        //queue_set(fd, &proc_cmd, sizeof(proc_cmd));
        sleep(1);
    }

    proc_cmd.cmd = PROC_CMD_EXIT;
    queue_set(fd, &proc_cmd, sizeof(proc_cmd));
    
    return 0;
}
