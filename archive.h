#include <zlib.h>

#ifndef CHUNK
#define CHUNK   0x4000
#endif

static void yawn_compress(FILE *src, FILE *dst, int lvl)
{
    int flush;
    unsigned int have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    if (deflateInit(&strm, lvl) != Z_OK) {
        perror("deflateInit()");
        exit(EXIT_FAILURE);
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, src);
        if (ferror(src)) {
            deflateEnd(&strm);
            perror("fread()");
            exit(EXIT_FAILURE);
        }

        flush = feof(src) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            if (deflate(&strm, flush) == Z_STREAM_ERROR) {
                perror("deflate()");
                exit(EXIT_FAILURE);
            }

            have = CHUNK - strm.avail_out;

            if (fwrite(out, 1, have, dst) != have || ferror(dst)) {
                deflateEnd(&strm);
                perror("fwrite()");
                exit(EXIT_FAILURE);
            }
        } while (strm.avail_out == 0);

    } while (flush != Z_FINISH);

    deflateEnd(&strm);
}

static void yawn_decompress(FILE *src, FILE *dst)
{
    int ret;
    unsigned int have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = Z_NULL;
    strm.next_in = Z_NULL;

    if (inflateInit(&strm) != Z_OK) {
        perror("inflateInit()");
        exit(EXIT_FAILURE);
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, src);
        if (ferror(src)) {
            inflateEnd(&strm);
            perror("fread()");
            exit(EXIT_FAILURE);
        }
        if (strm.avail_in == 0)
            break;

        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                perror("inflate()");
                exit(EXIT_FAILURE);

            case Z_STREAM_ERROR:
                perror("inflate()");
                exit(EXIT_FAILURE);
            }

            have = CHUNK - strm.avail_out;

            if (fwrite(out, 1, have, dst) != have || ferror(dst)) {
                inflateEnd(&strm);
                perror("fwrite()");
                exit(EXIT_FAILURE);
            }

        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
}
