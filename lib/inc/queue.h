/*************************************************************************
	> File Name: queue.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 16/10/2018
 ************************************************************************/

#ifndef __QUEUE_H
#define __QUEUE_H

#include <sys/time.h>

int queue_create(const char *q_name_p, int mode);
int queue_get(int fd, int sec, void *buf, int size);
int queue_get_tv(int fd, struct timeval *tv, void *buf, int size);
int queue_set(int fd, void *buf, int size);

#endif
