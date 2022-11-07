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

#define MOPM_VERSION "0.2.4"

const char *mopm_help = "mopm Version "MOPM_VERSION"\n"
                        "\nUsage: mopm <command> <argument>\n"
                        "To see the usage of a specific command, use: mopm <command>\n"
                        "\nAvailable commands: help\n"
                        "                    install\n"
                        "                    uninstall\n";

static char *as_strcat(char *a, char *b) {
  char *p, *q, *rtn;
  rtn = q = malloc(strlen(a) + strlen(b) + 1);
  for (p = a; (*q = *p) != '\0'; ++p, ++q) {}
  for (p = b; (*q = *p) != '\0'; ++p, ++q) {}
  return rtn;
}

int main(int argc, char *argv[])
{
  int i;
  char *cmd;
  char *cmd_full;
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
  cmd = get_current_directory();
  asprintf(&cmd_full, "%s\\%s", cmd, info.bin);
  for (i = 2; i < argc; i++)
  {
    char *str;
    asprintf(&str, "%s %s", cmd_full, argv[i]);
    free(cmd_full);
    cmd_full = str;
  }
  system(cmd_full);
  free(cmd_full);
  return 0;
}