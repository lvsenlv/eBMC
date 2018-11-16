/*************************************************************************
	> File Name: parser.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 22/10/2018
 ************************************************************************/

#include "parser.h"
#include "errno_rw.h"
#include "file.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define PAR_ATTR_CMT                            0x01
#define PAR_ATTR_SEC                            0x02
#define PAR_ATTR_KEY                            0x03

/*
    attr[0:7]:      attr
    attr[8:15]:     multi of data[0]
    attr[16:23]:    multi of data[1]
*/
#define PAR_SET_ATTR(t, s)                      (t |= s)
#define PAR_GET_ATTR(t)                         (t & 0xFF)
#define PAR_SET_DATA0_MULTI(t, s)               (t |= (s<<8))
#define PAR_GET_DATA0_MULTI(t)                  ((t>>8) & 0xFF)
#define PAR_SET_DATA1_MULTI(t, s)               (t |= (s<<16))
#define PAR_GET_DATA1_MULTI(t)                  ((t>>16) & 0xFF)

static char *strskp(char *str)
{
    char *s;

    if(NULL == str)
        return NULL;

    s = str;
    while((0 != *s) && (isspace(*s)))
    {
        s++;
    }

    return s;
}

static int parse_sec(char *str, char *ptr, int size)
{
    char *src;
    char *dest;

    if((NULL == str) || (NULL == ptr) || (0 == size))
        return -1;

    src = str;
    dest = ptr;
    while((0 != *src) && (isspace(*src)))
    {
        src++;
    }

    if('[' != *src++)
    {
        *ptr = 0;
        return -1;
    }

    while((0 != *src) && (isspace(*src)))
    {
        src++;
    }

    if((0 == *src) || (']' == *src))
    {
        *ptr = 0;
        return -1;
    }

    while((0 != *src) && (']' != *src))
    {
        *dest++ = *src++;
        size--;

        if(0 == size)
        {
            *(dest-1) = 0;
            return 0;
        }
    }

    if(0 == *src)
    {
        *ptr = 0;
        return -1;
    }

    *dest = 0;
    size = dest - ptr;
    while(size > 0)
    {
        if(!isspace(ptr[size-1]))
            break;

        size--;
    }

    ptr[size] = 0;
    
    return 0;
}

static line_t *alloc_new_line(const char *sec_p, const char *key_p, const char *val_p, int attr)
{
    line_t *line_pt;
    const char *ptr;
    int multi;
    int len;

    if(PAR_ATTR_KEY == attr)
    {
        if(NULL == key_p)
            return NULL;

        ptr = key_p;
    }
    else if((PAR_ATTR_CMT == attr) || (PAR_ATTR_SEC == attr))
    {
        if(NULL == sec_p)
            return NULL;
            
        ptr = sec_p;
    }
    else
        return NULL;

    len = strlen(ptr) + 1;
    if((PAR_ONE_LINE_MAX_LEN < len) || (1 == len))
        return NULL;

    line_pt = (line_t *)calloc(1, sizeof(line_t));
    if(NULL == line_pt)
        return NULL;

    if('\n' == ptr[len-2])
    {
        len--;
    }

    if(PAR_LINE_ALIGN_SIZE >= len)
    {
        multi = 1;
        line_pt->data[0] = malloc(PAR_LINE_ALIGN_SIZE);
    }
    else
    {
        multi = (0 == (len%PAR_LINE_ALIGN_SIZE)) ? (len/PAR_LINE_ALIGN_SIZE) : (len/PAR_LINE_ALIGN_SIZE+1);
        line_pt->data[0] = malloc(multi*PAR_LINE_ALIGN_SIZE);
        if(NULL == line_pt->data[0])
        {
            free(line_pt);
            return NULL;
        }
    }

    PAR_SET_DATA0_MULTI(line_pt->attr, multi);
    PAR_SET_ATTR(line_pt->attr, attr);
    memcpy(line_pt->data[0], ptr, len);
    line_pt->data[0][len-1] = 0;
    
    if(PAR_ATTR_CMT == attr)
        return line_pt;
    
    if(PAR_ATTR_SEC == attr)
    {
        line_pt->hash = hash_calculate(ptr, len-1);
        return line_pt;
    }

    line_pt->hash = hash_calculate_sk(sec_p, key_p, 0, len-1);
    if(NULL == val_p)
        return line_pt;

    len = strlen(val_p) + 1;
    if(1 == len)
        return line_pt;

    if('\n' == val_p[len-2])
    {
        len--;
    }
    
    if(PAR_LINE_ALIGN_SIZE >= len)
    {
        multi = 1;
        line_pt->data[1] = malloc(PAR_LINE_ALIGN_SIZE);
    }
    else
    {
        multi = (0 == (len%PAR_LINE_ALIGN_SIZE)) ? (len/PAR_LINE_ALIGN_SIZE) : (len/PAR_LINE_ALIGN_SIZE+1);
        line_pt->data[1] = malloc(multi*PAR_LINE_ALIGN_SIZE);
        if(NULL == line_pt->data[1])
        {
            free(line_pt->data[0]);
            free(line_pt);
            return NULL;
        }
    }
    
    PAR_SET_DATA1_MULTI(line_pt->attr, multi);
    memcpy(line_pt->data[1], val_p, len);
    line_pt->data[1][len-1] = 0;
    
    return line_pt;
}

