/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#ifndef M_FIND_PACKAGE_H_
#define M_FIND_PACKAGE_H_

#include <curl/curl.h>
#include <jansson.h>

struct find_package_data
{
  json_t *json_root;
  const char *entry, *version, *des, *author, *license, *checksum, *bin_url;
};

struct mo_program;

int m_find_package(struct mo_program *mo);

#endif