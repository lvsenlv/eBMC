/*************************************************************************
	> File Name: file.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 16/10/2018
 ************************************************************************/

#ifndef __FILE_H
#define __FILE_H

#include "common.h"

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS                       64 //Make sure st_size is 64bits instead of 32bits
#endif

#define FILE_NAME_MAX_LENGTH                    128

#ifdef __LINUX
#define DIR_DELIMITER                           '/'
typedef struct stat                             stat_t;
#define file_get_info(name, info)               lstat(name, info)
#elif defined __WINDOWS
#define DIR_DELIMITER                           '\\'
typedef struct _stati64                         stat_t;
#define file_get_info(name, info)               _stati64(name, info)
#endif

#define dir_create                              dir_create_recursion
#define file_create_r                           file_create_recursion

int dir_check(const char *ptr);
int dir_create_recursion(const char *dir_p, int mode);
int dir_delete(const char *folder_p);
int file_check(const char *ptr);
int file_create(const char *f_name_p, int mode);
int file_create_recursion(const char *f_name_p, int mode);
uint64_t file_get_size(const char *f_name_p);
int fd_wr_lock(int fd);
int fd_rd_lock(int fd);
int fd_rw_unlock(int fd);

#endif
