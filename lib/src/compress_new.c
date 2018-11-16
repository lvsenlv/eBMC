/*************************************************************************
	> File Name: compress.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 05/11/2018
 ************************************************************************/

#include "compress.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define NODE_CNT                                256
#define ELEM_MAX_SIZE                           80

typedef struct hfm_struct {
    uint8_t ch;
    uint64_t weight;
    int parent;
    int l_child;
    int r_child;
}hfm_t;

typedef struct queue_struct {
    int tag;
    int front;
    int rear;
    uint8_t length;
    char elem[ELEM_MAX_SIZE];
}queue_t;

static int sort_tree(hfm_t *hfm_pt)
{
    hfm_t tmp_node;
    int i;
    int j;

    for(i = NODE_CNT - 1; i >= 0; i--)
    {
        for(j = 0; j < i; j++)
        {
            if(hfm_pt[j].weight < hfm_pt[j + 1].weight)
            {
                tmp_node = hfm_pt[j];
                hfm_pt[j] = hfm_pt[j + 1];
                hfm_pt[j + 1] = tmp_node;
            }
        }
    }
    
    for(i = 0; i < NODE_CNT; i++)
    {
        if (0 == hfm_pt[i].weight)
            break;
    }

    return i;
}

static int count_leaf(hfm_t *hfm_pt)
{
    int i;
    
    for(i = 0; i < NODE_CNT; i++)
    {
        if (0 == hfm_pt[i].weight)
            break;
    }
    
    return i;
}

static void get_min_tree(hfm_t *hfm_pt, int n, int *index_p)
{
    int i;
    int tmp;
    uint64_t min;

    for(i = 0; i <= n; i++)
    {
        if(0 == hfm_pt[i].parent)
        {
            min = hfm_pt[i].weight;
            tmp = i;
            break;
        }
    }
    
    for(i++; i <= n; i++)
    {
        if((0 == hfm_pt[i].parent) && (hfm_pt[i].weight < min))
        {
            min = hfm_pt[i].weight;
            tmp = i;
        }
    }

    *index_p = tmp;
}

static hfm_t *create_hfm_tree(char *dest_p, const char *src_p, int src_len, short *leaf_cnt_p)
{
    int i;
    hfm_t *hfm_pt;
    uint8_t *data_p;
    int num;
    int s1;
    int s2;

    if((NULL == dest_p) || (NULL == src_p) || (0 > src_p))
        return NULL;

    hfm_pt = malloc(2 * NODE_CNT * sizeof(hfm_t));
    if(NULL == hfm_pt)
        return NULL;

    for(i = 0; i < NODE_CNT; i++)
    {
        hfm_pt[i].weight = 0;
        hfm_pt[i].ch = i & 0xFF;
    }

    data_p = (uint8_t *)dest_p;
    for(i = 0; i < src_len; i++)
    {
        hfm_pt[data_p[i]].weight++;
    }

    sort_tree(hfm_pt);
    *leaf_cnt_p = count_leaf(hfm_pt);
    if(1 == *leaf_cnt_p)
    {
        hfm_pt[0].parent = 1;
        return hfm_pt;
    }
    
    num = *leaf_cnt_p*2 - 1;
    for(i = num-1; i >= 0; i--)
    {
        hfm_pt[i].parent    = 0;
        hfm_pt[i].l_child   = 0;
        hfm_pt[i].r_child   = 0;
    }
    
    /* ------------初始化完毕！对应算法步骤1--------- */
    for (i = *leaf_cnt_p; i < num; i++)    //创建非叶子结点,建哈夫曼树
    {
        //在ht[0]~ht[i-1]的范围内选择两个parent为0且weight最小的结点，其序号分别赋值给s1、s2返回
        get_min_tree(hfm_pt, i-1, &s1);
        hfm_pt[s1].parent = i;
        hfm_pt[i].l_child = s1;
         
        get_min_tree(hfm_pt, i-1, &s2);
        hfm_pt[s2].parent = i;
        hfm_pt[i].r_child = s2;

        hfm_pt[i].weight = hfm_pt[s1].weight + hfm_pt[s2].weight;
    }
     
     return hfm_pt;
}

char **CrtHuffmanCode(HTNode * ht, short LeafNum)
/*从叶子结点到根，逆向求每个叶子结点对应的哈夫曼编码*/
{
    char *cd, **hc;    //容器
    int i, start, p, last;

    hc = (char **)malloc((LeafNum) * sizeof(char *));    /*分配n个编码的头指针 */

    if (1 == LeafNum)    //只有一个叶子节点时 
    {
        hc[0] = (char *)malloc((LeafNum + 1) * sizeof(char));
        strcpy(hc[0], "0");
        return hc;
    }

    cd = (char *)malloc((LeafNum + 1) * sizeof(char));    /*分配求当前编码的工作空间 */
    cd[LeafNum] = '\0';    /*从右向左逐位存放编码，首先存放编码结束符 */
    for (i = 0; i < LeafNum; i++) {    /*求n个叶子结点对应的哈夫曼编码 */
        start = LeafNum;    /*初始化编码起始指针 */
        last = i;
        for (p = ht[i].parent; p != 0; p = ht[p].parent) {    /*从叶子到根结点求编码 */
            if (ht[p].LChild == last)
                cd[--start] = '0';    /*左分支标0 */
            else
                cd[--start] = '1';    /*右分支标1 */
                last = p;
        }
        
        hc[i] = (char *)malloc((LeafNum - start) * sizeof(char));    /*为第i个编码分配空间 */
        strcpy(hc[i], &cd[start]);
        //
        printf("%3d号 %3c 码长:%2d;编码:%s\n", ht[i].ch, ht[i].ch,
        LeafNum - start, &cd[start]);
    }    //getchar();
    free(cd);    // Printcode(hc,n);
    
    return hc;
}


int compress_file(const char *f_dest_p, const char *f_src_p)
{
    hfm_t *hfm_pt;
    queue_t queue;
    short leaf_cnt;
    
    if((NULL == f_dest_p) || (NULL == f_src_p))
    {
        errno = EINVAL;
        return -1;
    }

    if(0 != access(f_src_p, F_OK))
        return -1;

    hfm_pt = create_hfm_tree(f_dest_p, f_src_p, 1, &leaf_cnt);
    if(NULL == hfm_pt)
        return 0;
    
    memset(&queue, 0, sizeof(queue));
     hc = CrtHuffmanCode(ht, LeafNum);    //取得哈夫曼0、1编码,hc的长度为LeafNum

    return 0;
}

int main(int argc, char **argv)
{
    compress_file(NULL, NULL);
    
    return 0;
}
