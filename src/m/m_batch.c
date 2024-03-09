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
#include "../mopm.h"

const char *batch_template = "@ECHO OFF\n"
                             "IF EXIST %s (\n"
                             "  %s %%*\n"
                             ") ELSE (\n"
                             "  echo Could not find package's entry: %s\n"
                             ")";

int m_create_batch(struct mo_program *mo)
{
  char *entry_dir;
  char *batch_dir;
  FILE *batch_f;

  asprintf(&entry_dir, "%s\\%s", mo->pkg_dir, mo->fpd.entry);
  asprintf(&batch_dir, "%s\\..\\%s.bat", mo->pkg_dir, mo->pkg_name);
  batch_f = fopen(batch_dir, "w");
  if (batch_f == NULL)
  {
    m_free(entry_dir);
    m_free(batch_dir);
    return M_FAIL;
  }

  fprintf(batch_f, batch_template, entry_dir, entry_dir, entry_dir);
  fclose(batch_f);

  m_free(entry_dir);
  m_free(batch_dir);
  return M_SUCCESS;
}