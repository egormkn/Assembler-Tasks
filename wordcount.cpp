#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <getopt.h>
#include "wordcount.h"

void print_help() {
    printf("Usage: %s [options]\n"
                   "Options:\n"
                   "\t%-20s\t%s\n"
                   "\t%-20s\t%s\n"
                   "\t%-20s\t%s\n",
           "wordcount",
           "-h, --help", "View help",
           "-s, --size <size>", "Set string size",
           "-c, --count <count>", "Set number of tests");
}

bool test_wordcount(size_t size, int test) {
    printf("Test %d: ", test + 1);
    fflush(stdout);
    char *str = (char *) malloc(size + 1);
    for (size_t i = 0; i < size; ++i) {
        if (rand() % 4 == 0) {
            str[i] = ' ';
        } else {
            str[i] = (char) ('a' + rand() % ('z' - 'a' + 1));
        }
    }
    str[size] = '\0';

    clock_t time_slow = clock();
    int slow = wordcount_slow(str + 2, size - 2);
    time_slow = clock() - time_slow;

    clock_t time_fast = clock();
    int fast = wordcount_fast(str + 2, size - 2);
    time_fast = clock() - time_fast;

    free(str);

    if (slow == fast) {
        printf("%.2lf ms (%.1lfx boost)\n", 1000 * time_fast / (double) CLOCKS_PER_SEC, (double) time_slow / time_fast);
    } else {
        printf("wrong answer (%d instead of %d)\n", fast, slow);
    }
    return slow == fast;
}

int main(int argc, char *argv[]) {
    srand((unsigned int) time(0));

    size_t size = (size_t) (1 << 26);
    int count = 10;

    const struct option options[] = {
            {"size",  required_argument, NULL, 's'},
            {"count", required_argument, NULL, 'c'},
            {"help",  no_argument,       NULL, 'h'},
            {NULL, 0,                    NULL, 0}
    };

    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "s:c:h", options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                size = (size_t) atoi(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
            default:
                break;
        };
    };

    printf("# Running %d tests on %zu-byte strings\n", count, size);
    for (int i = 0; i < count; ++i) {
        if (!test_wordcount(size, i)) {
            printf("FAIL\n");
            return -1;
        }
    }

    printf("OK\n");
    return 0;
}