#include "m_debug.h"

#include <stdio.h>

void *_m_malloc(size_t size, const char *file, const int line, const char *func)
{
    void *p = malloc(size);
    printf("Allocated = %s, %i, %s, %p[%li]\n", file, line, func, p, size);
    return p;
}

void _m_free(void* block, const char *file, const int line, const char *func)
{
    free(block);
    printf("Free'd = %s, %i, %s, %p\n", file, line, func, block);
}