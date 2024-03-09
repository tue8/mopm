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
#include "m_debug.h"
#include "m_string.h"
#include "m_directory.h"
#include "m_cmd.h"
#include "../mopm.h"
#include <string.h>

int m_init(int argc, char *cmd)
{
  char *appdata;
  char *mopm_dir;
  struct cmd_info info;
  int result;

  appdata = getenv("APPDATA");

  get_cmd_info(&info, cmd);

  if (argc == 1)
  {
    printf("Usage: mopm %s %s\n", cmd, info.usage);
    return M_FAIL;
  }
  ASSERT(argc != info.args + 1, "Invalid number of arguments provided\n");
  ASSERT(appdata == NULL, "Could not find %%APPDATA%%\n");

  asprintf(&mopm_dir, "%s\\mopm", appdata);
  create_directory(mopm_dir);
  m_free(mopm_dir);
  return M_SUCCESS;
}

static int check_name_len(char *name)
{
  return (((strlen(name) <= 32) && (strlen(name) > 2)) == 0);
}

static int check_version_len(char *version)
{
  return (((strlen(version) <= 32) && (strlen(version) > 0)) == 0);
}

static int install_fail(struct mo_program *mo)
{
  m_free(mo->pkg);
  if (mo->pkg_name != NULL)
    m_free(mo->pkg_name);
  if (mo->pkg_version != NULL)
    m_free(mo->pkg_version);
  vctrl_cleanup(&(mo->_vctrl), 0);
  return M_FAIL;
}

int m_init_install(struct mo_program *mo)
{
  if (vctrl_init(&(mo->_vctrl)) == 1)
    return install_fail(mo);

  if (strlen(mo->pkg) == 0)
  {
    fprintf(stderr, "Invalid package name\n");
    return install_fail(mo);
  }
  mo->pkg_name = get_str_before_char(mo->pkg, '@');
  mo->pkg_version = get_str_after_char(mo->pkg, '@');
  if (mo->pkg_name == NULL && mo->pkg_version == NULL)
  {
    mo->pkg_name = m_strdup(mo->pkg);
    mo->pkg_version = NULL;
  }
  else if ((mo->pkg_name != NULL && mo->pkg_version == NULL) ||
           (mo->pkg_name == NULL && mo->pkg_version != NULL))
  {
    fprintf(stderr, "Invalid package name/version");
    return install_fail(mo);
  }
  if (check_name_len(mo->pkg_name) == 1 ||
      (mo->pkg_version != NULL && check_version_len(mo->pkg_version) == 1))
  {
    fprintf(stderr, "Invalid package name/version length");
    return install_fail(mo);
  }

  return M_SUCCESS;
}

static int uninstall_fail(struct mo_program *mo)
{
  m_free(mo->pkg);
  vctrl_cleanup(&(mo->_vctrl), 0);
  return M_FAIL;
}

int m_init_uninstall(struct mo_program *mo)
{
  char *last_char;

  if (vctrl_init(&(mo->_vctrl)) == 1)
    return uninstall_fail(mo);
  if (check_name_len(mo->pkg) == 1)
  {
    fprintf(stderr, "Invalid package name length");
    return uninstall_fail(mo);
  }

  last_char = strrchr(mo->pkg, '@');
  if (last_char != NULL)
  {
    fprintf(stderr, "You cannot uninstall a specific package version in mopm");
    return uninstall_fail(mo);
  }

  return M_SUCCESS;
}