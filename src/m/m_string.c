/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_debug.h"

char *_get_str_after_char(const char *_str, int _char)
{
  char *str;
  char *last_char;

  str = m_strdup(_str);
  last_char = strrchr(str, _char);
  if (last_char == NULL)
  {
    m_free(str);
    return NULL;
  }
  last_char += 1;
  str += (int)(last_char - str);
  return str;
}

char *_get_str_before_char(const char *_str, int _char)
{
  char *str;
  char *last_char;

  str = m_strdup(_str);
  last_char = strrchr(str, _char);
  if (last_char == NULL)
  {
    m_free(str);
    return NULL;
  }
  *(str + (strlen(str) - strlen(last_char))) = '\0';
  return str;
}

static int vasprintf(char **strp, const char *format, va_list ap)
{
  int len;
  int retval;
  char *str;

  len = _vscprintf(format, ap);
  if (len == -1)
    return -1;
  str = (char *)m_malloc(len + 1);
  if (str == NULL)
    return -1;
  retval = vsnprintf(str, len + 1, format, ap);
  if (retval == -1)
  {
    m_free(str);
    return -1;
  }
  *strp = str;
  return retval;
}

int _asprintf(char **strp, const char *format, ...)
{
  va_list ap;
  int retval;

  va_start(ap, format);
  retval = vasprintf(strp, format, ap);
  va_end(ap);
  return retval;
}

char *d_get_str_after_char(const char *_str, int _char, const char *file, const int line, const char *func)
{
  char *str;

  str = _get_str_after_char(_str, _char);
  return str;
}

char *d_get_str_before_char(const char *_str, int _char, const char *file, const int line, const char *func)
{
  char *str;

  str = _get_str_before_char(_str, _char);
  return str;
}

int d_asprintf(const char *file, const int line, const char *func, char **strp, const char *format, ...)
{
  va_list args;
  int retval;

  va_start(args, format);
  retval = vasprintf(strp, format, args);
  va_end(args);
  return retval;
}

char *m_strcat(char *dest, char *src)
{
  while (*dest)
    dest++;
  while (*dest++ = *src++)
    ;
  return --dest;
}