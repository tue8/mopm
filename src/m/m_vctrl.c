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
#include "../mopm.h"
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
  char *dirclone;

  asprintf(&_vctrl->dir, "%s\\mopm\\.vctrl", getenv("APPDATA"));

  dirclone = m_strdup(_vctrl->dir);
  dirclone[strlen(_vctrl->dir) - strlen("\\.vctrl")] = '\0';

  asprintf(&_vctrl->dirclone, "%s\\%s", dirclone, ".vctrl.clone");

  if ((_vctrl->file = fopen(_vctrl->dir, "r")) == NULL)
  {
    if ((_vctrl->file = fopen(_vctrl->dir, "w")) == NULL)
    {
      perror("Could not open .vctrl");
      result = M_FAIL;
      goto out;
    }
  }

  _vctrl->fclone = fopen(_vctrl->dirclone, "w+");
  if (_vctrl->fclone == NULL)
  {
    perror("Could not create .vctrl.clone");
    result = M_FAIL;
    goto out;
  }

  result = M_SUCCESS;
out:
  m_free(dirclone);
  return result;
}

int vctrl_cleanup(struct vctrl *_vctrl, int code)
{
  fclose(_vctrl->file);
  fclose(_vctrl->fclone);
  if (code == M_FAIL)
  {
    if (remove(_vctrl->dirclone) != 0)
      perror("Could not remove .vctrl.clone");
  }
  else if (code == M_SUCCESS)
  {
    if (remove(_vctrl->dir) != 0)
      perror("Could not remove .vctrl");
    if (rename(_vctrl->dirclone, _vctrl->dir) != 0)
      perror("Could not rename .vctrl.clone");
  }

  m_free(_vctrl->dir);
  m_free(_vctrl->dirclone);
  return 0;
}

int vctrl_pkg_con(struct vctrl *_vctrl,
                  char *pkg, void *ud,
                  vctrl_func func)
{
  int result = M_FAIL;
  _vctrl->pkg_con = M_SUCCESS;
  while (fgets(_vctrl->line, sizeof(_vctrl->line), _vctrl->file) &&
         _vctrl->pkg_con == M_SUCCESS)
    result = func(_vctrl, pkg, ud);
  return result;
}