static void par_append_comment(parser_t *parser_pt, const char *str_p)
{
    line_t *line_pt;
    
    if((NULL == parser_pt) || (NULL == str_p))
        return;
    
    line_pt = alloc_new_line(str_p, NULL, NULL, PAR_ATTR_CMT);
    if(NULL == line_pt)
        return;
    
    if(NULL != parser_pt->tail_pt)
    {
        parser_pt->tail_pt->next_pt = line_pt;
        line_pt->prev_pt = parser_pt->tail_pt;
        parser_pt->tail_pt = line_pt;
    }
    else
    {
        parser_pt->line_pt = line_pt;
        parser_pt->tail_pt = line_pt;
    }
}

static void par_append_sec(parser_t *parser_pt, const char *str_p)
{
    line_t *line_pt;
    
    if((NULL == parser_pt) || (NULL == str_p))
        return;
    
    line_pt = alloc_new_line(str_p, NULL, NULL, PAR_ATTR_SEC);
    if(NULL == line_pt)
        return;

    if(NULL != parser_pt->tail_pt)
    {
        parser_pt->tail_pt->next_pt = line_pt;
        line_pt->prev_pt = parser_pt->tail_pt;
        parser_pt->tail_pt = line_pt;
    }
    else
    {
        parser_pt->line_pt = line_pt;
        parser_pt->tail_pt = line_pt;
    }
}

static void par_append_key(parser_t *parser_pt, const char *sec_p, const char *key_p, const char *val_p)
{
    line_t *line_pt;
    
    if((NULL == parser_pt) || (NULL == key_p) || (NULL == val_p))
        return;
    
    line_pt = alloc_new_line(sec_p, key_p, val_p, PAR_ATTR_KEY);
    if(NULL == line_pt)
        return;
    
    if(NULL != parser_pt->tail_pt)
    {
        parser_pt->tail_pt->next_pt = line_pt;
        line_pt->prev_pt = parser_pt->tail_pt;
        parser_pt->tail_pt = line_pt;
    }
    else
    {
        parser_pt->line_pt = line_pt;
        parser_pt->tail_pt = line_pt;
    }
}

parser_t *par_load(const char *f_name_p)
{
    uint64_t f_size;
    parser_t *parser_pt;
    FILE *fp;
    char buf[PAR_ONE_LINE_MAX_LEN];
    char sec[PAR_ONE_LINE_MAX_LEN];
    char key[PAR_ONE_LINE_MAX_LEN];
    char val[PAR_ONE_LINE_MAX_LEN];
    char prev_sec[PAR_ONE_LINE_MAX_LEN];
    char *data_p;
    
    if(NULL == f_name_p)
    {
        errno = EINVAL;
        return NULL;
    }

    if(0 != access(f_name_p, F_OK))
    {
        if(0 != file_create(f_name_p, 0644));
            return NULL;

        f_size = 0;
    }
    else
    {
        f_size = file_get_size(f_name_p);
        if(PAR_FILE_MAX_SIZE < f_size)
        {
            errno = EFBIG;
            return NULL;
        }
    }

    fp = fopen(f_name_p, "rb");
    if(NULL == fp)
        return NULL;
    
    parser_pt = (parser_t *)calloc(1, sizeof(parser_t));
    if(NULL == parser_pt)
    {
        fclose(fp);
        return NULL;
    }

    sec[0] = 0;
    key[0] = 0;
    val[0] = 0;
    while((NULL != fgets(buf, sizeof(buf), fp)) && (0 == feof(fp)))
    {
        data_p = strskp(buf);
        if(0 == *data_p)
            continue;

        if((';' == *data_p) || ('#' == *data_p))
        {
            par_append_comment(parser_pt, data_p);
            continue;
        }

        if(0 == parse_sec(data_p, sec, sizeof(sec)))
        {
            par_append_sec(parser_pt, sec);
            strcpy(prev_sec, sec);
            continue;
        }

        if((2 == sscanf(data_p, "%[^ =] = \"%[^\"]\"", key, val)) || 
            (2 == sscanf(data_p, "%[^ =] = '%[^\']'", key, val)) || 
            (2 == sscanf(data_p, "%[^ =] = %[^;#]", key, val)))
        {
            par_append_key(parser_pt, prev_sec, key, val);
            continue;
        }
    }

    fclose(fp);
    return parser_pt;
}

