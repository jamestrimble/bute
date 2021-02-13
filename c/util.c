#include "util.h"

#include <stdint.h>
#include <stdio.h>

static void bute_fail()
{
    fprintf(stderr, "Out of memory.\n");
    abort();
}

void *bute_xmalloc(size_t size)
{
    void *result = malloc(size);
    if (!result && size)
        bute_fail();
    return result;
}

void *bute_xcalloc(size_t nmemb, size_t size)
{
    void *result = calloc(nmemb, size);
    if (!result && nmemb)
        bute_fail();
    return result;
}

void *bute_xrealloc(void *ptr, size_t size)
{
    void *result = realloc(ptr, size);
    if (!result && size)
        bute_fail();
    return result;
}

size_t new_vec_capacity(size_t old_capacity)
{
    if (old_capacity < 2) {
        return old_capacity + 1;
    }
    if (old_capacity >= SIZE_MAX / 2) {
        // avoid overflow
        return old_capacity;
    }
    return old_capacity + old_capacity / 2;
}
