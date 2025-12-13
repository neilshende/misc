#include "bounds.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct BoundRegion {
    void *addr;            // starting address
    size_t len;            // length of region
    struct BoundRegion *next;
} BoundRegion;

static BoundRegion *head = NULL;

// -------------------------------------------
// Insert region into map[address] = length
// -------------------------------------------
void CHECKBOUNDS(void *ptr, size_t len)
{
    BoundRegion *r = malloc(sizeof(BoundRegion));
    r->addr = ptr;
    r->len = len;
    r->next = head;
    head = r;

    // Debug print (optional)
    // printf("CHECKBOUNDS: registered [%p] length %zu\n", ptr, len);
}


// ------------------------------------------------------------
// Returns 1 if ptr..ptr+len is inside any registered region
// ------------------------------------------------------------
static int is_within_bounds(void *ptr, size_t len)
{
    uintptr_t start = (uintptr_t)ptr;
    uintptr_t end   = start + len;

    BoundRegion *cur = head;

    while (cur) {
        uintptr_t rstart = (uintptr_t)cur->addr;
        uintptr_t rend   = rstart + cur->len;

        // Check containment
        if (start >= rstart && end <= rend)
            return 1;

        cur = cur->next;
    }
    return 0;
}


// ------------------------------------------------------------
// RANGECHECK: aborts if write would be out-of-bounds
// ------------------------------------------------------------
void RANGECHECK(void *ptr, size_t len)
{
    if (!is_within_bounds(ptr, len)) {
        fprintf(stderr,
            "BOUNDS VIOLATION: write of %zu bytes at %p\n",
            len, ptr);
        abort();
    }
}

