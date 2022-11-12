/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_directory.h"
#include <malloc.h>
#include <stdio.h>

char *get_current_directory()
{
  TCHAR *path = m_malloc(MAX_PATH);
  if (GetCurrentDirectoryA(MAX_PATH, path) == (DWORD)0)
    return 0;
  return path;
}

int remove_directory(char *path)
{
  WIN32_FIND_DATA fd_file;
  HANDLE hfind;
  char s_path[2048];

  hfind = NULL;
  sprintf(s_path, "%s\\*.*", path);
  if ((hfind = FindFirstFile(s_path, &fd_file)) == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "Path not found: %s\n", path);
    return 1;
  }
  do
  {
    if (strcmp(fd_file.cFileName, ".") != 0 && strcmp(fd_file.cFileName, "..") != 0)
    {
      sprintf(s_path, "%s\\%s", path, fd_file.cFileName);
      if (fd_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (remove_directory(s_path) == 1)
          return 1;
      }
      else
        DeleteFile(s_path);
    }
  } while (FindNextFile(hfind, &fd_file));
  FindClose(hfind);
  RemoveDirectory(path);
  return 0;
}