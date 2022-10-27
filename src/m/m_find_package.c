#include "m_find_package.h"
#include "m_string.h"
#include "m_curl.h"
#include "m_debug.h"
#include <stdio.h>

int free_fpd(struct find_package_data *fpd)
{
  m_free(fpd->rls_url);
  m_free(fpd->version);
  m_free(fpd->license);
  m_free(fpd->des);
  m_free(fpd->checksum);
  m_free(fpd->author);
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

int get_binary_url(CURL *curl_handle, const char *rls_url,
                  const char *pkg_version, char **bin_url)
{
  struct get_res releases_json;
  json_t *root;
  json_t *data;
  json_error_t error;
  int result = 1;

  if (bin_url == NULL)
    goto out;

  if (send_http_get(curl_handle, rls_url, &releases_json) != CURLE_OK)
  {
    fprintf(stderr, "Could not send http request\n");
    goto out;
  }

  if (strcmp(releases_json.ptr, "") == 0 ||
      strcmp(releases_json.ptr, "404: Not Found") == 0)
  {
    fprintf(stderr, "Invalid releases url provided\n");
    goto out;
  }

  root = json_loads(releases_json.ptr, 0, &error);
  m_free(releases_json.ptr);

  if (root == 0)
  {
    fprintf(stderr, "Invalid json - Error on line %d: %s\n",
            error.line, error.text);
    goto out;
  }

  if (json_is_array(root) == 0)
  {
    fprintf(stderr, "Invalid json\n");
    goto out;
  }

  for (int i = 0; i < json_array_size(root); i++)
  {
    json_t *element = json_array_get(root, i);
    json_t *version = json_object_get(element, "name");
    json_t *assets = json_object_get(element, "assets");
    json_t *tag = json_object_get(element, "tag_name");

    if (json_is_object(element) && json_is_array(assets) &&
        json_is_string(tag) &&
        strcmp(json_string_value(version), pkg_version) == 0)
    {
      json_t *assets_obj = json_array_get(assets, 0);

      if (strcmp(json_string_value(tag), "mopm") != 0)
        continue;

      if (json_is_object(assets_obj))
      {
        json_t *download_url = json_object_get(assets_obj,
                                               "browser_download_url");
        if (json_is_string(download_url))
        {
          const char *bin_url_json = json_string_value(download_url);

          if (bin_url_json == NULL || strcmp(bin_url_json, "") == 0)
            continue;

          *bin_url = strdup(bin_url_json);
          result = 0;
          goto out;
        }
      }
    }
  }

  fprintf(stderr, "Could not find package's binary\n");
out:
  json_decref(root);
  return result;
}

int find_package(struct find_package_data *ret_data, CURL *curl_handle,
                 char *pkg, char *pkg_name, char *pkg_version)
{
  char *manifest_url;
  struct get_res manifest_raw;

  json_t *root;
  json_t *data;
  json_error_t err_buffer;

  ret_data->result = 1;

  printf("Searching for package...\n");
  asprintf(&manifest_url, "https://raw.githubusercontent.com/Localtings/"
                          "mopm-pkgs/main/packages/%s/manifest.json",
           pkg_name);

  if (send_http_get(curl_handle, manifest_url, &manifest_raw) != CURLE_OK)
    goto out;

  m_free(manifest_url);

  if (strcmp(manifest_raw.ptr, "404: Not Found") == 0)
  {
    m_free(manifest_raw.ptr);
    goto out;
  }

  root = json_loads(manifest_raw.ptr, 0, &err_buffer);
  m_free(manifest_raw.ptr);

  if (root == 0 || json_is_object(root) == 0)
  {
    ret_data->result = 2;
    goto out;
  }
  else
  {
    json_t *rls_url = json_object_get(root, "releases_url");
    json_t *stable_ver = json_object_get(root, "stable");
    json_t *versions = json_object_get(root, "versions");

    if (json_is_string(rls_url) == 0 || json_is_string(stable_ver) == 0 ||
        json_is_array(versions) == 0)
    {
      ret_data->result = 2;
      goto out;
    }

    for (int i = 0; i < json_array_size(versions); i++)
    {
      json_t *version_obj = json_array_get(versions, i);

      if (json_is_object(version_obj))
      {
        json_t *version = json_object_get(version_obj, "version");
        json_t *des = json_object_get(version_obj, "description");
        json_t *author = json_object_get(version_obj, "author");
        json_t *license = json_object_get(version_obj, "license");
        json_t *checksum = json_object_get(version_obj, "checksum");

        if (json_is_string(version) == 0 || json_is_string(des) == 0 ||
            json_is_string(author) == 0 || json_is_string(license) == 0 ||
            json_is_string(checksum) == 0)
        {
          ret_data->result = 2;
          goto out;
        }

        if (strcmp(json_string_value(version), (pkg_version == NULL) ?
        json_string_value(stable_ver) : pkg_version)  == 0)
        {
          ret_data->rls_url = strdup(json_string_value(rls_url));
          ret_data->version = strdup(json_string_value(stable_ver));
          ret_data->des = strdup(json_string_value(des));
          ret_data->author = strdup(json_string_value(author));
          ret_data->license = strdup(json_string_value(license));
          ret_data->checksum = strdup(json_string_value(checksum));

          ret_data->result = 0;
          goto out;
        }
      }
    }
  }
out:
  json_decref(root);
  return ret_data->result;
}