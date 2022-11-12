/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_init.h"
#include <stdio.h>
#include <stdlib.h>
#include "m_vctrl.h"
#include "m_debug.h"
#include "m_string.h"
#include "m_directory.h"
#include "m_cmd.h"
#include <string.h>

int m_init(int argc, char *cmd)
{
  char *appdata;
  char *mopm_dir;
  struct cmd_info info;
  int result;

  appdata = getenv("APPDATA");
  result = 1;
  
  get_cmd_info(&info, cmd);
  if (argc == 1)
  {
    printf("Usage: mopm %s %s\n", cmd, info.usage);
    return 1;
  }
  else if (argc != info.args+1)
  {
    fprintf(stderr, "Invalid number of arguments provided\n");
    return 1;
  }
  if (appdata == NULL)
  {
    fprintf(stderr, "Could not find %%APPDATA%%\n");
    return 1;
  }
  asprintf(&mopm_dir, "%s\\mopm", appdata);
  create_directory(mopm_dir);
  result = 0;
out:
  free(mopm_dir);
  return result;
}

static int check_name_len(char *name)
{
  return (((strlen(name) <= 32) && (strlen(name) > 2)) == 0);
}

static int check_version_len(char *version)
{
  return (((strlen(version) <= 32) && (strlen(version) > 0)) == 0);
}

int m_init_install(struct vctrl *_vctrl, char *pkg, char **pkg_name, char **pkg_version)
{
  int result;

  result = 1;
  if (vctrl_init(_vctrl) == 1)
    goto out;
  if (strlen(pkg) == 0)
  {
    fprintf(stderr, "Invalid package name\n");
    goto out;
  }
  *pkg_name = get_str_before_char(pkg, '@');
  *pkg_version = get_str_after_char(pkg, '@');
  if (*pkg_name == NULL && *pkg_version == NULL)
  {
    *pkg_name = m_strdup(pkg);
    *pkg_version = NULL;
  }
  else if ((*pkg_name != NULL && *pkg_version == NULL) ||
           (*pkg_name == NULL && *pkg_version != NULL))
  {
    fprintf(stderr, "Invalid package name/version");
    goto out;
  }
  if (check_name_len(*pkg_name) == 1 ||
     (*pkg_version != NULL && check_version_len(*pkg_version) == 1))
  {
    fprintf(stderr, "Invalid package name/version length");
    goto out;
  }
  result = 0;
out:
  if (result == 1)
  {
    m_free(pkg);
    if (*pkg_name != NULL)
      m_free(*pkg_name);
    if (*pkg_version != NULL)
      m_free(*pkg_version);
    vctrl_cleanup(_vctrl, 0);
  }
  return result;
}

int m_init_uninstall(struct vctrl *_vctrl, char *pkg)
{
  int result;
  char *last_char;

  result = 1;
  if (vctrl_init(_vctrl) == 1)
    goto out;
  if (check_name_len(pkg) == 1)
  {
    fprintf(stderr, "Invalid package name length");
    goto out;
  }
  last_char = strrchr(pkg, '@');
  if (last_char != NULL)
  {
    fprintf(stderr, "You cannot uninstall a specific package version in mopm");
    goto out;
  }
  result = 0;
out:
  if (result == 1)
  {
    m_free(pkg);
    vctrl_cleanup(_vctrl, 0);
  }
  return result;
}