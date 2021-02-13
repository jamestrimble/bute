#include "util.h"

#include <stdio.h>

static void bute_fail()
{
    fprintf(stderr, "Out of memory.\n");
    abort();
}

void *bute_xmalloc(size_t size)
{
    void *result = malloc(size);
    if (!result)
        bute_fail();
    return result;
}

void *bute_xcalloc(size_t nmemb, size_t size)
{
    void *result = calloc(nmemb, size);
    if (!result)
        bute_fail();
    return result;
}

void *bute_xrealloc(void *ptr, size_t size)
{
    void *result = realloc(ptr, size);
    if (!result)
        bute_fail();
    return result;
}

size_t new_vec_capacity(size_t old_capacity)
{
    return old_capacity < 2 ? old_capacity + 1 : old_capacity + old_capacity / 2;
}
