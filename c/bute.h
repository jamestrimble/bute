#ifndef BUTE_H
#define BUTE_H

#include "bitset.h"

struct Bute {
    int m;   // number of setwords needed for each bitset
    struct Bitset *bitset_free_list_head;
    setword **vv_dominated_by;
    setword **vv_that_dominate;
    setword **adj_vv_dominated_by;
    int n;  // number of vertices
};

struct Graph;

void Bute_init(struct Bute *bute, struct Graph G);

void Bute_destroy(struct Bute *bute);

#endif