void par_unload(parser_t *parser_pt)
{
    line_t *line_pt;
    
    if(NULL == parser_pt)
        return;

    line_pt = parser_pt->line_pt;

    while(NULL != line_pt)
    {
        if(NULL != line_pt->data[0])
        {
            free(line_pt->data[0]);
        }

        if(NULL != line_pt->data[1])
        {
            free(line_pt->data[1]);
        }
        
        line_pt = line_pt->next_pt;
    }

    free(parser_pt);
}

void par_dump(parser_t *parser_pt, FILE *fp)
{
    line_t *line_pt;
    
    if((NULL == parser_pt) || (NULL == fp))
        return;

    line_pt = parser_pt->line_pt;
    while(NULL != line_pt)
    {
        if(PAR_ATTR_SEC == PAR_GET_ATTR(line_pt->attr))
        {
            fprintf(fp, "\n[%s]\n", line_pt->data[0]);
        }
        else if(PAR_ATTR_KEY == PAR_GET_ATTR(line_pt->attr))
        {
            fprintf(fp, "%s = %s\n", line_pt->data[0], line_pt->data[1]);
        }
        else
        {
            fprintf(fp, "%s\n", line_pt->data[0]);
        }

        line_pt = line_pt->next_pt;
    }
}

int par_getnsec(parser_t *parser_pt)
{
    line_t *line_pt;
    int cnt;

    if(NULL == parser_pt)
        return 0;

    cnt = 0;
    line_pt = parser_pt->line_pt;
    while(NULL != line_pt)
    {
        if(PAR_ATTR_SEC == PAR_GET_ATTR(line_pt->attr))
        {
            cnt++;
        }

        line_pt = line_pt->next_pt;
    }

    return cnt;
}

char *par_getsecname(parser_t *parser_pt, int index)
{
    line_t *line_pt;
    int cnt;

    if((NULL == parser_pt) || (0 >= index))
        return NULL;

    cnt = 0;
    line_pt = parser_pt->line_pt;
    while(NULL != line_pt)
    {
        if(PAR_ATTR_SEC == PAR_GET_ATTR(line_pt->attr))
        {
            cnt++;
            if(cnt == index)
                break;
        }

        line_pt = line_pt->next_pt;
    }

    if(NULL == line_pt)
        return NULL;

    return line_pt->data[0];
}

char *par_getstring(parser_t *parser_pt, const char *sec_p, const char *key_p, char *def)
{
    line_t *line_pt;
    uint32_t hash;
    
    if(NULL == parser_pt)
        return def;

    hash = hash_calculate_sk(sec_p, key_p, 0, 0);
    if(0 == hash)
        return def;

    line_pt = parser_pt->line_pt;
    while(NULL != line_pt)
    {
        if(hash != line_pt->hash)
        {
            line_pt = line_pt->next_pt;
            continue;
        }

        if(0 != strcmp(key_p, line_pt->data[0]))
            continue;

        if(NULL != line_pt->data[1])
            return line_pt->data[1];

        break;
    }

    return def;
}

int par_getint(parser_t *parser_pt, const char *sec_p, const char *key_p, int def)
{
    char *ptr;

    ptr = par_getstring(parser_pt, sec_p, key_p, NULL);
    if(NULL == ptr)
        return def;
    
    return atoi(ptr);
}

int par_getboolean(parser_t *parser_pt, const char *sec_p, const char *key_p, int def)
{
    char *ptr;

    ptr = par_getstring(parser_pt, sec_p, key_p, NULL);
    if(NULL == ptr)
        return def;

    if(('y' == *ptr) || ('Y' == *ptr) || ('1' == *ptr) || ('t' == *ptr) || ('T' == *ptr))
        return 1;

    if(('n' == *ptr) || ('N' == *ptr) || ('0' == *ptr) || ('f' == *ptr) || ('F' == *ptr))
        return 0;

    return def;
}

double par_getdouble(parser_t *parser_pt, const char *sec_p, const char *key_p, double def)
{
    char *ptr;

    ptr = par_getstring(parser_pt, sec_p, key_p, NULL);
    if(NULL == ptr)
        return def;
    
    return atof(ptr);
}

