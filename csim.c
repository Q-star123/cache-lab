#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

typedef struct {
    int valid;
    unsigned long tag;
    int lru_counter;
} CacheLine;

typedef struct {
    CacheLine **sets;
    int s;            // 组索引位数
    int S;            // 组数 (2^s)
    int E;            // 每组行数
    int b;            // 块偏移位数
} Cache;

int hits = 0, misses = 0, evictions = 0;
int global_time = 0;

void accessCache(Cache *cache, unsigned long address) {
    // 计算组索引和标记 (Tag)
    unsigned long set_index = (address >> cache->b) & (cache->S - 1);
    unsigned long tag = address >> (cache->b + cache->s);

    CacheLine *set = cache->sets[set_index];
    
    // 1. 查找是否命中
    for (int i = 0; i < cache->E; i++) {
        if (set[i].valid && set[i].tag == tag) {
            hits++;
            set[i].lru_counter = ++global_time;
            return;
        }
    }

    // 2. 没命中（Miss）
    misses++;
    int empty_index = -1;
    int lru_index = 0;
    int min_lru = 0x7fffffff;

    for (int i = 0; i < cache->E; i++) {
        if (!set[i].valid) {
            empty_index = i;
            break;
        }
        if (set[i].lru_counter < min_lru) {
            min_lru = set[i].lru_counter;
            lru_index = i;
        }
    }

    if (empty_index != -1) {
        // 有空位
        set[empty_index].valid = 1;
        set[empty_index].tag = tag;
        set[empty_index].lru_counter = ++global_time;
    } else {
        // 满了，发生 Eviction (驱逐)
        evictions++;
        set[lru_index].tag = tag;
        set[lru_index].lru_counter = ++global_time;
    }
}

int main(int argc, char *argv[]) {
    int s = 0, E = 1, b = 0;
    char trace_file[100] = {0};
    int opt;

    // 解析命令行参数
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': strcpy(trace_file, optarg); break;
            default: 
                printf("Usage: %s -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
                exit(1);
        }
    }

    // 初始化缓存结构
    Cache cache;
    cache.s = s;
    cache.S = 1 << s;
    cache.E = E;
    cache.b = b;
    
    cache.sets = malloc(cache.S * sizeof(CacheLine *));
    for (int i = 0; i < cache.S; i++) {
        cache.sets[i] = calloc(cache.E, sizeof(CacheLine)); // calloc 默认初始化为0
    }

    // 打开 trace 文件
    FILE *fp = fopen(trace_file, "r");
    if (!fp) {
        perror("Error opening trace file");
        return 1;
    }

    char op;
    unsigned long addr;
    int size;
    
    // 逐行读取 trace 文件内容
    while (fscanf(fp, " %c %lx,%d", &op, &addr, &size) == 3) {
        if (op == 'I') continue; // 题目要求忽略指令加载
        if (op == 'L' || op == 'S' || op == 'M') {
            accessCache(&cache, addr);
            if (op == 'M') {
                // M 代表 Modify，相当于读了又写，所以要访问两次
                accessCache(&cache, addr);
            }
        }
    }

    fclose(fp);
    
    // 释放内存
    for (int i = 0; i < cache.S; i++) {
        free(cache.sets[i]);
    }
    free(cache.sets);

    printSummary(hits, misses, evictions);
    return 0;
}