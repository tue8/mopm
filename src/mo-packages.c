/*
 * Created on Sun Nov 23 2024
 * mopm Package Manger
 * https://github.com/tue8/mopm
 * Licensed under MIT license
 * Copyright (c) 2024 tue8
 */

#include "mopm.h"
#include <stdio.h>

static void cleanup(struct mo_program* mo, int code)
{
  /*
   * vctrl_cleanup with M_SUCCESS means that
   * .vctrl will be replaced with .vctrl.clone
   * which is not what we want
   */
  vctrl_cleanup(&mo->_vctrl, M_FAIL);
  exit(code);
}

int main(int argc, char* argv[])
{
  struct mo_program mo;
  struct vctrl *_vctrl = &mo._vctrl;
  
  if (vctrl_init(_vctrl) == 1)
    vctrl_cleanup(_vctrl, M_FAIL);

  if (ftell(_vctrl->file) != 0L)
    rewind(_vctrl->file);

  while (fgets(_vctrl->line, sizeof(_vctrl->line), _vctrl->file))
  {
    printf(_vctrl->line);
  }

  cleanup(&mo, M_SUCCESS);
}