/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_debug.h"

#include <stdio.h>
#include <string.h>

void *_m_malloc(size_t size, const char *file, const int line, const char *func)
{
  void *p = malloc(size);
  printf("Allocated = %s, %i, %s, %p\n", file, line, func, p);
  return p;
}

void _m_free(void *block, const char *file, const int line, const char *func)
{
  free(block);
  printf("Freed = %s, %i, %s, %p\n", file, line, func, block);
}

char *_m_strdup(char const *str, const char *file, const int line, const char *func)
{
  void *p = strdup(str);
  printf("Allocated = %s, %i, %s, %p\n", file, line, func, p);
  return p;
}