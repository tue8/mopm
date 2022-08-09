#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <errno.h>

#include <curl/curl.h>
#include <sha256.h>
#include <toml.h>

#define MOPM_VERSION "0.2.0"


#define STRING_MAX_LEN 255

#define START_LOADER(_switch, _func) CreateThread(NULL, 0, _func, &_switch, 0, NULL);

#define STOP_LOADER(_switch, _exit_code)\
{\
    _switch = -1;\
    while (_switch == -1);\
    if (_exit_code == -1) printf("\bfailed\n");\
    else printf("\bdone\n");\
}

typedef struct
{
    char *ptr;
    size_t len;
} GET_RES;

typedef struct
{
    toml_datum_t description;
    toml_datum_t author;
    toml_datum_t license;
    toml_datum_t checksum;
    toml_datum_t binary_url;
} toml_pkg;

static char *get_str_after_char(const char *_str, int _char)
{
    char *str = strdup(_str);
    char *last_char;

    strcpy(str, _str);

    last_char = strrchr(str, _char);
    if (last_char == NULL)
    {
        free(str);
        return NULL;
    }
    last_char += 1;
    str += (int)(last_char - str);

    return str;
}

static char *get_str_before_char(const char *_str, int _char)
{
    char *str = strdup(_str);
    char *last_char;

    strcpy(str, _str);

    last_char = strrchr(str, _char);
    if (last_char == NULL)
    {
        free(str);
        return NULL;
    }
    *(str + (strlen(str) - strlen(last_char))) = '\0';

    return str;
}

static int file_size(FILE *file)
{
    int file_size = 1;
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    return file_size;
}

DWORD WINAPI loader_th(void *data)
{
    printf(" \\");
    
    while (1)
    {
        if (*(int*)data == -1) break;
        Sleep(250);
        printf("\b|");
        Sleep(250);
        printf("\b/");
        Sleep(250);
        printf("\b-");
        Sleep(250);
        printf("\b\\");
    }

    *(int*)data = 1;

    return 0;
}

static size_t write_file(void *data, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(data, size, nmemb, stream);
    return written;
}

static size_t receive_http_get_res(void *data, size_t size, size_t nmemb, GET_RES *res)
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

static int send_http_get_res(CURL *curl_handle, char *url, GET_RES *http_get_res)
{
    CURLcode perform_res;
    curl_off_t size;

    curl_easy_reset(curl_handle);

    http_get_res->len = 0;
    http_get_res->ptr = malloc(http_get_res->len + 1);

    if (http_get_res->ptr == NULL)
    {
        perror("Could not allocate memory");
        return 1;
    }

    http_get_res->ptr[0] = '\0';

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "mopm package manager");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, receive_http_get_res);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, http_get_res);

    perform_res = curl_easy_perform(curl_handle);

    curl_easy_reset(curl_handle);
    return 0;
}

