#include "m_curl.h"

static size_t write_file(void *data, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(data, size, nmemb, stream);
    return written;
}

static size_t write_get_res(void *data, size_t size, size_t nmemb,
                            struct get_res *res)
{
    size_t new_len = res->len + size * nmemb;
    res->ptr = realloc(res->ptr, new_len + 1);

    if (res->ptr == NULL)
    {
        perror("Could not re-allocate memory\n");
        return 1;
    }

    memcpy(res->ptr + res->len, data, size * nmemb);
    res->ptr[new_len] = '\0';
    res->len = new_len;

    return size * nmemb;
}

int send_http_get(CURL *curl_handle, const char *url, struct get_res *res)
{
    CURLcode perform_res;
    curl_off_t size;

    curl_easy_reset(curl_handle);

    res->len = 0;
    res->ptr = malloc(res->len + 1);
    if (res->ptr == NULL)
    {
        perror("Could not allocate memory");
        return 1;
    }
    res->ptr[0] = '\0';

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mopm");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_get_res);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, res);

    perform_res = curl_easy_perform(curl_handle);
    curl_easy_reset(curl_handle);
    return 0;
}

int download_to_file(CURL *curl_handle, char *url, char *file_dir)
{
    FILE *file_handler;
    CURLcode perform_res;

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mopm");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_file);

    file_handler = fopen(file_dir, "wb");
    if (file_handler == NULL)
    {
        fclose(file_handler);
        return -1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file_handler);
    perform_res = curl_easy_perform(curl_handle);

    fclose(file_handler);
    curl_easy_reset(curl_handle);
    return perform_res;
}