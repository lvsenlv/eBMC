/*************************************************************************
	> File Name: errno_rw.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: Aug 27th,2017 Saturday 09:27:27
 ************************************************************************/

#include "errno_rw.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;
static char *tbl[] = {
    "Illegal file format",
    "Illegal hash value",
    "Not a common file",
    "Null",
};

static void create_key(void)
{
    pthread_key_create(&key, free);
}

int *__errno_location_rw(void)
{
    int *ptr;

    pthread_once(&key_once, create_key);

    ptr = (int *)pthread_getspecific(key);
    if(NULL == ptr)
    {
        ptr = (int *)malloc(sizeof(int));
        if(NULL == ptr)
            return NULL;

        *ptr = 0;
        pthread_setspecific(key, ptr);
    }

    return ptr;
}

char *strerror_rw(int num)
{
    if(134 > num)
        return strerror(num);

    if((134+(sizeof(tbl)/sizeof(char *))) <= num)
        return "Unknown error";

    return tbl[num-134];
}
