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

#define V_TERMINATE 3

struct vctrl
{
  FILE *file, *fclone;
  char *dir, *dirclone;
  char line[133];
  int pkg_con;
};

typedef int (*vctrl_func)(struct vctrl *, char *, void *);

int vctrl_init(struct vctrl *_vctrl);
int vctrl_cleanup(struct vctrl *_vctrl, int success);
int vctrl_pkg_con(struct vctrl *_vctrl,
                  char *pkg, void *ud,
                  vctrl_func func);
int file_size(FILE *file);

#endif