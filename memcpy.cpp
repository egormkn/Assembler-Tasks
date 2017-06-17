#include <cstring>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <getopt.h>
#include "memcpy.h"

int test_memcpy(const char *mode, size_t size, size_t count) {
    int result = 0;
    if (strcmp(mode, "all") == 0) {
        result |= test_memcpy("1", size, count);
        if (size % 2 == 0)
            result |= test_memcpy("2", size, count);
        if (size % 4 == 0)
            result |= test_memcpy("4", size, count);
        if (size % 8 == 0)
            result |= test_memcpy("8", size, count);
        if (size % 16 == 0)
            result |= test_memcpy("16", size, count);
        result |= test_memcpy("1-asm", size, count);
        if (size % 2 == 0)
            result |= test_memcpy("2-asm", size, count);
        if (size % 4 == 0)
            result |= test_memcpy("4-asm", size, count);
        if (size % 8 == 0)
            result |= test_memcpy("8-asm", size, count);
        if (size % 16 == 0)
            result |= test_memcpy("nt", size, count);
        if (size % 16 == 0)
            result |= test_memcpy("nt-asm", size, count);
        result |= test_memcpy("memcpy", size, count);
        result |= test_memcpy("memcpy-asm", size, count);
        return result;
    }

    char *a = (char *) malloc(size), *b = (char *) malloc(size);
    for (size_t i = 0; i < size; ++i) {
        a[i] = (char) (rand() % 128);
    }
    printf("Using mode \"%s\" for %zu-byte arrays (%zu tests): ", mode, size, count);
    fflush(stdout);
    clock_t start = clock();
    if (strcmp(mode, "1") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy<char>(b, a, size);
    } else if (strcmp(mode, "2") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy<short>(b, a, size);
    } else if (strcmp(mode, "4") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy<int>(b, a, size);
    } else if (strcmp(mode, "8") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy<long long>(b, a, size);
    } else if (strcmp(mode, "16") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy<__m128i>(b, a, size);
    } else if (strcmp(mode, "1-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy_1_asm(b, a, size);
    } else if (strcmp(mode, "2-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy_2_asm(b, a, size);
    } else if (strcmp(mode, "4-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy_4_asm(b, a, size);
    } else if (strcmp(mode, "8-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            element_wise_copy_8_asm(b, a, size);
    } else if (strcmp(mode, "nt") == 0) {
        for (size_t i = 0; i != count; ++i)
            copy_nt(b, a, size);
    } else if (strcmp(mode, "nt-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            copy_nt_asm(b, a, size);
    } else if (strcmp(mode, "memcpy") == 0) {
        for (size_t i = 0; i != count; ++i)
            memcpy(b, a, size);
    } else if (strcmp(mode, "memcpy-asm") == 0) {
        for (size_t i = 0; i != count; ++i)
            memcpy_asm(b, a, size);
    } else {
        printf("unknown mode\n");
        result = -1;
    }

    if (result == 0) {
        clock_t end = clock();
        if (memcmp(a, b, size) == 0) {
            printf("%.2lfs\n", (end - start) / (double) CLOCKS_PER_SEC);
        } else {
            printf("incorrect copy\n");
            result = -1;
        }
    }

    fflush(stdout);
    free(a);
    free(b);
    return result;
}

int main(int argc, char *argv[]) {

    srand((unsigned int) time(0));

    size_t size = (size_t) (1 << 28) + 8;
    size_t count = 100;
    bool aligned = false;
    const char *mode = "memcpy";

    const struct option options[] = {
            {"mode",    required_argument, NULL, 'm'},
            {"size",    required_argument, NULL, 's'},
            {"count",   required_argument, NULL, 'c'},
            {"help",    no_argument,       NULL, 'h'},
            {"aligned", no_argument,       NULL, 'a'},
            {NULL, 0,                      NULL, 0}
    };

    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "am:s:c:h", options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                aligned = true;
                break;
            case 'm':
                mode = optarg;
                break;
            case 's':
                size = (size_t) atoi(optarg);
                break;
            case 'c':
                count = (size_t) atoi(optarg);
                break;
            case 'h':
                printf("Usage: %s [options]\n"
                               "Options:\n"
                               "\t%-20s\t%s\n"
                               "\t%-20s\t%s\n"
                               "\t%-20s\t%s\n"
                               "\t%-20s\t%s\n"
                               "\t%-20s\t%s\n\n"
                               "Available modes:\n"
                               "\t1, 2, 4, 8, 16, 1-asm, 2-asm, 4-asm, 8-asm,\n"
                               "\tnt, nt-asm, memcpy, memcpy-asm\n\n",
                       "memcpy",
                       "-h, --help", "View help",
                       "-m, --mode <mode>", "Set memcpy copy mode",
                       "-s, --size <size>", "Set array size to copy",
                       "-a, --align", "Align memory to 16 bytes",
                       "-c, --count <count>", "Set number of tests");
                return 0;
            case '?':
            default:
                break;
        };
    };

    if (aligned) {
        size = size - size % 16;
        printf("Size of array reduced to %zu bytes\n", size);
    }

    return test_memcpy(mode, size, count);
}