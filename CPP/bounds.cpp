#include "bounds.hpp"
#include <map>
#include <mutex>
#include <cstdio>
#include <cstdlib>

//
// Map of starting address --> length
//
static std::map<void*, size_t> bounds;
static std::mutex bounds_mutex;

//
// Register a region
//
void CHECKBOUNDS(void* ptr, size_t len)
{
    std::lock_guard<std::mutex> lock(bounds_mutex);
    bounds[ptr] = len;

    // Uncomment for debugging:
    // printf("CHECKBOUNDS: registered %p len=%zu\n", ptr, len);
}

//
// Check if ptr..ptr+len lies within ANY registered region
//
static bool is_within_bounds(void* ptr, size_t len)
{
    uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t end   = start + len;

    // We need to find the region whose starting address <= start.
    //
    // lower_bound gives first key >= start, so we need to look one step back.
    std::lock_guard<std::mutex> lock(bounds_mutex);

    auto it = bounds.upper_bound(ptr);
    if (it == bounds.begin())
        return false;

    --it;

    void* region_base = it->first;
    size_t region_len = it->second;

    uintptr_t rstart = reinterpret_cast<uintptr_t>(region_base);
    uintptr_t rend   = rstart + region_len;

    return start >= rstart && end <= rend;
}

//
// RANGECHECK: abort if write is outside known regions
//
void RANGECHECK(void* ptr, size_t len)
{
    if (!is_within_bounds(ptr, len)) {
        fprintf(stderr,
            "BOUNDS ERROR: write of %zu bytes at %p is OUTSIDE any registered region\n",
            len, ptr);
        abort();
    }
}

