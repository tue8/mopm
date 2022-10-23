#include "m/m_string.h"
#include "m/m_curl.h"
#include "m/m_vctrl.h"
#include "m/m_find_package.h"
#include "m/m_directory.h"
#include "m/m_check.h"
#include <stdio.h>

static int vctrl_condition_func(struct vctrl *_vctrl, char *pkg, void *ud)
{
  fputs(_vctrl->line, _vctrl->file2);
  return 0;
}

static int vctrl_condition(struct vctrl *_vctrl, char *line, char *pkg, void *ud)
{
  char *l_pkg_name = get_str_before_char(line, '@');
  char *l_pkg_version = get_str_after_char(line, '@');
  char *pkg_name = get_str_before_char(pkg, '@');
  char *pkg_version = get_str_after_char(pkg, '@');

  int result = 0;

  if (l_pkg_name == NULL || l_pkg_version == NULL)
    result = 1;
  else
  {
    l_pkg_version[strlen(l_pkg_version) - 1] = '\0';

    if (strcmp(l_pkg_name, pkg_name) == 0
    && strcmp(l_pkg_version, pkg_version) != 0)
      result = 1;  
  }
  
  free(l_pkg_name);
  free(l_pkg_version);
  free(pkg_name);
  free(pkg_version);
  return result;
}

int main(int argc, char *argv[])
{
  CURL *curl_handle;

  char *pkg_name;
  char *pkg_version;

  struct vctrl _vctrl;

  char *bin_url;
  char *bin_dir;

  char *pkg = strdup(argv[1]);

  int success = 0;
  CURLcode download_result = 0;

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  if (curl_handle == NULL)
  {
    fprintf(stderr, "Could not initialize curl\n");
    return 1;
  }

  if (argc != 2)
  {
    fprintf(stderr, "Invalid arguments count\n");
    return 1;
  }

  if (strlen(pkg) == 0)
  {
    fprintf(stderr, "Invalid package name");
    goto out;
  }

  if ((pkg_name = get_str_before_char(pkg, '@')) == NULL ||
      (pkg_version = get_str_after_char(pkg, '@')) == NULL)
  {
    pkg_name = strdup(pkg);
    pkg_version = NULL;
  }

  if (check_name_len(pkg_name) == 1)
  {
    fprintf(stderr, "Invalid package name");
    goto out;
  }

  if (pkg_version != NULL && check_version_len(pkg_version) == 1)
  {
    fprintf(stderr, "Invalid package version");
    goto out;
  }

  if (vctrl_init(&_vctrl, 1) == 1)
    goto out;

  asprintf(&bin_dir, "%s\\mopm\\%s.exe", getenv("APPDATA"), pkg_name);

  if (find_package(curl_handle,
                   &pkg, pkg_name, pkg_version,
                   &_vctrl,
                   bin_dir, &bin_url) != 0)
    goto out;

  printf("Downloading package...\n");

  download_result = download_to_file(curl_handle, bin_url, bin_dir);

  free(bin_url);

  if (download_result != CURLE_OK)
  {
    fprintf(stderr, "Could not download package\n");
    remove(bin_dir);
    goto out;
  }

  printf("Successfully downloaded package\n");

  if (file_size(_vctrl.file) == 0)
    fprintf(_vctrl.file2, pkg);
  else
  {
    struct vctrl_pkg_con_data result;
    int exist = 0;

    vctrl_pkg_con(&result, &_vctrl, pkg, &exist,
                  &vctrl_condition_func, &vctrl_condition);

    if (exist == 0)
    {
      char last_c;

      if (fseek(_vctrl.file2, -2, SEEK_END) != 0)
        goto out;

      last_c = fgetc(_vctrl.file2);

      if (fseek(_vctrl.file2, 0L, SEEK_END) != 0)
        goto out;

      if (last_c == '\n')
        fprintf(_vctrl.file2, pkg);
      else
        fprintf(_vctrl.file2, "\n%s", pkg);
    }
  }

  success = 1;
out:
  free(bin_dir);
  free(pkg);
  free(pkg_name);
  free(pkg_version);

  vctrl_cleanup(&_vctrl, success);

  if (success == 1)
    printf("Successfully installed package.");
  else
    printf("Failed to install package.");
  return success == 0;
}