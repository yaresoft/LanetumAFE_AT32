/*
 * fwsign.c - firmware image hasher, Linux port.
 *
 * Port of the Windows tool in /home/yiz/fwsigning/fwsign.c.  That version is
 * built against Windows CNG (bcrypt.dll for SHA-256 and ECDSA P-256); this one
 * uses OpenSSL and implements the subset the build needs:
 *
 *   fwsign <firmware.bin> -hash <offset> [-o <out>]
 *   fwsign verify-hash <firmware.bin> -hash <offset>
 *
 * The hash rule is taken from the original and matches what the AT32 boot
 * loader's fw_hash_verify() recomputes:
 *
 *   hash = SHA-256( file[0 .. off) || file[off+96 .. end) )
 *
 * i.e. everything except the 96-byte block at <off>, which holds the 32-byte
 * hash followed by a 64-byte signature slot.  The hash is written into that
 * 32-byte field; the signature slot is left alone.
 *
 * ECDSA signing (-key) is not implemented here - the original stores its
 * private key as a raw CNG blob, so porting it is a separate job.  Ask before
 * relying on it.
 *
 * Build:
 *   gcc -O2 -Wall -Wextra -std=gnu11 -o fwsign fwsign.c -lcrypto
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/sha.h>

#define HASH_SIZE 32
#define SIG_SIZE  64

static void usage(const char *prog)
{
    fprintf(stderr,
            "usage:\n"
            "  %s <firmware.bin> -hash <offset> [-o <out>]\n"
            "  %s verify-hash <firmware.bin> -hash <offset>\n"
            "\n"
            "  <offset> is where the 32-byte hash sits; the 64-byte signature\n"
            "  slot is assumed immediately after it, and both are excluded from\n"
            "  the hash.\n"
            "\n"
            "  Without -o the input is rewritten in place.\n",
            prog, prog);
}

static unsigned char *read_file(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    unsigned char *buf;
    long n;

    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", path);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot size %s\n", path);
        fclose(f);
        return NULL;
    }
    buf = malloc((size_t)n ? (size_t)n : 1U);
    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) {
        fprintf(stderr, "error: short read on %s\n", path);
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *len = (size_t)n;
    return buf;
}

static int write_file_atomic(const char *path, const unsigned char *data, size_t len)
{
    char tmp[1024];
    FILE *f;
    int n = snprintf(tmp, sizeof(tmp), "%s.tmp", path);

    if (n < 0 || (size_t)n >= sizeof(tmp)) {
        fprintf(stderr, "error: output path too long\n");
        return 0;
    }
    f = fopen(tmp, "wb");
    if (!f) {
        fprintf(stderr, "error: cannot write %s\n", tmp);
        return 0;
    }
    if (fwrite(data, 1, len, f) != len) {
        fprintf(stderr, "error: short write on %s\n", tmp);
        fclose(f);
        remove(tmp);
        return 0;
    }
    fclose(f);
    if (rename(tmp, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s\n", tmp, path);
        remove(tmp);
        return 0;
    }
    return 1;
}

/* SHA-256 over file[0..off) followed by file[off+96..len). */
static int image_hash(const unsigned char *data, size_t len, uint32_t off,
                      unsigned char out[HASH_SIZE])
{
    SHA256_CTX ctx;
    size_t skip_end = (size_t)off + HASH_SIZE + SIG_SIZE;

    if (skip_end > len) {
        fprintf(stderr, "error: -hash 0x%x + 96 exceeds file size %zu\n", off, len);
        return 0;
    }
    if (!SHA256_Init(&ctx)) {
        return 0;
    }
    if (off != 0U && !SHA256_Update(&ctx, data, off)) {
        return 0;
    }
    if ((len - skip_end) != 0U && !SHA256_Update(&ctx, data + skip_end, len - skip_end)) {
        return 0;
    }
    return SHA256_Final(out, &ctx) ? 1 : 0;
}

static void print_hex(const char *label, const unsigned char *b, size_t n)
{
    size_t i;

    printf("%s", label);
    for (i = 0; i < n; i++) {
        printf("%02x", b[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    const char *path = NULL;
    const char *out = NULL;
    uint32_t off = 0U;
    int have_off = 0;
    int verify = 0;
    unsigned char *data;
    unsigned char h[HASH_SIZE];
    size_t len = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "verify-hash") == 0) {
            verify = 1;
        } else if (strcmp(argv[i], "-hash") == 0) {
            if (++i >= argc) { usage(argv[0]); return 2; }
            off = (uint32_t)strtoul(argv[i], NULL, 0);
            have_off = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (++i >= argc) { usage(argv[0]); return 2; }
            out = argv[i];
        } else if (strcmp(argv[i], "-key") == 0) {
            fprintf(stderr, "error: ECDSA signing is only in the Windows build\n");
            return 2;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown option %s\n", argv[i]);
            usage(argv[0]);
            return 2;
        } else if (!path) {
            path = argv[i];
        } else {
            fprintf(stderr, "error: more than one input file\n");
            return 2;
        }
    }

    if (!path || !have_off) {
        usage(argv[0]);
        return 2;
    }

    data = read_file(path, &len);
    if (!data) {
        return 1;
    }

    if (!image_hash(data, len, off, h)) {
        free(data);
        return 1;
    }
    print_hex(verify ? "SHA-256 (file)    : " : "SHA-256 (fill)     : ", h, HASH_SIZE);

    if (verify) {
        if (memcmp(h, data + off, HASH_SIZE) != 0) {
            print_hex("SHA-256 (stored)  : ", data + off, HASH_SIZE);
            fprintf(stderr, "hash MISMATCH\n");
            free(data);
            return 1;
        }
        printf("hash OK\n");
        free(data);
        return 0;
    }

    memcpy(data + off, h, HASH_SIZE);
    if (!write_file_atomic(out ? out : path, data, len)) {
        free(data);
        return 1;
    }
    printf("Hash written       : %s\n", out ? out : path);
    free(data);
    return 0;
}
