#ifndef M_CHECK_H_
#define M_CHECK_H_

#include <curl/curl.h>

struct find_package_data;
struct vctrl;

int check_name_len(char *name);
int check_version_len(char *version);
int check_name_version(char **pkg, char **pkg_version);
int check_package_install(struct find_package_data *fpd, CURL *curl_handle,
                          struct vctrl *_vctrl, char *pkg,
                          char *pkg_name, char *pkg_version,
                          char *bin_dir, char **bin_url);

#endif