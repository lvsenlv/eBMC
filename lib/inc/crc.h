/*************************************************************************
	> File Name: crc.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 30/03/2018
 ************************************************************************/

#ifndef __CRC_H
#define __CRC_H

#include "common.h"

/*
    The value of POLY please refer to:
    https://en.wikipedia.org/wiki/Cyclic_redundancy_check
*/

#define CRC8_POLY                           0x31            //CRC-8-Dallas/Maxim
#define CRC8_INIT                           0xFF

#define CRC16_POLY                          0x8005          //CRC-16-IBM
#define CRC16_INIT                          0xFFFF

#define CRC32_POLY                          0x04C11DB7      //CRC-32
#define CRC32_INIT                          0xFFFFFFFF

#ifdef __CREATE_CRC_TABLE
void create_table(void);
void crc8_init_table(void);
void crc16_init_table(void);
void crc32_init_table(void);
#endif

uint8_t crc8_calculate_directly(void *data, int size);
uint8_t crc8_calculate(void *data, int size);
uint16_t crc16_calculate_directly(void *data, int size);
uint16_t crc16_calculate(void *data, int size);
uint32_t crc32_calculate_directly(void *data, int size);
uint32_t crc32_calculate(void *data, int size);

#endif
