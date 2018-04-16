#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Shuffles array values
 * @param array
 * @param n
 */
void shuffle(int *array, size_t n) {
    srand((unsigned int) time(0));
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int main(int argc, char **argv) {
    clock_t start, end;
    char sequential = 0;
    unsigned long buffer_size = 512;
    unsigned long long file_size = ULLONG_MAX;

    int opt;
    while ((opt = getopt(argc, argv, "b:s:")) != -1) {
        switch (opt) {
            case 'b':
                buffer_size = (unsigned long) atoi(optarg);
                break;
            case 's':
                file_size = (unsigned long long) atoll(optarg);
                break;
            default:
                break;
        }
    }

    char *buffer = malloc(buffer_size);

    FILE *fin = fopen("test.bin", "rb");
    if (fin == NULL) {
        printf("File not found");
        return 0;
    }

    unsigned long long size = 0, size2 = 0;
    int read_count;

    /*start = clock();
    while (size < file_size && (read_count = read(fileno(fin), buffer, buffer_size)) > 0) {
        size += read_count;
    }
    end = clock();
    double elapsed_time = (end - start) / (double) CLOCKS_PER_SEC;
    printf("Sequential read (%llu bytes, %lu bytes per block): %lf\n", size, buffer_size, elapsed_time);

    rewind(fin);*/
    unsigned long parts = (unsigned long) ((17179869184 + buffer_size - 1) / buffer_size);

    int *part_numbers = malloc(parts * sizeof(int));
    for (int i = 0; i < parts; ++i) {
        part_numbers[i] = i;
    }

    shuffle(part_numbers, parts);

    fclose(fin);
    fin = fopen("test.bin", "rb");

    start = clock();
    for (long i = 0; i < parts; ++i) {
        unsigned long offset = part_numbers[i] * buffer_size;
        fseek(fin, offset, SEEK_SET);
        unsigned int left = buffer_size;
        while (left > 0 && size2 < file_size && (read_count = read(fileno(fin), buffer, left)) > 0) {
            size2 += read_count;
            left -= read_count;
        }
    }
    end = clock();
    free(part_numbers);

    double elapsed_time = (end - start) / (double) CLOCKS_PER_SEC;
    printf("Random read     (%llu bytes, %lu bytes per block): %lf\n", size2, buffer_size, elapsed_time);

    free(buffer);
    fclose(fin);
}