/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_curl.h"
#include "m_string.h"
#include "m_debug.h"
#include "../mopm.h"
#include <string.h>

#define PBLEN 40

static int progress_bar(curl_off_t curr, curl_off_t total)
{
  int done = (int)(((double)curr / (double)total) * PBLEN), i, j;
  int remain = PBLEN - done;
  char buffer[PBLEN + 3] = "";

  m_strcat(buffer, "[");

  for (i = 0; i < done - 1; i++)
    m_strcat(buffer, "=");

  if (done > 0)
    m_strcat(buffer, ">");

  for (j = 0; j < remain; j++)
    m_strcat(buffer, " ");

  m_strcat(buffer, "]");

  printf("\r%s", buffer);
  fflush(stdout);
  return 0;
}

struct download_data
{
  CURL *curl_handle;
  curl_off_t total_len;
  curl_off_t curr_len;
  FILE *file;
};

static size_t write_file(void *data, size_t size, size_t nmemb, struct download_data *dd)
{
  size_t written = fwrite(data, size, nmemb, dd->file);
  dd->curr_len += written;
  if (dd->total_len == 0)
    curl_easy_getinfo(dd->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dd->total_len);
  progress_bar(dd->curr_len, dd->total_len);
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

static int init_curl_handler(CURL *curl_handle, const char *url)
{
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mopm");
  return 0;
}

int send_http_get(CURL *curl_handle, char *url, struct get_res *res)
{
  CURLcode perform_res;
  curl_off_t size;

  init_curl_handler(curl_handle, url);

  res->len = 0;
  res->ptr = m_malloc(res->len + 1);
  if (res->ptr == NULL)
  {
    perror("Could not allocate memory");
    return 1;
  }
  res->ptr[0] = '\0';

  curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_get_res);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, res);

  perform_res = curl_easy_perform(curl_handle);
  curl_easy_reset(curl_handle);
  return 0;
}

int download_to_file(struct mo_program *mo)
{
  CURLcode perform_res;
  struct download_data dd;

  init_curl_handler(mo->curl_handle, mo->fpd.bin_url);

  curl_easy_setopt(mo->curl_handle, CURLOPT_WRITEFUNCTION, write_file);

  dd.total_len = 0;
  dd.curr_len = 0;
  dd.curl_handle = mo->curl_handle;
  dd.file = fopen(mo->bin_dir, "wb");

  if (dd.file == NULL)
  {
    fclose(dd.file);
    return -1;
  }

  curl_easy_setopt(mo->curl_handle, CURLOPT_WRITEDATA, &dd);
  perform_res = curl_easy_perform(mo->curl_handle);
  printf("\n");

  fclose(dd.file);
  curl_easy_reset(mo->curl_handle);
  return perform_res;
}