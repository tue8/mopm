/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_DIRECTORY
#define M_DIRECTORY

#include <windows.h>
#include "m_debug.h"

#define create_directory(path)\
ASSERT(CreateDirectory(path, NULL) == 0 && GetLastError() != 183L, "Could not create " #path "\n");\
\

char *get_current_directory();
int remove_directory(char *path);

#endif