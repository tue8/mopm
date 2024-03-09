/*
 * Created on Sun Nov 13 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_validate_package.h"
#include "m_checksum.h"
#include "m_string.h"
#include "m_debug.h"
#include "../mopm.h"
#include <io.h>

int m_validate_package(struct mo_program *mo)
{
  char *entry_dir;

  asprintf(&entry_dir, "%s\\%s", mo->pkg_dir, mo->fpd.entry);
  if (_access(entry_dir, 0) == -1)
  {
    m_free(entry_dir);
    return M_SUCCESS;
  }

  if (verify_checksum(&(mo->_vctrl), entry_dir, mo->pkg, mo->fpd.checksum) == M_SUCCESS)
  {
    fprintf(stderr, "Package is already installed\n");
    m_free(entry_dir);
    return M_FAIL;
  }

  m_free(entry_dir);
  return M_SUCCESS;
}