#ifndef M_FIND_PACKAGE_H_
#define M_FIND_PACKAGE_H_

#include <curl/curl.h>
#include <jansson.h>

struct find_package_data
{
  int result;
  char *rls_url, *version, *des, *author, *license, *checksum;
};

int free_fpd(struct find_package_data *fpd);
int check_fpd(struct find_package_data *fpd);
int get_binary_url(CURL *curl_handle, const char *rls_url,
                  const char *pkg_version, char **bin_url);
int find_package(struct find_package_data *ret_data, CURL *curl_handle,
                 char *pkg, char *pkg_name, char *pkg_version);

#endif