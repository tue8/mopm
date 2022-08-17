#ifndef M_STRING_H_
#define M_STRING_H_

#include <string.h>

#define STRING_MAX_LEN 255

char *get_str_after_char(const char *_str, int _char)
{
    char *str = strdup(_str);
    char *last_char;

    strcpy(str, _str);

    last_char = strrchr(str, _char);
    if (last_char == NULL)
    {
        free(str);
        return NULL;
    }
    last_char += 1;
    str += (int)(last_char - str);

    return str;
}

char *get_str_before_char(const char *_str, int _char)
{
    char *str = strdup(_str);
    char *last_char;

    strcpy(str, _str);

    last_char = strrchr(str, _char);
    if (last_char == NULL)
    {
        free(str);
        return NULL;
    }
    *(str + (strlen(str) - strlen(last_char))) = '\0';

    return str;
}

#endif