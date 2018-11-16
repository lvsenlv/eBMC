/*************************************************************************
	> File Name: common.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 23/09/2018
 ************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long   uint64_t;

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS                       64 //Make sure st_size is 64bits instead of 32bits
#endif

#include <stdio.h>

#define PACKED                                  __attribute__((packed))
#define BASE_PATH_VAR                           "/lvsenlv/code/eBMC/procmonitor"
#define BASE_PATH_PIPE                          BASE_PATH_VAR"/pipe"
#define ARRAY_SIZE(a)                           (sizeof(a)/sizeof(a[0]))

#define STDOUT(format, args...) \
        do \
        { \
            fprintf(stdout, format, ##args); \
        }while(0)
        
#define STDERR(format, args...) \
        do \
        { \
            fprintf(stderr, format, ##args); \
        }while(0)

#ifndef __DEBUG
#define DEBUG(format, args...)                  ((void)0)
#define DEBUG_ERR(format, args...)              ((void)0)
#else
#define DEBUG(format, args...) \
        do \
        { \
            fprintf(stdout, format, ##args); \
        }while(0)
        
#define DEBUG_ERR(format, args...) \
        do \
        { \
            fprintf(stderr, "[%s][%d] ", __func__, __LINE__); \
            fprintf(stderr, format, ##args); \
        }while(0)
#endif

#endif
