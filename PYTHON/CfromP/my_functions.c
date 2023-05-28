#include <stdio.h>

int square(int i, int *j) {
        printf("inside C, *j is %d\n", *j);
        *j = i*i;
	return i * i;
}
