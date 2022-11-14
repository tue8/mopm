/*
 * Created on Sun Nov 13 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_validate_package.h"
#include "m_vctrl.h"
#include "m_find_package.h"
#include "m_checksum.h"
#include "m_string.h"
#include "m_debug.h"
#include <io.h>

int validate_package(struct vctrl *_vctrl, struct find_package_data *fpd,
                     const char *pkg_dir, char *pkg)
{
  char *entry_dir;
  char *pkg_full;
  char *pkg_version;
  int result;

  result = 0;
  asprintf(&entry_dir, "%s\\%s", pkg_dir, fpd->entry);
  asprintf(&pkg_full, "%s@%s", pkg,
          ((pkg_version = get_str_before_char(pkg, '@')) == NULL) ?
          fpd->version : pkg_version);
  if (pkg_version != NULL)
    m_free(pkg_version);
  if (_access(entry_dir, 0) == -1)
    goto out;
  if (verify_checksum(_vctrl, entry_dir, pkg_full, fpd->checksum) == 1)
  {
    fprintf(stderr, "Package is already installed\n");
    result = 1;
    goto out;
  }
out:
  m_free(pkg_full);
  m_free(entry_dir);
  return result;
}