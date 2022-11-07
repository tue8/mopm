/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_CMD_H_
#define M_CMD_H_

struct cmd_info
{
  char *bin;
  char *usage;
  int args;
};

int get_cmd_info(struct cmd_info *info, char *cmd);

#endif