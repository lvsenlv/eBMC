/*************************************************************************
	> File Name: parser.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 22/10/2018
 ************************************************************************/

#ifndef __PARSER_H
#define __PARSER_H

#include "common.h"

#define PAR_FILE_MAX_SIZE                       (4*1024*1024) //4Mb
#define PAR_ONE_LINE_MAX_LEN                    512
#define PAR_LINE_ALIGN_SIZE                     64

typedef struct line_struct {
    uint32_t hash;
    uint32_t attr;
    char *data[2];
    struct line_struct *prev_pt;
    struct line_struct *next_pt;
}line_t;

typedef struct parser_struct {
    line_t *line_pt;
    line_t *tail_pt;
}parser_t;

parser_t *par_load(const char *f_name_p);
void par_unload(parser_t *parser_pt);
void par_dump(parser_t *parser_pt, FILE *fp);
int par_getnsec(parser_t *parser_pt);
char *par_getsecname(parser_t *parser_pt, int index);
char *par_getstring(parser_t *parser_pt, const char *sec_p, const char *key_p, char *def);
int par_getint(parser_t *parser_pt, const char *sec_p, const char *key_p, int def);
int par_getboolean(parser_t *parser_pt, const char *sec_p, const char *key_p, int def);
double par_getdouble(parser_t *parser_pt, const char *sec_p, const char *key_p, double def);
void par_setstring(parser_t *parser_pt, const char *sec_p, const char *key_p, const char *val_p);
void par_setint(parser_t *parser_pt, const char *sec_p, const char *key_p, int num);
void par_setboolean(parser_t *parser_pt, const char *sec_p, const char *key_p, int num);
void par_setdouble(parser_t *parser_pt, const char *sec_p, const char *key_p, double num);

#endif
