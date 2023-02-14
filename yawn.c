#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <glob.h>

#include "archive.h"
#include "fetch.h"

#ifndef PROGVER
#define PROGVER     "0.1"
#endif

/* Global variables */
static char name[100];
static char url[100];
static char desc[500];
static char author[100];

static void show_help(void)
{
    fprintf(stdout,
        "yawn - v%s\n\n"
        "Usage:\n"
        "   -c [src] [dst]      -- Compress the file with zlib compression\n"
        "   -d [src] [dst]      -- Decompress the file with zlib compression\n"
        "   -i [template]       -- Template file to install the program\n"
        "   -u [template]       -- Template file to uninstall the program\n"
        "   -p [template]       -- Show information about a template\n"
        , PROGVER
    );
}

static void fill_clean(char *target)
{
    memset(target, '\0', sizeof(*target));
}

static void read_and_print(char *file)
{
    FILE *fp;
    char buf[4096];
    size_t size;
    long pos;
    size_t i, k;

    fill_clean(name);
    fill_clean(url);
    fill_clean(desc);
    fill_clean(author);
    fill_clean(buf);

    fp = fopen(file, "rb");
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    rewind(fp);

    size = fread(buf, 1L, pos, fp);
    fclose(fp);

    for (i = 0; i < size; i++) {
        if (buf[i] == 'N' && buf[i + 1] == 'A' &&
            buf[i + 2] == 'M' && buf[i + 3] == 'E') {

            for (k = 0; k < 4; k++)
                buf[i + k] = '\0';

            if (buf[i + 4] == '=') {
                while (buf[i + 4] != '\n') {
                    strncat(name, &buf[i + 5], 1);
                    i++;
                }
            }
        } else if (buf[i] == 'U' && buf[i + 1] == 'R' &&
            buf[i + 2] == 'L') {

            for (k = 0; k < 3; k++)
                buf[i + k] = '\0';

            if (buf[i + 3] == '=') {
                while (buf[i + 3] != '\n') {
                    strncat(url, &buf[i + 4], 1);
                    i++;
                }
            }
        } else if (buf[i] == 'D' && buf[i + 1] == 'E' &&
            buf[i + 2] == 'S' && buf[i + 3] == 'C') {

            for (k = 0; k < 4; k++)
                buf[i + k] = '\0';

            if (buf[i + 4] == '=') {
                while (buf[i + 4] != '\n') {
                    strncat(desc, &buf[i + 5], 1);
                    i++;
                }
            }
        } else if (buf[i] == 'A' && buf[i + 1] == 'U' &&
            buf[i + 2] == 'T' && buf[i + 3] == 'H' &&
            buf[i + 4] == 'O' && buf[i + 5] == 'R') {

            for (k = 0; k < 6; k++)
                buf[i + k] = '\0';

            if (buf[i + 6] == '=') {
                while (buf[i + 6] != '\n') {
                    strncat(author, &buf[i + 7], 1);
                    i++;
                }
            }
        }
    }
}

