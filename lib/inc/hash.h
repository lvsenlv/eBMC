/*************************************************************************
	> File Name: hash.h
	> Author: 
	> Mail: 
	> Created Time: 18/10/2018
 ************************************************************************/

#ifndef __HASH_H
#define __HASH_H

#include "common.h"

uint32_t hash_calculate(const char *data_p, uint32_t size);
uint32_t hash_calculate_sk(const char *sec_p, const char *key_p, uint32_t sec_len, uint32_t key_len);

#endif
