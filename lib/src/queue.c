/*************************************************************************
	> File Name: queue.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 16/10/2018
 ************************************************************************/

#include "queue.h"
#include "errno_rw.h"
#include "file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static int queue_write_lock(int fd)
{
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = (pid_t)0;
    
    if(-1 == fcntl(fd, F_SETLKW, &lock))
        return -1;

    return 0;
}

static int queue_unlock(int fd)
{
    struct flock lock;

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = (pid_t)0;
    
    if(-1 == fcntl(fd, F_SETLK, &lock))
        return -1;

    return 0;
}

int queue_create(const char *q_name_p, int mode)
{
    if(NULL == q_name_p)
    {
        errno = EINVAL;
        return -1;
    }
    
    if(0 == access(q_name_p, F_OK))
    {
        if(0 != unlink(q_name_p))
            return -1;
    }

    if(0 != mkfifo(q_name_p, mode))
        return -1;
    
    return 0;
}

int queue_get(int fd, int sec, void *buf, int size)
{
    struct timeval tv;
    fd_set rfd;
    int mfd;
    int res;
    
    if((0 > fd) || (0 > sec) || (NULL == buf) || (0 > size))
    {
        errno = EINVAL;
        return -1;
    }

    tv.tv_sec = sec;
    tv.tv_usec = 0;
    FD_ZERO(&rfd);
    FD_SET(fd, &rfd);
    mfd = fd + 1;

    res = select(mfd, &rfd, NULL, NULL, &tv);
    if(0 > res)
        return -1;

    if(0 == res)
        return 0;

    res = read(fd, buf, size);
    if(0 > res)
        return -1;

    return res;
}

int queue_get_tv(int fd, struct timeval *tv, void *buf, int size)
{
    fd_set rfd;
    int mfd;
    int res;
    
    if((0 > fd) || (NULL == tv) || (NULL == buf) || (0 > size))
    {
        errno = EINVAL;
        return -1;
    }

    FD_ZERO(&rfd);
    FD_SET(fd, &rfd);
    mfd = fd + 1;

    res = select(mfd, &rfd, NULL, NULL, tv);
    if(0 > res)
        return -1;

    if(0 == res)
        return 0;

    res = read(fd, buf, size);
    if(0 > res)
        return -1;

    return res;
}

int queue_set(int fd, void *buf, int size)
{
    int res;
    
    if((0 > fd) || (NULL == buf) || (0 > size))
    {
        errno = EINVAL;
        return -1;
    }
    
    res = write(fd, buf, size);
    if(0 > res)
        return -1;

    return res;
}

int queue_set_safely(int fd, void *buf, int size)
{
    int res;
    
    if((0 > fd) || (NULL == buf) || (0 > size))
    {
        errno = EINVAL;
        return -1;
    }

    queue_write_lock(fd);
    res = write(fd, buf, size);
    queue_unlock(fd);
    
    if(0 > res)
        return -1;

    return res;
}
