/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_checksum.h"
#include "m_string.h"
#include <stdio.h>
#include <sha256.h>
#include "m_debug.h"
#include <string.h>
#include "../mopm.h"

char *get_checksum(const char *filename)
{
  FILE *f;
  int i, j, k = 0;
  sha256_context ctx;
  unsigned char buf[1000];
  unsigned char sha256sum[32];
  char *hex_sha256sum;

  hex_sha256sum = m_malloc(sizeof(char) * 65);
  if ((f = fopen(filename, "rb")) == NULL)
    return NULL;
  sha256_starts(&ctx);
  while ((i = fread(buf, 1, sizeof(buf), f)) > 0)
    sha256_update(&ctx, buf, i);
  sha256_finish(&ctx, sha256sum);
  for (j = 0; j < 32; j++)
  {
    sprintf((char *)(hex_sha256sum + k), "%02x", sha256sum[j]);
    k += 2;
  }
  fclose(f);
  return hex_sha256sum;
}

int verify_checksum(struct vctrl *_vctrl, char *bin_dir, char *pkg,
                    const char *checksum)
{
  int result = M_FAIL;
  if (ftell(_vctrl->file) != 0L)
    rewind(_vctrl->file);
  while (fgets(_vctrl->line, sizeof(_vctrl->line), _vctrl->file))
  {
    char* pkg_n;
    asprintf(&pkg_n, "%s\n", pkg);
    if (strcmp(_vctrl->line, pkg) == 0 || strcmp(_vctrl->line, pkg_n) == 0)
    {
      char* bin_checksum = get_checksum(bin_dir);
      result = (bin_checksum != NULL &&
                strcmp(bin_checksum, checksum) == 0) ? M_SUCCESS : M_FAIL;
      m_free(bin_checksum);
      break;
    }
    m_free(pkg_n);
  }
  return result;
}