/*
 * Created on Sun Nov 06 2022
 * mopm Package Manger
 * https://github.com/Localtings/mopm
 * Licensed under MIT license
 * Copyright (c) 2022 Localtings
 */

#include "m_find_package.h"
#include "m_string.h"
#include "m_curl.h"
#include "m_debug.h"
#include <stdio.h>

int free_fpd(struct find_package_data *fpd)
{
  json_decref(fpd->json_root);
  return 0;
}

int check_fpd(struct find_package_data *fpd)
{
  if (fpd->result != 0)
  {
    switch (fpd->result)
    {
    case 1:
      fprintf(stderr, "Package could not be found\n");
      break;
    case 2:
      fprintf(stderr, "Package's manifest error\n");
      break;
    }
  }

  return (fpd->result != 0);
}

int find_package(struct find_package_data *ret_data, CURL *curl_handle,
                 char *pkg, char *pkg_name, char *pkg_version)
{
  char *manifest_url;
  struct get_res manifest_raw;

  json_t *data;
  json_error_t err_buffer;

  ret_data->result = 1;

  printf("Searching for package...\n");
  asprintf(&manifest_url, "https://raw.githubusercontent.com/Localtings/"
                          "mopm-pkgs/main/packages/%s/manifest.json",
           pkg_name);

  if (send_http_get(curl_handle, manifest_url, &manifest_raw) != CURLE_OK)
    return 1;

  m_free(manifest_url);

  if (strcmp(manifest_raw.ptr, "404: Not Found") == 0)
  {
    m_free(manifest_raw.ptr);
    return 1;
  }

  ret_data->json_root = json_loads(manifest_raw.ptr, 0, &err_buffer);
  m_free(manifest_raw.ptr);

  if (ret_data->json_root == 0 || json_is_object(ret_data->json_root) == 0)
  {
    ret_data->result = 2;
    goto out;
  }
  else
  {
    json_t *stable_ver = json_object_get(ret_data->json_root, "stable");
    json_t *versions = json_object_get(ret_data->json_root, "versions");
    int i;

    if (json_is_string(stable_ver) == 0 || json_is_array(versions) == 0)
    {
      ret_data->result = 2;
      goto out;
    }

    for (i = 0; i < json_array_size(versions); i++)
    {
      json_t *version_obj = json_array_get(versions, i);

      if (json_is_object(version_obj))
      {
        json_t *bin_url = json_object_get(version_obj, "file");
        json_t *version = json_object_get(version_obj, "version");
        json_t *des = json_object_get(version_obj, "description");
        json_t *author = json_object_get(version_obj, "author");
        json_t *license = json_object_get(version_obj, "license");
        json_t *checksum = json_object_get(version_obj, "checksum");

        if (json_is_string(version) == 0 || json_is_string(des) == 0 ||
            json_is_string(author) == 0 || json_is_string(license) == 0 ||
            json_is_string(checksum) == 0 || json_is_string(bin_url) == 0)
        {
          ret_data->result = 2;
          goto out;
        }

        if (strcmp(json_string_value(version), (pkg_version == NULL) ?
        json_string_value(stable_ver) : pkg_version)  == 0)
        {
          ret_data->version = json_string_value(stable_ver);
          ret_data->des = json_string_value(des);
          ret_data->author = json_string_value(author);
          ret_data->license = json_string_value(license);
          ret_data->checksum = json_string_value(checksum);
          ret_data->bin_url = json_string_value(bin_url);
          ret_data->result = 0;
          goto out;
        }
      }
    }
  }
out:
  if (ret_data->result != 0)
    json_decref(ret_data->json_root);
  return ret_data->result;
}