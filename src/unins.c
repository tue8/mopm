#include "m/m_string.h"
#include "m/m_curl.h"
#include "m/m_vctrl.h"
#include "m/m_find_package.h"
#include "m/m_check.h"
#include <stdio.h>

static int vctrl_condition_func(struct vctrl *_vctrl, char *pkg, void *ud)
{
  fputs(_vctrl->line, _vctrl->file2);
  return 0;
}

static int vctrl_condition(struct vctrl *_vctrl, char *line, char *pkg, void *ud)
{
  return (strcmp(line, pkg) == 0);
}

int main(int argc, char *argv[])
{
  CURL *curl_handle;

  char *pkg_name;
  char *pkg_version;

  struct vctrl _vctrl;

  char *bin_dir;

  char *pkg = strdup(argv[1]);

  int success = 0;

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
                   bin_dir, NULL) != 3)
    goto out;

  if (file_size(_vctrl.file) == 0)
    goto out;
  else
  {
    struct vctrl_pkg_con_data result;

    vctrl_pkg_con(&result, &_vctrl, pkg, NULL,
                  &vctrl_condition_func, &vctrl_condition);
  }

  if (remove(bin_dir) != 0)
  {
    perror("Could not remove package's binary");
    goto out;
  }

  success = 1;
out:
  free(bin_dir);
  free(pkg);
  free(pkg_name);
  free(pkg_version);

  vctrl_cleanup(&_vctrl, success);

  if (success == 1)
    printf("Successfully uninstalled package.");
  else
    printf("Failed to uninstall package.");
  return success == 0;
}