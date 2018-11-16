/*************************************************************************
	> File Name: tmp.c
	> Author: 
	> Mail: 
	> Created Time: 18/10/2018
 ************************************************************************/

#include "hash.h"
#include <string.h>

/*
  This hash function has been taken from an Article in Dr Dobbs Journal.
  This is normally a collision-free function, distributing keys evenly.
  The key is stored anyway in the struct so that collision can be avoided
  by comparing the key itself in last resort.
*/

uint32_t hash_calculate(const char *data_p, uint32_t size)
{
    uint32_t len;
    uint32_t hash;
    uint32_t i;

    if(NULL == data_p)
        return 0;

    hash = 0;
    len = (0 != size) ? size : strlen(data_p);
    
    for(i = 0; i < len; i++)
    {
        hash += (uint32_t)data_p[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    
    return hash;
}

uint32_t hash_calculate_sk(const char *sec_p, const char *key_p, uint32_t sec_len, uint32_t key_len)
{
    uint32_t len;
    uint32_t len_bak;
    uint32_t hash;
    uint32_t i;

    hash = 0;
    len = 0;

    if(NULL != sec_p)
    {
        len = (0 != sec_len) ? sec_len : strlen(sec_p);
        for(i = 0; i < len; i++)
        {
            hash += (uint32_t)sec_p[i];
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    }

    if(NULL != key_p)
    {
        len_bak = len;
        len = (0 != key_len) ? key_len : strlen(key_p);
        
        if((0 != len_bak) && (0 != len))
        {
            hash += ':';
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        
        for(i = 0; i < len; i++)
        {
            hash += (uint32_t)key_p[i];
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    }
    
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    
    return hash;
}

