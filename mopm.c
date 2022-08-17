#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <windows.h>

#include "m_string.h"
#include "m_curl.h"
#include "m_checksum.h"
#include "m_loader.h"

#include <toml.h>

#define MOPM_VERSION "0.2.4"

typedef struct
{
    toml_datum_t description;
    toml_datum_t author;
    toml_datum_t license;
    toml_datum_t checksum;
    toml_datum_t binary_url;
} toml_pkg;

static int file_size(FILE *file)
{
    int file_size;
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    return file_size;
}

static int find_package(CURL *curl_handle, const char* input_pkg_name, const char* input_pkg_version,
                          const char *vctrl_dir, const char *pkg_download_dir, const char *input_argv,
                          toml_pkg *toml_vars_buffer)
{
    toml_table_t *toml_parser;

    char toml_err_buffer[STRING_MAX_LEN];
    char toml_manifest_url[STRING_MAX_LEN];
    GET_RES toml_manifest_raw;
    int result;

    int loader_end_switch;

    printf("Searching for package...");
    start_loader(&loader_end_switch);
    snprintf(toml_manifest_url, sizeof(toml_manifest_url), "https://raw.githubusercontent.com/Localtings/mopm-pkgs/main/packages/%s/%s/manifest.toml", input_pkg_name, input_pkg_version);

    if (send_http_get(curl_handle, toml_manifest_url, &toml_manifest_raw) != CURLE_OK)
    {
        stop_loader(&loader_end_switch, -1);
        printf("Package could not be found - Could not send http request\n");
        return 1;
    }

    if (strcmp(toml_manifest_raw.ptr, "404: Not Found") == 0)
    {
        stop_loader(&loader_end_switch, -1);
        printf("Package could not be found\n");
        return 1;
    }

    toml_parser = toml_parse(toml_manifest_raw.ptr, toml_err_buffer, sizeof(toml_err_buffer));
    free(toml_manifest_raw.ptr);

    if (toml_parser == 0)
    {
        stop_loader(&loader_end_switch, -1);
        printf("Package could not be found - Cannot parse manifest: %s\n", toml_err_buffer);
        return 1;
    }

    toml_vars_buffer->description = toml_string_in(toml_parser, "description");
    toml_vars_buffer->author = toml_string_in(toml_parser, "author");
    toml_vars_buffer->license = toml_string_in(toml_parser, "license");
    toml_vars_buffer->checksum = toml_string_in(toml_parser, "checksum");
    toml_vars_buffer->binary_url = toml_string_in(toml_parser, "binary_url");

    if (toml_vars_buffer->description.ok == 0 ||
        toml_vars_buffer->author.ok == 0 ||
        toml_vars_buffer->license.ok == 0 ||
        toml_vars_buffer->checksum.ok == 0 ||
        toml_vars_buffer->binary_url.ok == 0)
    {
        stop_loader(&loader_end_switch, -1);
        printf("Package could not be found - Invalid manifest\n");
        return 1;
    }

    if (verify_package_checksum(vctrl_dir, pkg_download_dir, input_argv, toml_vars_buffer->checksum.u.s) == 1)
    {
        stop_loader(&loader_end_switch, 1);
        printf("Package is already installed\n");
        return -1;
    }
    
    stop_loader(&loader_end_switch, 1);
    return 0;
}

