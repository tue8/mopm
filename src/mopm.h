#ifndef MOPM_H_
#define MOPM_H_

#include "m/m_vctrl.h"
#include "m/m_find_package.h"

#define M_SUCCESS 0
#define M_FAIL 1

struct mo_program
{
  CURL *curl_handle;
  char *pkg_name;
  char *pkg_version;
  struct vctrl _vctrl;
  char *pkg_dir;
  char *bin_dir;
  char *batch_dir;
  char *pkg;
  struct find_package_data fpd;
};

#endif