/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_VCTRL_H_
#define M_VCTRL_H_

#include <stdio.h>

struct vctrl
{
  FILE *file, *fclone;
  char *dir, *dirclone;
  char line[133];
};

typedef int (*vctrl_func)(struct vctrl *, char *);

int vctrl_init(struct vctrl *_vctrl);
int vctrl_cleanup(struct vctrl *_vctrl, int success);
int vctrl_loop_over(struct vctrl *_vctrl, char *pkg, vctrl_func func);
int file_size(FILE *file);

#endif