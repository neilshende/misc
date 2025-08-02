Attached are following files:

1. mmap.c
2. smp.c
3. fastmemchr.c

mmap.c
------
This file implements memory mapping file "large.csv", instead of using fopen and fgets.

smp.c
-----
This file implements spilting the job into multiple threads for parallel processing.

fastmemchr.c
------------
This file implements the memchr function to use (uint64_t *) instead of (char *). Many corner cases
are handled here to get performance rather than degradation. The uint64_t pointer needs to be 8 byte aligned.
So first few bytes and last few bytes are handled via char pointer to keep alignment. Also when the char is found
inside 64 bit pointer, need to fall back to char pointer.


The programs were tested on ubuntu vm. All programs produced consistent results.
Just to be doubly sure, Following R code also gave the same result:

> data <- read.csv("large.csv", header=FALSE)
> sum(data[3])

