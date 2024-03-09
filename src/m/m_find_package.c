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
#include "../mopm.h"
#include <stdio.h>
#include <string.h>

#define FP_SUCCESS 0
#define FP_NOT_FOUND_ERR 1
#define FP_MANIFEST_ERR 2

char *manifest_origin_url = "https://raw.githubusercontent.com/Localtings/"
                            "mopm-pkgs/main/packages/%s/manifest.json";

static int fp_fail(struct mo_program *mo, int code)
{
  switch (code)
  {
  case FP_NOT_FOUND_ERR:
    fprintf(stderr, "Package could not be found\n");
    break;
  case FP_MANIFEST_ERR:
    fprintf(stderr, "Package's manifest error\n");
    break;
  }
}

static int extract_json_fpd(struct mo_program *mo)
{
  json_t *stable_ver = json_object_get(mo->fpd.json_root, "stable");
  json_t *versions = json_object_get(mo->fpd.json_root, "versions");
  int i;

  if (json_is_string(stable_ver) == 0 || json_is_array(versions) == 0)
    return fp_fail(mo, FP_MANIFEST_ERR);

  for (i = 0; i < json_array_size(versions); i++)
  {
    json_t *version_obj = json_array_get(versions, i);

    if (json_is_object(version_obj))
    {
      json_t *bin_url = json_object_get(version_obj, "file");
      json_t *entry = json_object_get(version_obj, "entry");
      json_t *version = json_object_get(version_obj, "version");
      json_t *des = json_object_get(version_obj, "description");
      json_t *author = json_object_get(version_obj, "author");
      json_t *license = json_object_get(version_obj, "license");
      json_t *checksum = json_object_get(version_obj, "checksum");

      if (json_is_string(version) == 0 || json_is_string(des) == 0 ||
          json_is_string(author) == 0 || json_is_string(license) == 0 ||
          json_is_string(checksum) == 0 || json_is_string(bin_url) == 0 ||
          json_is_string(entry) == 0)
        return fp_fail(mo, FP_MANIFEST_ERR);

      if (strcmp(json_string_value(version),
         (mo->pkg_version == NULL) ? json_string_value(stable_ver) : mo->pkg_version) == 0)
      {
        mo->fpd.bin_url = json_string_value(bin_url);
        mo->fpd.entry = json_string_value(entry);
        mo->fpd.version = json_string_value(stable_ver);
        mo->fpd.des = json_string_value(des);
        mo->fpd.author = json_string_value(author);
        mo->fpd.license = json_string_value(license);
        mo->fpd.checksum = json_string_value(checksum);
        return FP_SUCCESS;
      }
    }
  }

  return fp_fail(mo, FP_NOT_FOUND_ERR);
}

int m_find_package(struct mo_program *mo)
{
  char *manifest_url;
  struct get_res manifest_raw;
  json_error_t err_buffer;

  printf("Searching for package...\n");
  asprintf(&manifest_url, manifest_origin_url, mo->pkg_name);

  CURLcode result = send_http_get(mo->curl_handle, manifest_url, &manifest_raw);
  m_free(manifest_url);

  mo->fpd.json_root = json_loads(manifest_raw.ptr, 0, &err_buffer);
  m_free(manifest_raw.ptr);

  if (result != CURLE_OK || strcmp(manifest_raw.ptr, "404: Not Found") == 0)
    return fp_fail(mo, FP_NOT_FOUND_ERR);
  if (mo->fpd.json_root == 0 || json_is_object(mo->fpd.json_root) == 0)
    return fp_fail(mo, FP_MANIFEST_ERR);

  return extract_json_fpd(mo);
}