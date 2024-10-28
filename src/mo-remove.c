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
#include "m/m_directory.h"
#include "m/m_batch.h"
#include "m/m_debug.h"
#include "mopm.h"
#include <stdio.h>

static int vctrl_clone(struct vctrl *_vctrl, char *pkg_name, void *ud)
{
  char *l_pkg_name = get_str_before_char(_vctrl->line, '@');
  char *l_pkg_version = get_str_after_char(_vctrl->line, '@');
  char *pkg_name_n;
  asprintf(&pkg_name_n, "%s\n", pkg_name);

  if (l_pkg_name != NULL && l_pkg_version != NULL &&
      strcmp(l_pkg_name, pkg_name) != 0 &&
      strcmp(l_pkg_name, pkg_name_n) != 0)
    fputs(_vctrl->line, _vctrl->fclone);

  m_free(l_pkg_name);
  m_free(l_pkg_version);
  m_free(pkg_name_n);
  return M_SUCCESS;
}

static void cleanup(struct mo_program *mo, int code)
{
  m_free(mo->pkg);
  m_free(mo->pkg_dir);
  json_decref(mo->fpd.json_root);
  vctrl_cleanup(&mo->_vctrl, code);
  printf((code == M_SUCCESS) ? "Successfully uninstalled package."
                             : "Failed to uninstall package.");
  m_deduce();
  exit(code);
}

int main(int argc, char *argv[])
{
  struct mo_program mo;

  if (m_init(argc, "uninstall") == 1)
    return M_FAIL;

  curl_global_init(CURL_GLOBAL_ALL);
  mo.curl_handle = curl_easy_init();
  if (mo.curl_handle == NULL)
  {
    fprintf(stderr, "Could not initialize curl\n");
    return M_FAIL;
  }

  /*
  * pkg here is technically pkg_name since
  * we cant specify a version to uninstall
  * with mo-remove
  */
  mo.pkg = m_strdup(argv[1]);
  if (m_init_uninstall(&mo) == 1)
  {
    curl_easy_cleanup(mo.curl_handle);
    curl_global_cleanup();
    return M_FAIL;
  }

  asprintf(&mo.pkg_dir, "%s\\mopm\\%s", getenv("APPDATA"), mo.pkg);

  /***/

  asprintf(&mo.batch_dir, "%s\\..\\%s.bat", mo.pkg_dir, mo.pkg);

  int remove_success = remove(mo.batch_dir);
  m_free(mo.batch_dir);
  M_ASSERT(remove_success != 0, "Could not remove package's batch file\n");

  if (remove_directory(mo.pkg_dir) == 1 || file_size(mo._vctrl.file) == 0)
    cleanup(&mo, M_FAIL);

  vctrl_pkg_con(&mo._vctrl, mo.pkg, NULL, &vctrl_clone);
  cleanup(&mo, M_SUCCESS);
}