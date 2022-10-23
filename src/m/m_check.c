#include "m_check.h"
#include <string.h>

int check_name_len(char *name)
{
  return (((strlen(name) <= 100) && (strlen(name) > 2)) == 0);
}

int check_version_len(char *version)
{
  return (((strlen(version) <= 32) && (strlen(version) > 0)) == 0);
}