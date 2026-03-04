// 定长版本的内存池
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MEM_PAGE_SIZE 4096

typedef struct mempool_s {
    int blocksize;
    int freecount;
    char *free_ptr;
    char *mem;
} mempool_t;

int memp_create(mempool_t *m, int block_size) {
    if (!m) return -1;
    m->blocksize = block_size;
    m->freecount = MEM_PAGE_SIZE / block_size;

    m->mem = (char *)malloc(MEM_PAGE_SIZE);
    if (!m->mem) return -2;
    memset(m->mem, 0, MEM_PAGE_SIZE);       // 将m->mem起始的MEM_PAGE_SIZE个字节全部清零；

    m->free_ptr = m->mem;

    int i = 0;
    char *ptr = m->mem;
    for (i = 0; i < m->freecount; i++) {
        *(char **)ptr = ptr + block_size;   // (char**)ptr的含义是：当前块的起始位置，是一个「可以存储指针地址」的指针变量地址
        ptr = ptr + block_size;
    }
    *(char **)ptr = NULL;                     // 最后一个块的指针指向NULL，表示已经用完

    return 0;
}

void memp_destroy(mempool_t *m) {
    if (!m) return;
    free(m->mem);
}

void *memp_alloc(mempool_t *m) {
    if (!m || m->freecount == 0) return NULL;
    void *ptr = m->free_ptr;

    m->free_ptr = *(char **)ptr;
    m->freecount--;

    return ptr;
}

void memp_free(mempool_t *m, void *ptr) {
    *(char **)ptr = m->free_ptr;
    m->free_ptr = (char *)ptr;
    m->freecount++;

}

int main() {
    mempool_t m;
    memp_create(&m, 32);
    void *p1 = memp_alloc(&m);
    printf("memp_alloct: %p\n", p1);
    void *p2 = memp_alloc(&m);
    printf("memp_alloct: %p\n", p2);
    void *p3 = memp_alloc(&m);
    printf("memp_alloct: %p\n", p3);

    memp_free(&m, p2);
}