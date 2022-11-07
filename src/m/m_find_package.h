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
  int result;
  const char *version, *des, *author, *license, *checksum, *bin_url;
};

int free_fpd(struct find_package_data *fpd);
int check_fpd(struct find_package_data *fpd);
int find_package(struct find_package_data *ret_data, CURL *curl_handle,
                 char *pkg, char *pkg_name, char *pkg_version);

#endif