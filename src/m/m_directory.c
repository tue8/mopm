/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_directory.h"
#include <malloc.h>

char *get_current_directory()
{
  TCHAR *path = m_malloc(MAX_PATH);
  if (GetCurrentDirectoryA(MAX_PATH, path) == (DWORD)0)
    return 0;
  return path;
}