static int is_zlib_magic(char *file)
{
    unsigned char buf[1024];
    FILE *fp;

    fp = fopen(file, "rb");
    fread(buf, 1L, sizeof(buf), fp);
    fclose(fp);

    /**
     * This check is inefficient. One can pass this check by passing falsey
     * data by remapping this magic header.
    */
    if (buf[0] == 120 && buf[1] == 218)
        return 0;
    else
        return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        show_help();
        exit(EXIT_FAILURE);
    }

    int opt;
    char ftmp[103], path[206];
    FILE *src, *dst;
    struct stat st;

    fill_clean(ftmp);
    fill_clean(path);

    while ((opt = getopt(argc, argv, "c:i:uhp:d:")) != -1) {
        switch (opt) {
        /** Argument: "i" for installation */
        case 'i':
            if (getuid() != 0) {
                fprintf(stderr,
                    "You're now a shooter but not (yet) a root user.\n"
                    "You need to be the root user to execute yawn.\n"
                );
                exit(EXIT_FAILURE);
            }
            read_and_print(argv[2]);
            
            if (strlen(name) == 0) {
                fprintf(stderr, "Error: File name is missing in the template file\n");
                exit(EXIT_FAILURE);
            } else if (strlen(url) == 0) {
                fprintf(stderr, "Error: File URL is missing in the template file\n");
                exit(EXIT_FAILURE);
            }

            name[strcspn(name, "\n")] = '\0';
            sprintf(ftmp, "%s.zl", name);

            url[strcspn(url, "\n")] = '\0';

            if (url != "NO")
                yawn_get_file(url, ftmp); /* For test, you can comment out yawn_get_file */

            src = fopen(ftmp, "rb");
            dst = fopen(name, "wb");

            if (src == NULL || dst == NULL) {
                perror("fopen()");
                remove(argv[3]); /* Ignore checks */
                exit(EXIT_FAILURE);
            }

            /**
             * Note: This isn't any efficient, check can be easily bypass, if the input data
             * has the same magic header as here.
            */
            if (is_zlib_magic(ftmp) != 0) {
                fprintf(stderr, "Error: Unknown file format\n");
                exit(EXIT_FAILURE);
            }

            yawn_decompress(src, dst);

            sprintf(path, "/opt/%s", name);
            if (mkdir(path, 0755) != 0) {
                perror("mkdir()");
                remove(name); /* Ignore checks */
                exit(EXIT_FAILURE);
            }

            fill_clean(path);

            sprintf(path, "/opt/%s/%s", name, name);
            if (rename(name, path) != 0) {
                perror("rename()");
                exit(EXIT_FAILURE);
            }

            fprintf(stdout, "Installed!\n");
            fclose(src);
            fclose(dst);
           break;

        /** Argument: "u" for uninstallation */
        case 'u':
            read_and_print(argv[2]);

            glob_t globbuf;
            globbuf.gl_offs = 0;
            size_t i;
            int ret, skip = 0;
            char globby[200];

            fill_clean(globby);

            strcat(ftmp, "/opt/");
            /* Remove trailing newline from name */
            name[strcspn(name, "\n")] = '\0';

            strcat(ftmp, name);
            strcat(globby, ftmp);
            strcat(globby, "/*");

            ret = glob(globby, GLOB_DOOFFS, NULL, &globbuf);

            switch (ret) {
            case GLOB_ABORTED:
            case GLOB_NOMATCH:
                fprintf(stderr,
                    "glob() error: %s", ret == GLOB_ABORTED ?
                    "glob_aborted\n" :
                    "glob_nomatch\n"
                );
                exit(EXIT_FAILURE);
            }

            for (i = 0; i < globbuf.gl_pathc; i++) {
                stat(globbuf.gl_pathv[i], &st);

                if (S_ISDIR(st.st_mode)) {
                    fprintf(stderr, "Warning: %s is a directory, skipping...\n", globbuf.gl_pathv[i]);
                    skip = 1;
                }
                remove(globbuf.gl_pathv[i]);
            }

            if (skip == 1) {
                fprintf(stderr, "Warning: Directory %s contains another directory. Cannot delete, skipping...\n", ftmp);
            } else {
                rmdir(ftmp);
                fprintf(stdout, "Uninstalled!\n");
            }

            globfree(&globbuf);
            break;

        /** Argument: "c" to compress a file with zlib compression */
        case 'c':
            if (argc < 4) {
                fprintf(stderr, "Error: Source and destination is missing\n");
                exit(EXIT_FAILURE);
            }

            char out[100];
            fill_clean(out);
            strcat(out, argv[3]);
            strcat(out, ".zl");

            src = fopen(argv[2], "rb");
            dst = fopen(out, "wb");

            if (src == NULL || dst == NULL) {
                perror("fopen()");
                remove(argv[3]); /* Ignore checks */
                exit(EXIT_FAILURE);
            }

            yawn_compress(src, dst, Z_BEST_COMPRESSION);

            fclose(src);
            fclose(dst);
            break;

        /** Argument: "d" to decompress the zlib compressed archive */
        case 'd':
            if (argc < 4) {
                fprintf(stderr, "Error: Source and destination is missing\n");
                exit(EXIT_FAILURE);
            }

            src = fopen(argv[2], "rb");
            dst = fopen(argv[3], "wb");

            if (src == NULL || dst == NULL) {
                perror("fopen()");
                remove(argv[3]); /* Ignore checks */
                exit(EXIT_FAILURE);
            }

            yawn_decompress(src, dst);

            fclose(src);
            fclose(dst);
            break;

        /** Argument: "p" to print information about the package form the template */
        case 'p':
            read_and_print(argv[2]);
            /** Bullet mark from: https://www.alt-codes.net/bullet_alt_codes.php */
            fprintf(stdout,
                "• Program: %s"
                "• Description: %s"
                "• Author: %s"
                "• URL: %s",
                name, strlen(desc) == 0 ?
                    "Not specified\n" :
                    desc,
                strlen(author) == 0 ?
                    "Not specified\n" :
                    author,
                url
            );
            break;

        case 'h':
        default:
            show_help();
            break;
        }
    }
}
