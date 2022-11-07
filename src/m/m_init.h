/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_INIT_H_
#define M_INIT_H

#include <curl/curl.h>
struct vctrl;

int m_init(int argc, char *cmd);
int m_init_install(struct vctrl *_vctrl, char *pkg, char **pkg_name, char **pkg_version, int argc);

#endif