#ifndef M_DIRECTORY
#define M_DIRECTORY

#include <windows.h>

#define create_directory(path)                                                 \
  if (CreateDirectory(path, NULL) == 0                                         \
  && GetLastError() != ERROR_ALREADY_EXISTS)                                   \
  {                                                                            \
    fprintf(stderr, "Could not create " #path "\n");                           \
    goto out;                                                                  \
  }

char *get_current_directory();

#endif