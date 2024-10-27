/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include <stdio.h>
#include <stdlib.h>
#include "m/m_string.h"
#include "m/m_directory.h"
#include "m/m_init.h"
#include "m/m_cmd.h"
#include "m/m_debug.h"

#define MOPM_VERSION "0.3.0"

const char *mopm_help = "mopm Version "MOPM_VERSION"\n"
                        "\nUsage: mopm <command> <argument>\n"
                        "To see the usage of a specific command, use: mopm <command>\n\n"
                        "Available commands: install\n"
                        "                    uninstall\n";

int main(int argc, char *argv[])
{
  int i;
  char *cmd;
  struct cmd_info info;

  if (argc == 1)
  {
    printf(mopm_help);
    return 1;
  }
  else if (argc < 2)
  {
    fprintf(stderr, "Too few arguments provided\n");
    return 1;
  }

  if (get_cmd_info(&info, argv[1]) == 1)
  {
    fprintf(stderr, "Unknown command, see 'mopm' for more details.\n");
    return 1;
  }

  cmd = m_strdup(info.bin);
  for (i = 2; i < argc; i++)
  {
    char *str;
    asprintf(&str, "%s %s", cmd, argv[i]);
    m_free(cmd);
    cmd = str;
  }
  system(cmd);
  m_free(cmd);
  return 0;
}