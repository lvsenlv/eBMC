/*************************************************************************
	> File Name: crc.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 30/03/2018
 ************************************************************************/

#include "crc.h"

#ifndef __CREATE_CRC_TABLE
#include "crc_table.c"
extern uint8_t  crc8_table[];
extern uint16_t crc16_table[];
extern uint32_t crc32_table[];
#else
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t  crc8_table[256];
static uint16_t crc16_table[256];
static uint32_t crc32_table[256];

void crc8_init_table(void)
{
    int i;
    int j;
    uint8_t data;
    uint8_t crc;

    for(i = 0; i <= 0xFF; i++)    
    {
        crc = 0;
        data = i;
        for(j = 0; j < 8; j++)
        {
            if((data^crc) & 0x80)
            {
                crc = (crc << 1) ^ CRC8_POLY;
            }
            else
            {
                crc <<= 1;
            }

            data <<= 1;
        }

        crc8_table[i] = crc;
    }
}

void crc16_init_table(void)
{
    int i;
    int j;
    uint16_t data;
    uint16_t crc;

    for(i = 0; i <= 0xFF; i++)    
    {
        crc = 0;
        data = i << 8;
        for(j = 0; j < 8; j++)
        {
            if((data^crc) & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_POLY;
            }
            else
            {
                crc <<= 1;
            }

            data <<= 1;
        }

        crc16_table[i] = crc;
    }
}

void crc32_init_table(void)
{
    int i;
    int j;
    uint32_t data;
    uint32_t crc;

    for(i = 0; i <= 0xFF; i++)
    {
        data = i << 24;
        crc = 0;
        
        for(j = 0; j < 8; j++)
        {
            if((data^crc) & 0x80000000)
            {
                crc = (crc << 1) ^ CRC32_POLY;
            }
            else
            {
                crc <<= 1;
            }

            data <<= 1;
        }

        crc32_table[i] = crc;
    }
}

void create_table(void)
{
    char *f_name_p = "./crc_table.c";
    FILE *fp;
    int i;
    char buf[1024*8];
    char *buf_p;
    int res;

    fp = fopen(f_name_p, "w+");
    if(NULL == fp)
        return;

    crc8_init_table();
    crc16_init_table();
    crc32_init_table();

    buf_p = buf;

    buf_p += sprintf(buf_p, 
        "/*\n"
        "   CRC-8-Dallas/Maxim\n"
        "   Poly: 0x%x\n"
        "   Init: 0x%x\n"
        "*/\n", CRC8_POLY, CRC8_INIT);
    buf_p += sprintf(buf_p, "unsigned char crc8_table[] = {");
    for(i = 0; i <= 0xFF; i++)
    {
        if(0 == i%8)
        {
            buf_p += sprintf(buf_p, "\n    ");
        }

        buf_p += sprintf(buf_p, "0x%02x, ", crc8_table[i]);
    }
    buf_p += sprintf(buf_p, "\n};\n\n");
    
    buf_p += sprintf(buf_p, 
        "/*\n"
        "   CRC-16-IBM\n"
        "   Poly: 0x%x\n"
        "   Init: 0x%x\n"
        "*/\n", CRC16_POLY, CRC16_INIT);
    buf_p += sprintf(buf_p, "unsigned short crc16_table[] = {");
    for(i = 0; i <= 0xFF; i++)
    {
        if(0 == i%8)
        {
            buf_p += sprintf(buf_p, "\n    ");
        }

        buf_p += sprintf(buf_p, "0x%04x, ", crc16_table[i]);
    }
    buf_p += sprintf(buf_p, "\n};\n\n");
    
    buf_p += sprintf(buf_p, 
        "/*\n"
        "   CRC-32\n"
        "   Poly: 0x%x\n"
        "   Init: 0x%x\n"
        "*/\n", CRC32_POLY, CRC32_INIT);
    buf_p += sprintf(buf_p, "unsigned int crc32_table[] = {");
    for(i = 0; i <= 0xFF; i++)
    {
        if(0 == i%8)
        {
            buf_p += sprintf(buf_p, "\n    ");
        }

        buf_p += sprintf(buf_p, "0x%08x, ", crc32_table[i]);
    }
    buf_p += sprintf(buf_p, "\n};\n");

    res = fwrite(buf, 1, buf_p-buf, fp);
    if(res != (buf_p-buf))
    {
        fclose(fp);
        return;
    }
    
    fclose(fp);
}
#endif

uint8_t crc8_calculate_directly(void *data, int size)
{
    int i;
    uint8_t crc;
    uint8_t *data_p = (uint8_t *)data;
    
    crc = CRC8_INIT;
    
    while(0 < size--)
    {
        crc ^= *data_p++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ CRC8_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint8_t crc8_calculate(void *data, int size)
{
    uint8_t crc;
    uint8_t *data_p = (uint8_t *)data;

    crc = CRC8_INIT;

    while(0 < size--)
    {
        crc = (crc << 8) ^ crc8_table[crc ^ *data_p++];
    }
    
    return crc;
}

uint16_t crc16_calculate_directly(void *data, int size)
{
    int i;
    uint16_t crc;
    uint8_t *data_p = (uint8_t *)data;
    
    crc = CRC16_INIT;
    
    while(0 < size--)
    {
        crc ^= (*data_p++ << 8);
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint16_t crc16_calculate(void *data, int size)
{
    uint16_t crc;
    uint8_t *data_p = (uint8_t *)data;

    crc = CRC16_INIT;

    while(0 < size--)
    {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ *data_p++) & 0xFF];
    }
    
    return crc;
}

uint32_t crc32_calculate_directly(void *data, int size)
{
    int i;
    uint32_t crc;
    uint8_t *data_p = (uint8_t *)data;
    
    crc = CRC32_INIT;
    
    while(0 < size--)
    {
        crc ^= (*data_p++ << 24);
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80000000)
            {
                crc = (crc << 1) ^ CRC32_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint32_t crc32_calculate(void *data, int size)
{
    uint32_t crc;
    uint8_t *data_p = (uint8_t *)data;

    crc = CRC32_INIT;

    while(0 < size--)
    {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *data_p++) & 0xFF];
    }
    
    return crc;
}
