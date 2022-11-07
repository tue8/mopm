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
#include "m/m_checksum.h"
#include "m/m_extract.h"
#include "m/m_debug.h"
#include <stdio.h>

static int print_package_info(char *name, char *ver, const char *fpd_ver,
                              const char *des, const char *license,
                              const char *author)
{
  printf("Found: %s Version %s\n"
         "Description: %s\n"
         "License: %s\n"
         "Author: %s\n",
         name, (ver == NULL) ? fpd_ver : ver, des, license, author);
}

static int check_package_install(struct find_package_data *fpd,
                                 struct vctrl *_vctrl, char *pkg,
                                 char *bin_dir)
{
  if (check_fpd(fpd) == 1)
    return 1;
  else
  {
    if (verify_checksum(_vctrl, bin_dir, pkg, fpd->checksum) == 1)
    {
      fprintf(stderr, "Package is already installed\n");
      return 1;
    }
  }
  return 0;
}

static int vctrl_condition_func(struct vctrl *_vctrl, char *pkg, void *ud)
{
  fputs(_vctrl->line, _vctrl->file2);
  return 0;
}

static int vctrl_condition(struct vctrl *_vctrl, char *line, char *pkg, void *ud)
{
  char *l_pkg_name;
  char *l_pkg_version;
  char *pkg_name;
  char *pkg_version;
  int result;
  int name_result;
  int version_result;

  result = 0;
  if ((l_pkg_name = get_str_before_char(line, '@')) == NULL || (l_pkg_version = get_str_after_char(line, '@')) == NULL)
    result = 1;
  else
  {
    name_result = strcmp(l_pkg_name,
                         (pkg_name = get_str_before_char(pkg, '@')));
    version_result = strcmp(l_pkg_version,
                            (pkg_version = get_str_after_char(pkg, '@')));
    l_pkg_version[strlen(l_pkg_version) - 1] = '\0';
    if (name_result == 0 && version_result == 0)
      *(int *)ud = 1;
    else if (name_result == 0 && version_result != 0)
      result = 1;
  }
  m_free(l_pkg_name);
  m_free(l_pkg_version);
  m_free(pkg_name);
  m_free(pkg_version);
  return result;
}

static int vctrl_print_pkg(struct vctrl *_vctrl, char *pkg)
{
  struct vctrl_pkg_con_data result;
  int exist;

  exist = 0;
  vctrl_pkg_con(&result, _vctrl, pkg, &exist,
                &vctrl_condition_func, &vctrl_condition);
  if (exist != 1)
  {
    char last_c;
    if (fseek(_vctrl->file2, -2, SEEK_END) != 0)
      return 1;
    last_c = fgetc(_vctrl->file2);
    if (fseek(_vctrl->file2, 0L, SEEK_END) != 0)
      return 1;
    if (last_c == '\n')
      fprintf(_vctrl->file2, pkg);
    else
      fprintf(_vctrl->file2, "\n%s", pkg);
  }
  return 0;
}

int main(int argc, char *argv[])
{
  CURL *curl_handle;
  char *pkg_name;
  char *pkg_version;
  struct vctrl _vctrl;
  char *pkg_dir;
  char *bin_dir;
  char *pkg;
  int success;
  CURLcode download_result;
  struct find_package_data fpd;

  if (m_init(argc, "install") == 1)
    return 1;
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
  if (curl_handle == NULL)
  {
    fprintf(stderr, "Could not initialize curl\n");
    return 1;
  }
  pkg = m_strdup(argv[1]);
  if (m_init_install(&_vctrl, pkg, &pkg_name, &pkg_version, argc) == 1)
  {
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 1;
  }
  asprintf(&pkg_dir, "%s\\mopm\\%s", getenv("APPDATA"), pkg_name);
  asprintf(&bin_dir, "%s\\%s.zip", pkg_dir, pkg_name);

  success = 0;
  create_directory(pkg_dir);
  find_package(&fpd, curl_handle, pkg, pkg_name, pkg_version);
  if (check_package_install(&fpd, &_vctrl, pkg, bin_dir) != 0)
  {
    remove_directory(pkg_dir);
    goto out;
  }
  print_package_info(pkg_name, pkg_version, fpd.version, fpd.des, fpd.license, fpd.author);
  if (pkg_version == NULL)
  {
    m_free(pkg);
    asprintf(&pkg, "%s@%s", pkg_name, fpd.version);
  }
  printf("Downloading package...\n");
  download_result = download_to_file(curl_handle, fpd.bin_url, bin_dir);
  free_fpd(&fpd);
  if (download_result != CURLE_OK)
  {
    fprintf(stderr, "Could not download package\n");
    remove(bin_dir);
    goto out;
  }
  printf("Successfully downloaded package\n"
         "Extracting...\n");
  if (extract(bin_dir, pkg_dir) == 0)
    printf("Successfully extracted package\n");
  else
  {
    fprintf(stderr, "Failed to extract package\n");
    goto out;
  }

  if (file_size(_vctrl.file) == 0)
    fprintf(_vctrl.file2, pkg);
  else
    if (vctrl_print_pkg(&_vctrl, pkg) != 0)
    {
      fprintf(stderr, "Failed to edit .vctrl\n");
    }
  success = 1;
out:
  m_free(pkg);
  m_free(pkg_name);
  if (pkg_version != NULL)
    m_free(pkg_version);
  m_free(bin_dir);
  m_free(pkg_dir);
  vctrl_cleanup(&_vctrl, success);
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  printf((success == 1) ? "Successfully installed package."
                        : "Failed to install package.");
  return success == 0;
}