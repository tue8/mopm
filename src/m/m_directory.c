#include "m_directory.h"
#include <malloc.h>

char *get_current_directory()
{
  TCHAR *path = malloc(MAX_PATH);
  if (GetCurrentDirectoryA(MAX_PATH, path) == (DWORD)0)
    return 0;
  return path;
}