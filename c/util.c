#include "util.h"

#include <stdio.h>

void bute_fail()
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
