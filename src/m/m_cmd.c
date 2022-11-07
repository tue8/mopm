/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_cmd.h"
#include <string.h>

int get_cmd_info(struct cmd_info *info, char *cmd)
{
  if (strcmp(cmd, "install") == 0)
  {
    info->bin = "ins";
    info->usage = "<package name>@<package version> || <package name>";
    info->args = 1;
  }
  else if (strcmp(cmd, "uninstall") == 0)
  {
    info->bin = "unins";
    info->usage = "<package name>";
    info->args = 1;
  }
  else
    return 1;
  return 0;
}