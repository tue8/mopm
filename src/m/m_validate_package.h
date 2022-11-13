/*
 * Created on Sun Nov 13 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_VALIDATE_PACKAGE_H_
#define M_VALIDATE_PACKAGE_H_

struct vctrl;
struct find_package_data;

int validate_package(struct vctrl *_vctrl, struct find_package_data *fpd,
                     const char *pkg_dir, char *pkg);

#endif