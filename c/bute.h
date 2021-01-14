#ifndef BUTE_H
#define BUTE_H

#include "bitset.h"

struct Bute {
    int m;
    struct Bitset *bitset_free_list_head;
};

#endif
