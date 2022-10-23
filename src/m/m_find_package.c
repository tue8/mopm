#include "m_find_package.h"
#include "m_vctrl.h"
#include "m_checksum.h"
#include "m_string.h"
#include "m_curl.h"
#include <jansson.h>
#include <stdio.h>

static int get_binary_url(CURL *curl_handle, const char *rls_url,
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
  free(releases_json.ptr);

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

int find_package(CURL *curl_handle,
                        char **pkg, char *pkg_name, char *pkg_version,
                        struct vctrl *_vctrl,
                        char *bin_dir, char **bin_url)
{
  char *manifest_url;
  struct get_res manifest_raw;

  json_t *root;
  json_t *data;
  json_error_t err_buffer;

  int res = 0;

  printf("Searching for package...\n");
  asprintf(&manifest_url, "https://raw.githubusercontent.com/Localtings/"
                          "mopm-pkgs/main/packages/%s/manifest.json",
           pkg_name);

  if (send_http_get(curl_handle, manifest_url, &manifest_raw) != CURLE_OK)
  {
    res = 1;
    goto out;
  }

  free(manifest_url);

  if (strcmp(manifest_raw.ptr, "404: Not Found") == 0)
  {
    res = 1;
    free(manifest_raw.ptr);
    goto out;
  }

  root = json_loads(manifest_raw.ptr, 0, &err_buffer);
  free(manifest_raw.ptr);

  if (root == 0 || json_is_object(root) == 0)
  {
    res = 2;
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
      res = 2;
      goto out;
    }

    if (pkg_version == NULL)
    {
      free(*pkg);
      pkg_version = strdup(json_string_value(stable_ver));
      asprintf(pkg, "%s@%s", pkg_name, pkg_version);
    }

    for (int i = 0; i < json_array_size(versions); i++)
    {
      json_t *version_obj = json_array_get(versions, i);

      if (json_is_object(version_obj))
      {
        int checksum_result = 0;

        json_t *version = json_object_get(version_obj, "version");
        json_t *des = json_object_get(version_obj, "description");
        json_t *author = json_object_get(version_obj, "author");
        json_t *license = json_object_get(version_obj, "license");
        json_t *checksum = json_object_get(version_obj, "checksum");

        if (json_is_string(version) == 0 || json_is_string(des) == 0 ||
            json_is_string(author) == 0 || json_is_string(license) == 0 ||
            json_is_string(checksum) == 0)
        {
          res = 2;
          goto out;
        }

        if (strcmp(json_string_value(version), pkg_version) == 0)
        {
          printf("Found: %s Version %s\n"
               "Description: %s\n"
               "License: %s\n"
               "Author: %s\n",
               pkg_name, pkg_version,
               json_string_value(des),
               json_string_value(license),
               json_string_value(author));

          checksum_result = verify_checksum(_vctrl, bin_dir, *pkg, json_string_value(checksum));

          if (checksum_result == 1)
          {
            res = 3;
            goto out;
          }

          if (get_binary_url(curl_handle, json_string_value(rls_url), pkg_version, bin_url) == 1)
            res = 4;

          goto out;
        }
      }
    }
    res = 1;
  }
out:
  if (res == 1)
    fprintf(stderr, "Package could not be found\n");
  else if (res == 3)
    fprintf(stderr, "Package is already installed\n");

  json_decref(root);
  return res;
}