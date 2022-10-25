#include "m_check.h"
#include "m_checksum.h"
#include "m_find_package.h"
#include "m_vctrl.h"
#include "m_string.h"

int check_name_len(char *name)
{
  return (((strlen(name) <= 100) && (strlen(name) > 2)) == 0);
}

int check_version_len(char *version)
{
  return (((strlen(version) <= 32) && (strlen(version) > 0)) == 0);
}

int check_package_install(struct find_package_data *fpd, CURL *curl_handle,
                          struct vctrl *_vctrl, char *pkg,
                          char *pkg_name, char *pkg_version,
                          char *bin_dir, char **bin_url)
{
  if (check_fpd(fpd) == 1)
    return 1;
  else
  {
    printf("Found: %s Version %s\n"
           "Description: %s\n"
           "License: %s\n"
           "Author: %s\n",
           pkg_name, pkg_version,
           fpd->des,
           fpd->license,
           fpd->author);

    if (verify_checksum(_vctrl, bin_dir, pkg, fpd->checksum) == 1)
    {
      fprintf(stderr, "Package is already installed\n");
      return 1;
    }

    if (get_binary_url(curl_handle, fpd->rls_url, pkg_version, bin_url) == 1)
      return 1;
  }

  return 0;
}