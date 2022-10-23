#ifndef M_FIND_PACKAGE_H_
#define M_FIND_PACKAGE_H_

#include <curl/curl.h>
struct vctrl;

int find_package(CURL *curl_handle,
                char **pkg, char *pkg_name, char *pkg_version,
                struct vctrl *_vctrl,
                char *bin_dir, char **bin_url);

#endif