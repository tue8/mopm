/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_STRING_H_
#define M_STRING_H_

#include <stdarg.h>

#define STRING_MAX_LEN 255

#ifdef _DEBUG
#define get_str_after_char(_str, _char) d_get_str_after_char(_str, _char, __FILE__, __LINE__, __FUNCTION__)
#define get_str_before_char(_str, _char) d_get_str_before_char(_str, _char, __FILE__, __LINE__, __FUNCTION__)
#define asprintf(strp, format, ...) d_asprintf(__FILE__, __LINE__, __FUNCTION__, strp, format, __VA_ARGS__)
#else
#define get_str_after_char(_str, _char) _get_str_after_char(_str, _char)
#define get_str_before_char(_str, _char) _get_str_before_char(_str, _char)
#define asprintf(strp, format, ...) _asprintf(strp, format, __VA_ARGS__)
#endif

char *d_get_str_after_char(const char *_str, int _char, const char *file, const int line, const char *func);
char *d_get_str_before_char(const char *_str, int _char, const char *file, const int line, const char *func);
int d_asprintf(const char *file, const int line, const char *func, char **strp, const char *format, ...);
char *_get_str_after_char(const char *_str, int _char);
char *_get_str_before_char(const char *_str, int _char);
int _asprintf(char **strp, const char *format, ...);
char *m_strcat(char *dest, char *src);

#endif