#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>

uint64_t my_strtoull(char *field_start) {
        uint64_t res =0;
        while (isdigit(*field_start)) {
                res = 10 * res + (*field_start) - '0';
                ++field_start;
        }
        return res;
}

typedef struct {
    char *start;
    char *end;
    int col;
    uint64_t sum;
} thread_arg_t;

void *sum_column_chunk(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    char *p = a->start;

    // Skip to start of next full line if not beginning of file
    if (p != a->start && *(p - 1) != '\n') {
        while (p < a->end && *p != '\n') p++;
        if (p < a->end) p++;
    }

    while (p < a->end) {
        char *line_end = memchr(p, '\n', a->end - p);
        if (!line_end) line_end = a->end;

        int field = 0;
        char *field_start = p;
        char *field_end = p;

        while (field < a->col && field_end < line_end) {
            if (*field_end == ',') {
                field++;
                field_end++;
                field_start = field_end;
            } else {
                field_end++;
            }
        }

        if (field == a->col && field_start < line_end) {
            a->sum += my_strtoull(field_start);
        }

        p = line_end + 1;
    }

    return NULL;
}

uint64_t sum_column(const char *path, int col) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    struct stat st;
    if (fstat(fd, &st) < 0 || st.st_size == 0) {
        close(fd);
        return 0;
    }

    size_t size = st.st_size;
    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (data == MAP_FAILED) return 0;

    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) num_threads = 1;
    if ((size_t)num_threads > size / 64) num_threads = size / 64;

    pthread_t threads[num_threads];
    thread_arg_t args[num_threads];

    size_t chunk = size / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        char *start = data + i * chunk;

        // Ensure chunks don't split lines
        char *end;
        if (i == num_threads - 1) {
            end = data + size;
        } else {
            end = data + (i + 1) * chunk;
            while (end < data + size && *end != '\n') end++;
            if (end < data + size) end++;  // include newline
        }

        args[i].start = start;
        args[i].end = end;
        args[i].col = col;
        args[i].sum = 0;

        pthread_create(&threads[i], NULL, sum_column_chunk, &args[i]);
    }

    uint64_t total = 0;
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
        total += args[i].sum;
    }

    munmap(data, size);
    return total;
}

int main() {
    const char *path = "large.csv";
    int col = 2; // zero-based column index
    uint64_t result = sum_column(path, col);
    printf("Sum of column %d: %lu\n", col, result);
    return 0;
}

