/* 
Github: ChocolateSilk
参考过的博文：
https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm
https://www.cnblogs.com/tea-in-the-snow/p/16986210.html
https://lifeislife.cn/posts/c%E8%AF%AD%E8%A8%80getopt-%E5%87%BD%E6%95%B0%E7%9A%84%E7%94%A8%E6%B3%95/
还参考了gpt给的一些想法。
但是，2的驱逐数量不对、测试5和6过不了，我看不出是哪里不对（应该是read_data函数的问题）...，只有20/27。
*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
typedef struct {
    int valid;
    unsigned int tag;
    int log; /* 用来计算最少访问的缓存，以便冲突时替换 */
} Line;

typedef struct {
    Line *lines;
} Set;

typedef struct {
    Set *sets;
    int s, E, b;
} Cache;

Cache cache;
int hit = 0,misses = 0,eviction = 0;
void init_cache(int s, int E, int b) {
    cache.s = s;
    cache.E = E;
    cache.b = b;
    int set_num = 1 << s;
    cache.sets = malloc(set_num * sizeof(Set));
    for (int i = 0; i < set_num; i++) {
        cache.sets[i].lines = malloc(E * sizeof(Line));
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].log = 0;
        }
    }
}
void free_cache() {
    int set_num = 1 << cache.s;
    for (int i = 0; i < set_num; i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
}

void read_data(unsigned int addr) {
    unsigned int tag = addr >> (cache.s + cache.b);
    unsigned int set_index = (addr >> cache.b) & ((1 << cache.s) - 1);
    Set* set = &cache.sets[set_index]; 

    int victim = -1;
    int min_access = __INT_MAX__;
    for (int i = 0; i < cache.E; i++) {
        Line *line = &set->lines[i];
        if (line->valid == 1 && line->tag == tag) {
            hit++;
            line->log++;
            return;
        }
        if (line->valid == 1) {
            if (min_access > line->log) {
                min_access = line->log;
                victim = i;
            }
        }
        else {
            victim = i;
        }
    }
    misses++;
    Line *to_modify = &set->lines[victim];
    if (to_modify->valid) eviction++;

    to_modify->valid = 1;
    to_modify->tag = tag;
    to_modify->log = 0;

}
void read_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("No such file.");
        exit(1);
    }
    char op;
    unsigned int addr;
    int size;
    char buffer[1000];
    while (fgets(buffer, 1000, fp)) {
        if (sscanf(buffer, " %c %x,%d", &op, &addr, &size) == 3) {
            switch (op) {
                case 'L': read_data(addr); break;
                case 'S': read_data(addr); break;
                case 'M': read_data(addr); read_data(addr); break;
                default: break;
            }
        }
    }
    fclose(fp);
}
int main(int argc, char *argv[]){

    int s = 0,E = 0, b = 0;
    char *file;
    int c;
    while ((c = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (c) {
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': file = optarg; break;
        }
    }
    init_cache(s, E, b);
    read_file(file);
    printSummary(hit, misses, eviction);
    free_cache();
    return 0;
}
