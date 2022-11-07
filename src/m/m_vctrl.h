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
  int init;
  FILE *file, *file2;
  char *dir, *dir2;
  char line[133];
};

/* long ass struct name */
struct vctrl_pkg_con_data
{
  int write_func_result, con_func_result;
};

typedef int (*vctrl_func)(struct vctrl *, char *, void *);
typedef int (*vctrl_con)(struct vctrl *, char *, char *, void *);

int vctrl_init(struct vctrl *_vctrl);
int vctrl_cleanup(struct vctrl *_vctrl, int success);
int vctrl_pkg_con(struct vctrl_pkg_con_data *result, struct vctrl *_vctrl,
                    char *pkg, void *ud,
                    vctrl_func con_func, vctrl_con con);
int file_size(FILE *file);

#endif