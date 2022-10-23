#ifndef M_CURL_H_
#define M_CURL_H

#include <curl/curl.h>

struct get_res
{
    char *ptr;
    size_t len;
};

int send_http_get(CURL *curl_handle, const char *url, struct get_res *res);
int download_to_file(CURL *curl_handle, char *url, char *file_dir);

#endif