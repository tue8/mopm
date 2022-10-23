#ifndef M_VCTRL_H_
#define M_VCTRL_H_

#include <stdio.h>
#include <windows.h>

struct vctrl
{
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

int vctrl_init(struct vctrl *_vctrl, int install);

int vctrl_cleanup(struct vctrl *_vctrl, int success);

int vctrl_pkg_con(struct vctrl_pkg_con_data *result, struct vctrl *_vctrl,
                    char *pkg, void *ud,
                    vctrl_func con_func, vctrl_con con);

int file_size(FILE *file);

#endif