void par_setstring(parser_t *parser_pt, const char *sec_p, const char *key_p, const char *val_p)
{
    line_t *line_pt;
    line_t *tmp_line_pt;
    uint32_t hash;
    char not_found;
    int len;
    int multi;
    
    if((NULL == parser_pt) || (NULL == key_p) || (NULL == val_p))
        return;

    hash = hash_calculate(sec_p, 0);
    if(0 == hash)
        return;

    not_found = 0;
    line_pt = parser_pt->line_pt;
    
    while(NULL != line_pt)
    {
        if(hash != line_pt->hash)
        {
            line_pt = line_pt->next_pt;
            continue;
        }

        if(0 != strcmp(sec_p, line_pt->data[0]))
            continue;

        not_found = 1;
        break;
    }

    if(NULL == line_pt) //Sec was not found
    {
        par_append_sec(parser_pt, sec_p);
        par_append_key(parser_pt, sec_p, key_p, val_p);
        return;
    }

    line_pt = line_pt->next_pt;
    hash = hash_calculate_sk(sec_p, key_p, 0, 0);
    
    while(NULL != line_pt)
    {
        if(PAR_ATTR_SEC == PAR_GET_ATTR(line_pt->attr))
        {
            not_found = 2;
            break;
        }
        
        if(hash != line_pt->hash)
        {
            line_pt = line_pt->next_pt;
            continue;
        }

        if(0 != strcmp(key_p, line_pt->data[0]))
            break;

        not_found = 3;
        break;
    }

    if(1 == not_found) //Only sec was found and it is the end line
    {
        par_append_key(parser_pt, sec_p, key_p, val_p);
    }
    else if(2 == not_found) //Insert the key to known sec
    {
        tmp_line_pt = alloc_new_line(sec_p, key_p, val_p, PAR_ATTR_KEY);
        tmp_line_pt->next_pt = line_pt;
        tmp_line_pt->prev_pt = line_pt->prev_pt;
        line_pt->prev_pt = tmp_line_pt;
        line_pt = tmp_line_pt->prev_pt;
        line_pt->next_pt = tmp_line_pt;
    }
    else if(3 == not_found) //Modify the val
    {
        len = strlen(val_p) + 1;
        if(1 == len)
            return;

        if((PAR_ONE_LINE_MAX_LEN*PAR_GET_DATA1_MULTI(line_pt->attr)) > len)
        {
            memcpy(line_pt->data[1], val_p, len);
            line_pt->data[1][len-1] = 0;
        }
        else
        {
            if(NULL != line_pt->data[1])
            {
                free(line_pt->data[1]);
            }
            
            multi = (0 == (len%PAR_LINE_ALIGN_SIZE)) ? (len/PAR_LINE_ALIGN_SIZE) : (len/PAR_LINE_ALIGN_SIZE+1);
            line_pt->data[1] = malloc(multi*PAR_LINE_ALIGN_SIZE);
            if(NULL == line_pt->data[1])
                return;

            PAR_SET_DATA1_MULTI(line_pt->attr, multi);
            memcpy(line_pt->data[1], val_p, len);
            line_pt->data[1][len-1] = 0;
        }
    }
}

void par_setint(parser_t *parser_pt, const char *sec_p, const char *key_p, int num)
{
    char val[PAR_ONE_LINE_MAX_LEN];

    snprintf(val, sizeof(val), "%d", num);
    par_setstring(parser_pt, sec_p, key_p, val);
}

void par_setboolean(parser_t *parser_pt, const char *sec_p, const char *key_p, int num)
{
    char val[2];

    val[0] = (num) ? 'y' : 'n';
    val[1] = 0;
    par_setstring(parser_pt, sec_p, key_p, val);
}

void par_setdouble(parser_t *parser_pt, const char *sec_p, const char *key_p, double num)
{
    char val[PAR_ONE_LINE_MAX_LEN];

    snprintf(val, sizeof(val), "%lf", num);
    par_setstring(parser_pt, sec_p, key_p, val);
}

int par_main(int argc, char **argv)
{
    parser_t *parser_pt;

    parser_pt = par_load("mt.log");
    if(NULL == parser_pt)
    {
        STDOUT("%s\n", strerror_rw(errno));
        return -1;
    }

    STDOUT("--- Pristine ---\n\n");
    par_dump(parser_pt, stdout);

    par_setstring(parser_pt, "log1", "path", "/var/log/log1");
    par_setstring(parser_pt, "log2", "path", "/var/log/log2");
    par_setstring(parser_pt, "log3", "path", "/var/log/log3");
    
    par_setdouble(parser_pt, "log1", "size", 10241);
    par_setdouble(parser_pt, "log2", "size", 10242);
    par_setdouble(parser_pt, "log3", "size", 10243);
    
    par_setint(parser_pt, "log1", "rotate", 1);
    par_setint(parser_pt, "log2", "rotate", 2);
    par_setint(parser_pt, "log3", "rotate", 3);
    
    par_setboolean(parser_pt, "log1", "enable", 1);
    par_setboolean(parser_pt, "log2", "enable", 1);
    par_setboolean(parser_pt, "log3", "enable", 1);
    
    STDOUT("\n--- After setting ---\n\n");
    par_dump(parser_pt, stdout);
    
    par_unload(parser_pt);

    return 0;
}
