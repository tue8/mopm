/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_CHECKSUM_H_
#define M_CHECKSUM_H_

struct vctrl;

char *get_checksum(const char *filename);
int verify_checksum(struct vctrl *_vctrl, char *bin_dir, char *pkg,
                    const char *checksum);

#endif