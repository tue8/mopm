/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m/m_init.h"
#include "m/m_string.h"
#include "m/m_directory.h"
#include "m/m_validate_package.h"
#include "m/m_extract.h"
#include "m/m_batch.h"
#include "m/m_debug.h"
#include "m/m_curl.h"
#include "mopm.h"
#include <stdio.h>

static int print_package_info(struct mo_program *mo)
{
  printf("Found: %s Version %s\n"
         "Description: %s\n"
         "License: %s\n"
         "Author: %s\n",
         mo->pkg_name, mo->pkg_version, mo->fpd.des,
         mo->fpd.license, mo->fpd.author);
}


static int vctrl_clone(struct vctrl *_vctrl, char *pkg)
{
/*
* if all pkg vctrls are valid        -> pkg_valid -> true
* if even one pkg vctrl is corrupted -> pkg_valid -> false
*/
  int pkg_valid = M_FAIL;
  char *l_pkg_name = get_str_before_char(_vctrl->line, '@');
  char *l_pkg_version  = get_str_after_char(_vctrl->line, '@');
  char *pkg_name = get_str_before_char(pkg, '@');
  char *pkg_name_n;

  asprintf(&pkg_name_n, "%s\n", pkg);
  if (l_pkg_name != NULL && l_pkg_version != NULL)
  {
    pkg_valid = M_SUCCESS;
    if (strcmp(l_pkg_name, pkg_name) != 0 &&
        strcmp(l_pkg_name, pkg_name_n) != 0)
      fputs(_vctrl->line, _vctrl->fclone);
  }

  m_free(l_pkg_name);
  m_free(l_pkg_version);
  m_free(pkg_name);
  return pkg_valid;
}

/*
* loop over vctrl
* if pkg's legit in vctrl, copy to vctrl clone 
* end loop
* add new pkg to vctrl clone
*/
static int m_vctrl_add_pkg(struct vctrl *_vctrl, char *pkg)
{
  if (vctrl_loop_over(_vctrl, pkg, &vctrl_clone) == M_FAIL)
    return M_FAIL;

  if (file_size(_vctrl->fclone) == 0)
    fprintf(_vctrl->fclone, pkg);
  else
  {
    if (fseek(_vctrl->fclone, -2, SEEK_END) != 0)
      return M_FAIL;

    char last_c = fgetc(_vctrl->fclone);

    if (fseek(_vctrl->fclone, 0L, SEEK_END) != 0)
      return M_FAIL;

    if (last_c == '\n')
      fprintf(_vctrl->fclone, pkg);
    else
      fprintf(_vctrl->fclone, "\n%s", pkg);
  }
  return M_SUCCESS;
}

static void cleanup(struct mo_program *mo, int code)
{
  m_free(mo->pkg);
  m_free(mo->pkg_name);
  m_free(mo->pkg_version);
  m_free(mo->bin_dir);
  m_free(mo->pkg_dir);
  json_decref(mo->fpd.json_root);
  curl_easy_cleanup(mo->curl_handle);
  vctrl_cleanup(&mo->_vctrl, code);
  curl_global_cleanup();
  printf((code == M_SUCCESS) ? "Successfully installed package."
                             : "Failed to install package.");
  m_deduce();
  exit(code);
}

static int yes_no_prompt()
{
  char r = getchar();

  switch (r)
  {
  case 'Y':
  case 'y':
    return M_SUCCESS;
  case 'N':
  case 'n':
    return M_FAIL;
  default:
    printf("Invalid input, enter again (y/n): ");
    return yes_no_prompt();
  }

  return M_FAIL;
}

static int check_version_vctrl(struct mo_program* mo, const char* newest_version)
{
  int res = M_SUCCESS;

  if (ftell(mo->_vctrl.file) != 0L)
    rewind(mo->_vctrl.file);

  while (fgets(mo->_vctrl.line, sizeof(mo->_vctrl.line), mo->_vctrl.file))
  {
    char* pkg_name = get_str_before_char(mo->_vctrl.line, '@');
    char* pkg_version = get_str_after_char(mo->_vctrl.line, '@');
    if (strcmp(pkg_name, mo->pkg_name) == 0 &&
      mo->pkg_version == NULL &&
      strcmp(pkg_version, newest_version) != 0)
    {
      printf("New version available. Do you want to update? (y/n): ");
      res = yes_no_prompt();
    }

    m_free(pkg_name);
    m_free(pkg_version);
  }

  return res;
}

int main(int argc, char *argv[])
{
  struct mo_program mo;
  int fpd_res;

  /* init program */
  if (m_init(argc, "install") == M_FAIL)
    return M_FAIL;

  curl_global_init(CURL_GLOBAL_ALL);
  mo.curl_handle = curl_easy_init();
  M_ASSERT(mo.curl_handle == NULL, "Could not initialize curl\n");

  mo.pkg = m_strdup(argv[1]);
  if (m_init_install(&mo) == M_FAIL)
  {
    curl_easy_cleanup(mo.curl_handle);
    curl_global_cleanup();
    return M_FAIL;
  }

  asprintf(&mo.pkg_dir, "%s\\mopm\\%s", getenv("APPDATA"), mo.pkg_name);
  asprintf(&mo.bin_dir, "%s\\%s.zip", mo.pkg_dir, mo.pkg_name);

  /* retrieve info */
  if (m_find_package(&mo) == M_FAIL)
    cleanup(&mo, M_FAIL);

  if (mo.pkg_version == NULL)
  {
    /* ask user for updates */
    if (check_version_vctrl(&mo, mo.fpd.version) == M_FAIL)
    {
      fprintf(stderr, "Installation cancelled.\n");
      cleanup(&mo, M_FAIL);
    }

    mo.pkg_version = m_strdup(mo.fpd.version);
    m_free(mo.pkg);
    asprintf(&(mo.pkg), "%s@%s", mo.pkg_name, mo.pkg_version);
  }
  
  create_directory(mo.pkg_dir);
  

  if (m_validate_package(&mo) == M_FAIL)
    cleanup(&mo, M_FAIL);
  
  print_package_info(&mo);
  
  /* download */
  printf("Downloading package...\n");
  M_ASSERT(download_to_file(&mo) != CURLE_OK &&
          /* i couldnt care less */
          (remove(mo.bin_dir) == 0 || remove(mo.bin_dir) == -1),
           "Could not download package\n");

  printf("Successfully downloaded package\n"
         "Extracting...\n");

  /* install */
  M_ASSERT(m_extract(mo.bin_dir, mo.pkg_dir) == M_FAIL, "Failed to extract package\n");
  printf("Successfully extracted package\n");
  M_ASSERT(m_create_batch(&mo) != M_SUCCESS, "Failed to create batch file\n");


  if (file_size(mo._vctrl.file) == 0)
    fprintf(mo._vctrl.fclone, mo.pkg);
  else if (m_vctrl_add_pkg(&(mo._vctrl), mo.pkg) != M_SUCCESS)
    fprintf(stderr, "Failed to add pkg to .vctrl\n");
  cleanup(&mo, M_SUCCESS);
}