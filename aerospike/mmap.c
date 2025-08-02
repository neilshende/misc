#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>

uint64_t my_strtoull(char *field_start) {
        uint64_t res =0;
	while (isdigit(*field_start)) {
		res = 10 * res + (*field_start) - '0';
		++field_start;
	}
	return res;
}

uint64_t sum_column(const char *path, int col) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return 0;
    }

    size_t size = st.st_size;
    if (size == 0) {
        close(fd);
        return 0;
    }

    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return 0;
    }

    uint64_t sum = 0;
    char *p = data;
    char *end = data + size;

    while (p < end) {
        char *line_start = p;
        char *line_end = memchr(p, '\n', end - p);
        if (!line_end) line_end = end;

        int field = 0;
        char *field_start = line_start;
        char *field_end = line_start;

        // Navigate to the desired column
        while (field < col && field_end < line_end) {
            if (*field_end == ',') {
                field++;
                field_end++;
                field_start = field_end;
            } else {
                field_end++;
            }
        }

        // If we found the column and it's not at end of line
        if (field == col && field_start < line_end) {
            // Temporarily null-terminate the field
            sum += my_strtoull(field_start);
        }

        p = line_end + 1;
    }

    munmap(data, size);
    close(fd);
    return sum;
}

int main() {
    const char *path = "large.csv";
    int col = 2; // zero-based column index
    uint64_t result = sum_column(path, col);
    printf("Sum of column %d: %lu\n", col, result);
    return 0;
}

