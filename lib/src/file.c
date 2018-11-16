/*************************************************************************
	> File Name: file.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 16/10/2018
 ************************************************************************/

#include "file.h"
#include "errno_rw.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

int dir_check(const char *ptr)
{
    stat_t file_info;
    
    if(NULL == ptr)
    {
        errno = EINVAL;
        return -1;
    }

    if(0 != file_get_info(ptr, &file_info))
        return -1;

    if(file_info.st_mode & S_IFDIR)
        return 0;

    errno = ENOTDIR;
    return -1;
}

int dir_create_recursion(const char *dir_p, int mode)
{
    char buf[FILE_NAME_MAX_LENGTH];
    int i;

    if(NULL == dir_p)
    {
        errno = EINVAL;
        return -1;
    }

    for(i = 0; i < FILE_NAME_MAX_LENGTH; i++)
    {
        if('\0' == dir_p[i])
        {
            if(0 == access(dir_p, F_OK))
            {
                if(0 == dir_check(dir_p))
                    return 0;

                errno = ENOTDIR;
                return -1;
            }
            
            if(0 != mkdir(dir_p, mode | 0100))
                return -1;
            
            break;
        }

        if((DIR_DELIMITER != dir_p[i]) || (0 == i))
        {
            buf[i] = dir_p[i];
            continue;
        }

        buf[i] = '\0';
        if(0 == access(buf, F_OK))
        {
            if(0 == dir_check(buf))
            {
                buf[i] = DIR_DELIMITER;
                continue;
            }

            errno = ENOTDIR;
            return -1;
        }

        if(0 != mkdir(buf, mode | 0100))
            return -1;

        buf[i] = DIR_DELIMITER;
    }

    if(FILE_NAME_MAX_LENGTH == i)
    {
        errno = ENAMETOOLONG;   
        return -1;
    }

    return 0;
}

int dir_delete(const char *folder_p)
{
    DIR *dir_p;
    struct dirent *entry_p;
    char f_name[FILE_NAME_MAX_LENGTH];
    stat_t f_info;
    
    if(NULL == folder_p)
    {
        errno = EINVAL;
        return -1;
    }
    
    dir_p = opendir(folder_p);
    if(NULL == dir_p)
        return -1;
    
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
        
        snprintf(f_name, sizeof(f_name), "%s%c%s", folder_p, DIR_DELIMITER, entry_p->d_name);
        f_name[sizeof(f_name)-1] = '\0';

        if(0 != file_get_info(f_name, &f_info))
            return -1;
        
#ifdef __LINUX
        if(DT_DIR == entry_p->d_type)
#elif defined __WINDOWS
        if(f_info.st_mode & S_IFDIR)
#endif
        {
            if(0 != dir_delete(f_name))
                return -1;
        }
#ifdef __LINUX
        else if(DT_REG == entry_p->d_type)
#elif defined __WINDOWS
        else if((f_info.st_mode & S_IFREG) || (f_info.st_mode & S_IFLNK))
#endif
        {
            if(0 != unlink(f_name))
                return -1;
        }
        else
        {
            if(0 != unlink(f_name))
                return -1;
        }
    }
    
    closedir(dir_p);
    if(0 != rmdir(folder_p))
        return -1;
    
    return 0;
}

int file_check(const char *ptr)
{
    stat_t f_info;
    
    if(NULL == ptr)
    {
        errno = EINVAL;
        return -1;
    }

    if(0 != file_get_info(ptr, &f_info))
        return -1;

    if(f_info.st_mode & S_IFREG)
        return 0;

    errno = ENOTFILE;
    return -1;
}

uint64_t file_get_size(const char *f_name_p)
{
    stat_t f_info;
    
    if(NULL == f_name_p)
    {
        errno = EINVAL;
        return 0;
    }

    if(0 != file_get_info(f_name_p, &f_info))
        return 0;

    return (uint64_t)f_info.st_size;
}

int file_create(const char *f_name_p, int mode)
{
    int fd;

    if(NULL == f_name_p)
    {
        errno = EINVAL;
        return -1;
    }

    if(0 == access(f_name_p, F_OK))
    {
        if(0 != file_check(f_name_p))
            return -1;
        
        return 0;
    }
    
    fd = open(f_name_p, O_CREAT, mode);
    if(0 > fd)
        return -1;

    close(fd);
    return 0;
}

int file_create_recursion(const char *f_name_p, int mode)
{
    char buf[FILE_NAME_MAX_LENGTH];
    int len;
    int i;
    
    if(NULL == f_name_p)
    {
        errno = EINVAL;
        return -1;
    }

    len = strlen(f_name_p);
    if(FILE_NAME_MAX_LENGTH <= len)
        return -1;
    
    memcpy(buf, f_name_p, len);
    
    for(i = len; i > 0; i--)
    {
        if(DIR_DELIMITER == f_name_p[i])
            break;
    }

    if(0 == i)
        return file_create(f_name_p, mode);

    buf[i] = '\0';
    if(0 != dir_create_recursion(buf, mode))
        return -1;

    return file_create(f_name_p, mode);
}

int fd_wr_lock(int fd)
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

int fd_rd_lock(int fd)
{
    struct flock lock;

    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = (pid_t)0;
    
    if(-1 == fcntl(fd, F_SETLKW, &lock))
        return -1;

    return 0;
}

int fd_rw_unlock(int fd)
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
