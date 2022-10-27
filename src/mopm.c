#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "m/m_string.h"
#include "m/m_directory.h"

#define MOPM_VERSION "0.2.4"

const char *mopm_help = "mopm Package Manager Version " MOPM_VERSION "\n"
                        "A package manager for Windows that installs cli apps "
                        "onto your pc\n"
                        "\nUsage: mopm <command> <argument>\n"
                        "\nAvailable commands: help\n"
                        "                    install\n"
                        "                    uninstall\n";

static char *get_bin_from_cmd(char *cmd)
{
  if (strcmp(cmd, "install") == 0 || strcmp(cmd, "i") == 0 || strcmp(cmd, "ins") == 0)
    return "ins";
  if (strcmp(cmd, "uninstall") == 0 || strcmp(cmd, "u") == 0 || strcmp(cmd, "ui") == 0
  || strcmp(cmd, "unins") == 0)
    return "unins";

  return NULL;
}

int main(int argc, char *argv[])
{
  char *appdata = getenv("APPDATA");
  char *mopm_dir, *cmd, *cmd_bin;
  int i = 0;

  if (argc < 2)
  {
    fprintf(stderr, "Too few arguments provided\n");
    return 1;
  }

  if (appdata == NULL)
  {
    fprintf(stderr, "Could not find %%APPDATA%%\n");
    return 1;
  }

  asprintf(&mopm_dir, "%s\\mopm", appdata);
  cmd = get_current_directory();

  create_directory(mopm_dir);

  if (strcmp(argv[1], "help") == 0)
  {
    printf(mopm_help);
    goto out;
  }

  cmd_bin = get_bin_from_cmd(argv[1]);

  if (cmd_bin == NULL)
  {
    fprintf(stderr, "Unknown command, see 'mopm help' for more details.\n");
    goto out;
  }

  asprintf(&cmd, "%s\\%s.exe", cmd, cmd_bin);

  for (i = 2; i < argc; i++)
  {
    asprintf(&cmd, "%s %s", cmd, argv[i]);
  }

  system(cmd);
out:
  free(mopm_dir);
  free(cmd);

  return 0;
}