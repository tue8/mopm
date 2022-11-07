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
#include "m_vctrl.h"

char *get_checksum(const char *filename)
{
  FILE *f;
  int i, j, k = 0;
  sha256_context ctx;
  unsigned char buf[1000];
  unsigned char sha256sum[32];
  char *hex_sha256sum = m_malloc(sizeof(char) * 65);

  if ((f = fopen(filename, "rb")) == NULL)
  {
    return NULL;
  }

  sha256_starts(&ctx);

  while ((i = fread(buf, 1, sizeof(buf), f)) > 0)
  {
    sha256_update(&ctx, buf, i);
  }

  sha256_finish(&ctx, sha256sum);

  for (j = 0; j < 32; j++)
  {
    sprintf((char *)(hex_sha256sum + k), "%02x", sha256sum[j]);
    k += 2;
  }

  fclose(f);
  return hex_sha256sum;
}

struct checksum_data
{
  char *bin_dir;
  const char *checksum;
};

static int condition_func(struct vctrl *_vctrl, char *pkg, void *ud)
{
  struct checksum_data *cd = ud;
  char *bin_checksum = get_checksum(cd->bin_dir);
  int result = (bin_checksum != NULL && strcmp(bin_checksum, cd->checksum) == 0);
  m_free(bin_checksum);
  return result;
}

static int condition(struct vctrl *_vctrl, char *line, char *pkg, void *ud)
{
  int result;
  char *pkg_n;
  asprintf(&pkg_n, "%s\n", pkg);
  result = (strcmp(line, pkg) == 0 || strcmp(line, pkg_n) == 0) ? 0 : 1;
  m_free(pkg_n);
  return result;
}

int verify_checksum(struct vctrl *_vctrl, char *bin_dir, char *pkg,
                    const char *checksum)
{
  struct vctrl_pkg_con_data result;
  char line[133];

  struct checksum_data cd;

  cd.bin_dir = bin_dir;
  cd.checksum = checksum;

  vctrl_pkg_con(&result, _vctrl, pkg, &cd, &condition_func, &condition);

  return result.con_func_result;
}