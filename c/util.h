#ifndef BUTE_UTIL_H
#define BUTE_UTIL_H

#include <stdlib.h>

void *bute_xmalloc(size_t size);
void *bute_xcalloc(size_t nmemb, size_t size);
void *bute_xrealloc(void *ptr, size_t size);
size_t new_vec_capacity(size_t old_capacity);

#endif
