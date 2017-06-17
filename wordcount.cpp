#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <getopt.h>

bool test_wordcount(const char *mode, size_t size, int test) {
    printf("Test %d: ", test);
    int result = 0;
    char *str = (char *) malloc(size);
    for (size_t i = 0; i < size; ++i) {
        if (rand() % 5 == 0) {
            str[i] = ' ';
        } else {
            str[i] = (char) ('a' + rand() % ('a' - 'z'));
        }
    }
    
    printf("Using mode \"%s\" with %zu blocks of size %zu: ", mode, count, size);
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
    free(str);
    return true;
}

int main(int argc, char *argv[]) {

    srand((unsigned int) time(0));

    size_t size = (size_t) (2 << 28) + 1;
    size_t count = 100;
    const char *mode = "memcpy";

    const struct option options[] = {
            {"mode",  required_argument, NULL, 'm'},
            {"size",  required_argument, NULL, 's'},
            {"count", required_argument, NULL, 'c'},
            {"help",  no_argument,       NULL, 'h'},
            {NULL, 0,                    NULL, 0}
    };

    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "m:s:c:h", options, &option_index)) != -1) {
        switch (opt) {
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
                               "\t%-20s\t%s\n",
                       "wordcount",
                       "-h, --help", "View help",
                       "-m, --mode <mode>", "Set wordcount mode (slow/fast)",
                       "-s, --size <size>", "Set string size",
                       "-c, --count <count>", "Set number of tests");
                return 0;
            case '?':
            default:
                break;
        };
    };

    for (int i = 0; i < count; ++i) {
        if (!test_wordcount(mode, size, i)) {
            printf(stderr, "FAIL");
            return -1;
        }
    }

    printf("OK");
    return 0;
}