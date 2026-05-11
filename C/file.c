#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char buffer[1024];
    size_t n;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        fwrite(buffer, 1, n, stdout);
    }

    if (ferror(fp)) {
        perror("fread");
    }

    fclose(fp);
    return EXIT_SUCCESS;
}

