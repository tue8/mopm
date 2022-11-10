/*
 * Created on Thu Nov 10 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_batch.h"
#include <stdio.h>
#include "m_string.h"
#include <string.h>
#include "m_debug.h"

const char *batch_template = "@ECHO OFF\n"
                             "IF EXIST %s (\n"
                             "  %s %%*\n"
                             ") ELSE (\n"
                             "  echo Could not find package's entry: %s\n"
                             ")";

char *exe_to_bat(const char *entry)
{
  char *str;
  char *ext;

  str = m_strdup(entry);
  ext = get_str_after_char(str, '.');
  if (ext == NULL)
  {
    m_free(str);
    return NULL;
  }
  if (strcmp(ext, "exe") == 0)
  {
    str[strlen(str) - strlen(ext)] = '\0';
    strcat(str, "bat");
  }
  else
    m_free(str);
  m_free(ext);
  return str;
}

int create_batch(const char *pkg_name, const char *pkg_dir, const char *entry)
{
  int result;
  char *entry_dir;
  char *batch_dir;
  FILE *batch_f;
  char *entry_exe;

  entry_exe = exe_to_bat(entry);
  if(entry_exe == NULL)
    return 1;
  result = 1;
  asprintf(&entry_dir, "%s\\%s", pkg_dir, entry);
  asprintf(&batch_dir, "%s\\..\\%s", pkg_dir, entry_exe);
  batch_f = fopen(batch_dir, "w");
  if (batch_f == NULL)
    goto out;
  fprintf(batch_f, batch_template, entry_dir, entry_dir, entry_dir);
  fclose(batch_f);
  result = 0;
out:
  m_free(entry_dir);
  m_free(batch_dir);
  return result;
}