int main(int argc, char *argv[])
{
    CURL *curl_handle;

    char *appdata = getenv("APPDATA");

    char mopm_dir[STRING_MAX_LEN];
    char bin_dir[STRING_MAX_LEN];

    if (argc < 2)
    {
        printf("Too few arguments provided\n");
        goto out;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle == NULL)
    {
        printf("Could not initialize curl\n");
        goto curl_out;
    }

    if (appdata == NULL)
    {
        printf("Could not find %%APPDATA%%\n");
        goto curl_out;
    }

    snprintf(mopm_dir, sizeof(mopm_dir), "%s\\mopm", appdata);

    if (CreateDirectory(mopm_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Could not create mopm folder\n");
        goto curl_out;
    }

    snprintf(bin_dir, sizeof(bin_dir), "%s\\bin", mopm_dir);

    if (CreateDirectory(bin_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Could not create packages folder\n");
        goto curl_out;
    }

    if (strcmp(argv[1], "--help") == 0)
    {
        printf("mopm Package Manager Version %s\n"
               "A package manager for Windows that installs cli apps onto your pc\n"
               "\nUsage: mopm <command> <argument>\n"
               "\nAvailable commands: --help\n"
               "                    install\n"
               "                    uninstall\n",
               MOPM_VERSION);
    }
    else if (strcmp(argv[1], "install") == 0 ||
             strcmp(argv[1], "uninstall") == 0)
    {
        int opt_install = 0;
        int out_success = 0;

        char vctrl_dir[STRING_MAX_LEN];
        char vctrl_dir_clone[STRING_MAX_LEN];
        char pkg_download_dir[STRING_MAX_LEN];

        char *input_pkg_name;
        char *input_pkg_version;

        toml_pkg toml_pkg_vars;

        int find_package_res = 0;

        FILE *vctrl_file;
        int vctrl_file_size;
        FILE *vctrl_file_clone;
        DWORD vctrl_file_clone_attr;
        char vctrl_line[133];

        char argv_n[134];

        if (argc < 3)
        {
            printf("Too few arguments provided\n");
            goto out;
        }

        input_pkg_name = get_str_before_char(argv[2], '@');
        input_pkg_version = get_str_after_char(argv[2], '@');

        if (strlen(input_pkg_name) > 100)
        {
            printf("Invalid package name");
            goto out;
        }

        if (strlen(input_pkg_version) > 32)
        {
            printf("Invalid package version");
            goto out;
        }

        if (strcmp(argv[1], "install") == 0) opt_install = 1;

        snprintf(pkg_download_dir, sizeof(pkg_download_dir), "%s\\%s_v%s.exe", bin_dir, input_pkg_name, input_pkg_version);
        
        snprintf(vctrl_dir, sizeof(vctrl_dir), "%s\\.vctrl", mopm_dir);

        strcpy(vctrl_dir_clone, vctrl_dir);
        vctrl_dir_clone[strlen(vctrl_dir) - strlen(".vctrl")] = '\0';
        strcat(vctrl_dir_clone, ".vctrl.clone");

        vctrl_file = fopen(vctrl_dir, "r");
        if (vctrl_file == NULL && opt_install)
        {
            vctrl_file = fopen(vctrl_dir, "w");
        }

        if (vctrl_file == NULL)
        {
            perror("Could not open .vctrl");
            goto out;
        }

        vctrl_file_clone = fopen(vctrl_dir_clone, "w");
        if (vctrl_file_clone == NULL)
        {
            perror("Could not create .vctrl.clone");
            goto out;
        }

        vctrl_file_clone_attr = GetFileAttributes(vctrl_dir_clone);
        if ((vctrl_file_clone_attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
            SetFileAttributes(vctrl_dir_clone, vctrl_file_clone_attr | FILE_ATTRIBUTE_HIDDEN);
        }

        snprintf(argv_n, sizeof(argv_n), "%s\n", argv[2]);

        if (opt_install)
        {    
            CURLcode pkg_download_res;
            int loader_end_switch;
            int argv_exist = 0;
            
            if (find_package(curl_handle, input_pkg_name, input_pkg_version, vctrl_dir, 
                             pkg_download_dir, argv[2], &toml_pkg_vars) != 0)
                             goto out;
            
            printf("\nFound: %s Version %s\n"
                   "Description: %s\n"
                   "Licensed under %s license\n"
                   "Author: %s\n", 
            input_pkg_name, input_pkg_version, toml_pkg_vars.description.u.s, 
            toml_pkg_vars.license.u.s, toml_pkg_vars.author.u.s);

            printf("\nDownloading package file...");
            start_loader(&loader_end_switch);

            pkg_download_res = send_download_to_file(curl_handle, toml_pkg_vars.binary_url.u.s, pkg_download_dir);

            if (pkg_download_res != CURLE_OK)
            {
                stop_loader(&loader_end_switch, -1);
                printf("Could not download package\n");

                if (remove(pkg_download_dir) != 0)
                {
                    perror("Could not remove package binary");
                }

                goto out;
            }

            stop_loader(&loader_end_switch, 1);
            printf("Successfully downloaded binary file\n");

            if (file_size(vctrl_file) == 0)
            {
                fprintf(vctrl_file_clone, argv[2]);
                out_success = 1;
                goto out;
            }

            while (fgets(vctrl_line, sizeof(vctrl_line), vctrl_file))
            {
                if (strcmp(vctrl_line, argv_n) == 0 
                 || strcmp(vctrl_line, argv[2]) == 0)
                {
                    argv_exist = 1;
                }

                fputs(vctrl_line, vctrl_file_clone);
            }

            if (argv_exist == 0)
            {
                int file_length;

                fseek(vctrl_file, 0L, SEEK_END);

                file_length = ftell(vctrl_file);

                fseek(vctrl_file, file_length - 1, SEEK_SET);

                if (fgetc(vctrl_file) == '\n')
                {
                    fprintf(vctrl_file_clone, argv[2]);
                }
                else
                {
                    fprintf(vctrl_file_clone, "\n%s", argv[2]);
                }
            }

            out_success = 1;
            goto out;
        }
        else
        {
            FILE *pkg_binary;

            find_package_res = find_package(curl_handle, input_pkg_name, input_pkg_version, vctrl_dir, 
                                            pkg_download_dir, argv[2], &toml_pkg_vars);
            
            if (find_package_res == 1) goto out;

            if (file_size(vctrl_file) == 0)
            {
                printf(".vctrl is empty\n");
                goto out;
            }

            while (fgets(vctrl_line, sizeof(vctrl_line), vctrl_file))
            {
                if (strcmp(vctrl_line, argv[2]) != 0 && strcmp(vctrl_line, argv_n) != 0)
                {
                    fputs(vctrl_line, vctrl_file_clone);
                }
            }

            if (remove(pkg_download_dir) != 0)
            {
                perror("Could not remove package's binary file");
                goto out;
            }

            out_success = 1;
            goto out;
        }

    out:
        fclose(vctrl_file);
        fclose(vctrl_file_clone);

        if (out_success)
        {
            if (remove(vctrl_dir) != 0)
            {
                perror("Could not remove .vctrl");
            }

            if (rename(vctrl_dir_clone, vctrl_dir) != 0)
            {
                perror("Could not rename .vctrl.clone");
            }

            SetFileAttributes(vctrl_dir, vctrl_file_clone_attr);

            if (opt_install)
            {
                printf("Successfully installed package.");
            }
            else
            {
                printf("Successfully uninstalled package.");
            }
        }
        else
        {
            if (remove(vctrl_dir_clone) != 0)
            {
                perror("Could not remove .vctrl.clone");
            }
        }

        if (toml_pkg_vars.author.u.s != NULL) free(toml_pkg_vars.author.u.s);
        if (toml_pkg_vars.description.u.s != NULL) free(toml_pkg_vars.description.u.s);
        if (toml_pkg_vars.binary_url.u.s != NULL) free(toml_pkg_vars.binary_url.u.s);
        if (toml_pkg_vars.checksum.u.s != NULL) free(toml_pkg_vars.checksum.u.s);
        if (toml_pkg_vars.license.u.s != NULL) free(toml_pkg_vars.license.u.s);

        if (input_pkg_name != NULL) free(input_pkg_name);
        if (input_pkg_version != NULL) free(input_pkg_version);
    }
    else
    {
        printf("Invalid command, see 'mopm --help'\n");
    }

curl_out:
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return 0;
}