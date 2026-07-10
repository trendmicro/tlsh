//////////////////////////////////////////////////////////////////////////
//
// Benchmark: TLSH vs SHA256 throughput on large data
//
// Usage:
//   tlsh_sha256_benchmark [-size <bytes>] [-f <path>] [-chunk <bytes>]
//
//   -size <bytes>   allocate synthetic buffer of this size (default: 100 MB)
//   -f <path>       read file from disk into memory, then hash
//   -chunk <bytes>  chunk size for streaming (default: 4 MB)
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/evp.h>

#include "tlsh.h"

static long long now_ns()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

// Tlsh::update() hardcodes tlsh_option=0, so PRIVATE is only accessible via
// the batch final(data, len, option) path. Use batch for both variants so the
// comparison is apples-to-apples.
static long long bench_tlsh(const unsigned char *data, size_t size, int option)
{
    long long t0 = now_ns();
    Tlsh th;
    th.final(data, (unsigned int)size, option);
    long long t1 = now_ns();
    return t1 - t0;
}

static long long bench_sha256(const unsigned char *data, size_t size, size_t chunk_size)
{
    long long t0 = now_ns();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    for (size_t offset = 0; offset < size; offset += chunk_size) {
        size_t n = (offset + chunk_size <= size) ? chunk_size : (size - offset);
        EVP_DigestUpdate(ctx, data + offset, n);
    }
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);
    long long t1 = now_ns();
    return t1 - t0;
}

static void print_row(const char *algo, size_t size_bytes, long long ns)
{
    double ms         = ns / 1e6;
    double mb         = size_bytes / (1024.0 * 1024.0);
    double throughput = mb / (ns / 1e9);
    printf("%-16s %12.1f ms   %14.1f MB/s\n", algo, ms, throughput);
}

int main(int argc, char *argv[])
{
    size_t      data_size  = 0;
    size_t      chunk_size = 4 * 1024 * 1024;
    const char *filename   = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-size") == 0 && i + 1 < argc) {
            data_size = (size_t)atoll(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "-chunk") == 0 && i + 1 < argc) {
            chunk_size = (size_t)atoll(argv[++i]);
        } else {
            fprintf(stderr, "Usage: %s [-size <bytes>] [-f <path>] [-chunk <bytes>]\n", argv[0]);
            return 1;
        }
    }

    unsigned char *data = NULL;

    if (filename) {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            fprintf(stderr, "error: cannot open '%s'\n", filename);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        data_size = (size_t)ftell(fp);
        rewind(fp);
        data = (unsigned char *)malloc(data_size);
        if (!data) {
            fprintf(stderr, "error: out of memory (%zu bytes)\n", data_size);
            fclose(fp);
            return 1;
        }
        if (fread(data, 1, data_size, fp) != data_size) {
            fprintf(stderr, "error: read failed\n");
            free(data);
            fclose(fp);
            return 1;
        }
        fclose(fp);
        printf("Source:     file '%s'\n", filename);
    } else {
        if (data_size == 0)
            data_size = 100ULL * 1024 * 1024;
        data = (unsigned char *)malloc(data_size);
        if (!data) {
            fprintf(stderr, "error: out of memory (%zu bytes)\n", data_size);
            return 1;
        }
        for (size_t i = 0; i < data_size; i++)
            data[i] = (unsigned char)(i % 26 + 'A');
        printf("Source:     synthetic (cycling A-Z pattern)\n");
    }

    printf("Data size:  %.1f MB (%zu bytes)\n", data_size / (1024.0 * 1024.0), data_size);
    printf("SHA256 chunk: %.1f MB (%zu bytes)\n", chunk_size / (1024.0 * 1024.0), chunk_size);
    printf("TLSH mode:  batch (required for private option)\n");
    printf("\n");

    // warm-up pass
    bench_tlsh(data, data_size, 0);
    bench_tlsh(data, data_size, TLSH_OPTION_PRIVATE);
    bench_tlsh(data, data_size, TLSH_OPTION_THREADED);
    bench_tlsh(data, data_size, TLSH_OPTION_THREADED4);
    bench_sha256(data, data_size, chunk_size);

    // measured pass
    long long tlsh_ns           = bench_tlsh(data, data_size, 0);
    long long tlsh_private_ns   = bench_tlsh(data, data_size, TLSH_OPTION_PRIVATE);
    long long tlsh_threaded_ns  = bench_tlsh(data, data_size, TLSH_OPTION_THREADED);
    long long tlsh_threaded4_ns = bench_tlsh(data, data_size, TLSH_OPTION_THREADED4);
    long long sha256_ns         = bench_sha256(data, data_size, chunk_size);

    printf("%-16s %14s   %16s\n", "Algorithm", "Time (ms)", "Throughput (MB/s)");
    printf("%-16s %14s   %16s\n", "---------", "---------", "----------------");
    print_row("TLSH",           data_size, tlsh_ns);
    print_row("TLSH-private",   data_size, tlsh_private_ns);
    print_row("TLSH-threaded",  data_size, tlsh_threaded_ns);
    print_row("TLSH-4thread",   data_size, tlsh_threaded4_ns);
    print_row("SHA256",         data_size, sha256_ns);
    printf("\n");

    free(data);
    return 0;
}