static int send_download_to_file_req(CURL *curl_handle, char *url, char *file_dir)
{
    FILE *file_handler;
    CURLcode perform_res;

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
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

static char *get_checksum_sha256(const char *filename)
{
    FILE *f;
    int i, j, k = 0;
    sha256_context ctx;
    unsigned char buf[1000];
    unsigned char sha256sum[32];
    char *hex_sha256sum = malloc(65);

    if ((f = fopen(filename, "rb")) == NULL)
    {
        return NULL;
    }

    sha256_starts(&ctx);

    while ((i = fread(buf, 1, sizeof(buf), f)) > 0)
    {
        sha256_update(&ctx, buf, i);
    }

    sha256_finish(&ctx, sha256sum);

    for (j = 0; j < 32; j++)
    {
        sprintf((char *)(hex_sha256sum + k), "%02x", sha256sum[j]);
        k += 2;
    }

    *(hex_sha256sum + 65) = '\0';

    fclose(f);
    return hex_sha256sum;
}

static int verify_package_checksum(const char *vctrl_file_dir, const char *bin_file_dir, const char *input_argv, const char *checksum)
{
    char line[133];
    char *vctrl_bin_file_checksum = NULL;
    FILE *vctrl_file = fopen(vctrl_file_dir, "r");
    char input_argv_n[134];

    if (vctrl_file == NULL)
    {
        fclose(vctrl_file);
        return 0;
    }

    snprintf(input_argv_n, sizeof(input_argv_n), "%s\n", input_argv);

    while (fgets(line, sizeof(line), vctrl_file))
    {
        if (strcmp(line, input_argv) == 0
         || strcmp(line, input_argv_n) == 0)
        {
            vctrl_bin_file_checksum = get_checksum_sha256(bin_file_dir);

            if (
                vctrl_bin_file_checksum != NULL &&
                strcmp(vctrl_bin_file_checksum, checksum) == 0)
            {
                fclose(vctrl_file);
                return 1;
            }
        }
    }

    free(vctrl_bin_file_checksum);
    fclose(vctrl_file);
    return 0;
}

static int find_package(CURL *curl_handle, const char* input_pkg_name, const char* input_pkg_version,
                          const char *vctrl_dir, const char *pkg_download_dir, const char *input_argv,
                          toml_pkg *toml_vars_buffer, char *find_package_err_buffer)
{
    toml_table_t *toml_parser;

    char toml_err_buffer[STRING_MAX_LEN];
    char toml_manifest_url[STRING_MAX_LEN];
    GET_RES toml_manifest_raw;
    int result;

    int loader_end_switch;

    printf("Searching for package...");
    START_LOADER(loader_end_switch, loader_th);
    snprintf(toml_manifest_url, sizeof(toml_manifest_url), "https://raw.githubusercontent.com/Localtings/mopm-pkgs/main/packages/%s/%s/manifest.toml", input_pkg_name, input_pkg_version);

    if (send_http_get_res(curl_handle, toml_manifest_url, &toml_manifest_raw) != CURLE_OK)
    {
        STOP_LOADER(loader_end_switch, -1);
        strcpy(find_package_err_buffer, "Could not send http request");
        return 1;
    }

    if (strcmp(toml_manifest_raw.ptr, "404: Not Found") == 0)
    {
        STOP_LOADER(loader_end_switch, -1);
        strcpy(find_package_err_buffer, "Package not found");
        return 1;
    }

    toml_parser = toml_parse(toml_manifest_raw.ptr, toml_err_buffer, sizeof(toml_err_buffer));
    free(toml_manifest_raw.ptr);

    if (toml_parser == 0)
    {
        STOP_LOADER(loader_end_switch, -1);
        snprintf(find_package_err_buffer, sizeof(find_package_err_buffer), "Cannot parse manifest: %s", toml_err_buffer);
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
        STOP_LOADER(loader_end_switch, -1);
        strcpy(find_package_err_buffer, "Invalid manifest");
        return 1;
    }

    if (verify_package_checksum(vctrl_dir, pkg_download_dir, input_argv, toml_vars_buffer->checksum.u.s) == 1)
    {
        STOP_LOADER(loader_end_switch, 1);
        strcpy(find_package_err_buffer, "Package is already installed");
        return 1;
    }
    

    STOP_LOADER(loader_end_switch, 1);
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
        goto exit;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle == NULL)
    {
        printf("Could not initialize curl\n");
        goto exit_all;
    }

    if (appdata == NULL)
    {
        printf("Could not find %%APPDATA%%\n");
        goto exit_all;
    }

    snprintf(mopm_dir, sizeof(mopm_dir), "%s\\mopm", appdata);

    if (CreateDirectory(mopm_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Could not create mopm folder\n");
        goto exit_all;
    }

    snprintf(bin_dir, sizeof(bin_dir), "%s\\bin", mopm_dir);

    if (CreateDirectory(bin_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Could not create packages folder\n");
        goto exit_all;
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
        char vctrl_dir[STRING_MAX_LEN];
        char vctrl_dir_clone[STRING_MAX_LEN];
        char pkg_download_dir[STRING_MAX_LEN];

        char *input_pkg_name;
        char *input_pkg_version;

        toml_pkg toml_pkg_vars;

        int find_package_res = 0;
        char find_package_err_buffer[STRING_MAX_LEN];

        FILE *vctrl_file;
        int vctrl_file_size;
        FILE *vctrl_file_clone;
        DWORD vctrl_file_clone_attr;
        char vctrl_line[133];

        char argv_n[134];

        if (argc < 3)
        {
            printf("Too few arguments provided\n");
            goto exit;
        }

        input_pkg_name = get_str_before_char(argv[2], '@');
        input_pkg_version = get_str_after_char(argv[2], '@');

        if (strlen(input_pkg_name) > 100)
        {
            printf("Invalid package name");
            goto exit;
        }

        if (strlen(input_pkg_version) > 32)
        {
            printf("Invalid package version");
            goto exit;
        }

        snprintf(pkg_download_dir, sizeof(pkg_download_dir), "%s\\%s_v%s.exe", bin_dir, input_pkg_name, input_pkg_version);
        
        snprintf(vctrl_dir, sizeof(vctrl_dir), "%s\\.vctrl", mopm_dir);

        strcpy(vctrl_dir_clone, vctrl_dir);
        vctrl_dir_clone[strlen(vctrl_dir) - strlen(".vctrl")] = '\0';
        strcat(vctrl_dir_clone, ".vctrl.clone");

        vctrl_file = fopen(vctrl_dir, "r");
        if (vctrl_file == NULL && strcmp(argv[1], "install") == 0)
        {
            vctrl_file = fopen(vctrl_dir, "w");
        }

        if (vctrl_file == NULL)
        {
            perror("Could not open .vctrl");
            goto exit_failure;
        }

        vctrl_file_clone = fopen(vctrl_dir_clone, "w");
        if (vctrl_file_clone == NULL)
        {
            perror("Could not create .vctrl.clone");
            goto exit_failure;
        }

        vctrl_file_clone_attr = GetFileAttributes(vctrl_dir_clone);
        if ((vctrl_file_clone_attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
            SetFileAttributes(vctrl_dir_clone, vctrl_file_clone_attr | FILE_ATTRIBUTE_HIDDEN);
        }

        snprintf(argv_n, sizeof(argv_n), "%s\n", argv[2]);


        if (strcmp(argv[1], "install") == 0)
        {    
            CURLcode pkg_download_res;
            int loader_end_switch;
            int argv_exist = 0;

            find_package_res = find_package(curl_handle, input_pkg_name, input_pkg_version, vctrl_dir, 
                                            pkg_download_dir, argv[2], &toml_pkg_vars, find_package_err_buffer);
            
            if (find_package_res == 1)
            {
                printf("Could not find package: %s\n", find_package_err_buffer);
                goto exit_failure;
            }
            
            printf("\nFound: %s Version %s\n"
                   "Description: %s\n"
                   "Licensed under %s license\n"
                   "Author: %s\n", 
            input_pkg_name, input_pkg_version, toml_pkg_vars.description.u.s, 
            toml_pkg_vars.license.u.s, toml_pkg_vars.author.u.s);

            printf("\nDownloading package file...");
            START_LOADER(loader_end_switch, loader_th);

            pkg_download_res = send_download_to_file_req(curl_handle, toml_pkg_vars.binary_url.u.s, pkg_download_dir);

            if (pkg_download_res != CURLE_OK)
            {
                STOP_LOADER(loader_end_switch, -1);
                printf("Could not download package\n");

                if (remove(pkg_download_dir) != 0)
                {
                    perror("Could not remove package binary");
                }

                goto exit_failure;
            }

            STOP_LOADER(loader_end_switch, 1);
            printf("Successfully downloaded binary file\n");

            if (file_size(vctrl_file) == 0)
            {
                fputs(argv[2], vctrl_file_clone);
                goto install_success;
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
                fprintf(vctrl_file_clone, "\n%s", argv[2]);
            }
            
        install_success:
            printf("Successfully installed package.\n");
            goto exit_success;
        }
        else if (strcmp(argv[1], "uninstall") == 0)
        {
            FILE *pkg_binary;

            find_package_res = find_package(curl_handle, input_pkg_name, input_pkg_version, vctrl_dir, 
                                            pkg_download_dir, argv[2], &toml_pkg_vars, find_package_err_buffer);
            
            if (find_package_res == 1 && strcmp(find_package_err_buffer, "Package is already installed") != 0)
            {
                printf("Could not find package: %s\n", find_package_err_buffer);
                goto exit_failure;
            }

            if (file_size(vctrl_file) == 0)
            {
                printf(".vctrl is empty\n");
                goto exit_failure;
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
                goto exit_failure;
            }

        uninstall_success:
            printf("Successfully uninstalled package\n");
            goto exit_success;
        }

    exit_failure:
        fclose(vctrl_file);
        fclose(vctrl_file_clone);

        if (remove(vctrl_dir_clone) != 0)
        {
            perror("Could not remove .vctrl.clone");
        }

        goto exit;
    exit_success:
        fclose(vctrl_file);
        fclose(vctrl_file_clone);

        if (remove(vctrl_dir) != 0)
        {
            perror("Could not remove .vctrl");
        }

        if (rename(vctrl_dir_clone, vctrl_dir) != 0)
        {
            perror("Could not rename .vctrl.clone");
        }

        SetFileAttributes(vctrl_dir, vctrl_file_clone_attr);

        goto exit;
    exit:
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
exit_all:
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return 0;
}