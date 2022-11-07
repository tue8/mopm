/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_vctrl.h"
#include "m_string.h"
#include "m_debug.h"
#include <stdio.h>
#include <string.h>

int file_size(FILE *file)
{
  int file_size;
  fseek(file, 0L, SEEK_END);
  file_size = ftell(file);
  rewind(file);
  return file_size;
}

int vctrl_init(struct vctrl *_vctrl)
{
  int result;
  char *dir2;

  result = 1;
  _vctrl->init = 0;
  asprintf(&_vctrl->dir, "%s\\mopm\\.vctrl", getenv("APPDATA"));
  dir2 = m_strdup(_vctrl->dir);
  dir2[strlen(_vctrl->dir) - strlen("\\.vctrl")] = '\0';
  asprintf(&_vctrl->dir2, "%s\\%s", dir2, ".vctrl.clone");
  _vctrl->file = fopen(_vctrl->dir, "r");
  if (_vctrl->file == NULL)
  {
    perror("Could not open .vctrl");
    goto out;
  }
  _vctrl->file2 = fopen(_vctrl->dir2, "w+");
  if (_vctrl->file2 == NULL)
  {
    perror("Could not create .vctrl.clone");
    goto out;
  }
  result = 0;
out:
  if (result == 0)
    _vctrl->init = 1;
  m_free(dir2);
  return result;
}

int vctrl_cleanup(struct vctrl *_vctrl, int success)
{
  if (_vctrl->init == 1)
  {
    fclose(_vctrl->file);
    fclose(_vctrl->file2);
    if (success == 0)
    {
      if (remove(_vctrl->dir2) != 0)
      {
        perror("Could not remove .vctrl.clone");
      }

      return 1;
    }
    if (remove(_vctrl->dir) != 0)
    {
      perror("Could not remove .vctrl");
    }
    if (rename(_vctrl->dir2, _vctrl->dir) != 0)
    {
      perror("Could not rename .vctrl.clone");
    }
  }
  m_free(_vctrl->dir);
  m_free(_vctrl->dir2);
  return 0;
}

static int init_pkg_con_data(struct vctrl_pkg_con_data *result)
{
  result->con_func_result = 0;
  result->write_func_result = 0;
  return 0;
}

int vctrl_pkg_con(struct vctrl_pkg_con_data *result, struct vctrl *_vctrl,
                  char *pkg, void *ud,
                  vctrl_func con_func, vctrl_con con)
{
  init_pkg_con_data(result);
  while (fgets(_vctrl->line, sizeof(_vctrl->line), _vctrl->file))
  {
    if (con(_vctrl, _vctrl->line, pkg, ud) == 0)
    {
      if (con_func != NULL &&
          (result->con_func_result = con_func(_vctrl, pkg, ud)) == 1)
        break;
    }
  }
  return 0;
}