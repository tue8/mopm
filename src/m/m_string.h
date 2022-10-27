#ifndef M_STRING_H_
#define M_STRING_H_

#include <stdarg.h>

#define STRING_MAX_LEN 255

char *get_str_after_char(const char *_str, int _char);
char *get_str_before_char(const char *_str, int _char);
int vasprintf(char **strp, const char *format, va_list ap);
int asprintf(char **strp, const char *format, ...);
char *m_strcat(char *dest, char *src);

#endif