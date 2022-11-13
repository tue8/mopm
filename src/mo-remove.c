/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m/m_init.h"
#include "m/m_string.h"
#include "m/m_curl.h"
#include "m/m_vctrl.h"
#include "m/m_find_package.h"
#include "m/m_directory.h"
#include "m/m_batch.h"
#include "m/m_debug.h"
#include <stdio.h>

static int vctrl_condition_func(struct vctrl *_vctrl, char *pkg, void *ud)
{
  fputs(_vctrl->line, _vctrl->file2);
  return 0;
}

static int vctrl_condition(struct vctrl *_vctrl, char *line, char *pkg, void *ud)
{
  char *l_pkg_name;
  char *l_pkg_version;
  int result = ((l_pkg_name = get_str_before_char(line, '@')) == NULL ||
                (l_pkg_version = get_str_after_char(line, '@')) == NULL) ?
                1 : (strcmp(l_pkg_name, pkg) == 0);
  m_free(l_pkg_name);
  m_free(l_pkg_version);
  return result;
}

static int vctrl_remove_pkg(struct vctrl *_vctrl, char *pkg)
{
  struct vctrl_pkg_con_data result;

  vctrl_pkg_con(&result, _vctrl, pkg, NULL,
                &vctrl_condition_func, &vctrl_condition);
  return 0;
}

int main(int argc, char *argv[])
{
  CURL *curl_handle;
  struct vctrl _vctrl;
  char *pkg_dir;
  char *batch_dir;
  char *pkg;
  int success;
  int remove_success;
  struct find_package_data fpd;

  if (m_init(argc, "uninstall") == 1)
    return 1;
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
  if (curl_handle == NULL)
  {
    fprintf(stderr, "Could not initialize curl\n");
    return 1;
  }
  pkg = m_strdup(argv[1]);
  if (m_init_uninstall(&_vctrl, pkg) == 1)
  {
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 1;
  }
  asprintf(&pkg_dir, "%s\\mopm\\%s", getenv("APPDATA"), pkg);
  find_package(&fpd, curl_handle, pkg, pkg, NULL);
  if (check_fpd(&fpd) == 1)
    goto out;
  success = 0;
  asprintf(&batch_dir, "%s\\..\\%s.bat", pkg_dir, pkg);
  remove_success = remove(batch_dir);
  m_free(batch_dir);
  if (remove_success != 0)
  {
    perror("Could not remove package's batch file");
    goto out;
  }
  if (remove_directory(pkg_dir) == 1 || file_size(_vctrl.file) == 0)
    goto out;
  vctrl_remove_pkg(&_vctrl, pkg);
  success = 1;
out:
  free(pkg);
  free(pkg_dir);
  free_fpd(&fpd);
  vctrl_cleanup(&_vctrl, success);
  if (success == 1)
    printf("Successfully uninstalled package.");
  else
    printf("Failed to uninstall package.");
  return success == 0;
}