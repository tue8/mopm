#ifndef M_CHECKSUM_H_
#define M_CHECKSUM_H_

#include "m_vctrl.h"

char *get_checksum(const char *filename);

int verify_checksum(struct vctrl *_vctrl, char *bin_dir, char *pkg,
                    const char *checksum);

#endif