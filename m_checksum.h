#ifndef M_CHECKSUM_H_
#define M_CHECKSUM_H_

#include <sha256.h>

char *get_checksum_sha256(const char *filename)
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

int verify_package_checksum(const char *vctrl_file_dir, const char *bin_file_dir, const char *input_argv, const char *checksum)
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

#endif