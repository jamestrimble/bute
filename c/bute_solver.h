#ifndef BUTE_SOLVER_H
#define BUTE_SOLVER_H

#include "bitset.h"
#include "bute.h"

struct Bute {
    int m;   // number of setwords needed for each bitset
    struct ButeBitset *bitset_free_list_head;
    setword **vv_dominated_by;
    setword **vv_that_dominate;
    setword **adj_vv_dominated_by;
    int n;  // number of vertices
    struct ButeOptions options;
    struct ButeResult result;
};

struct ButeGraph;

void Bute_init(struct Bute *bute, struct ButeGraph G, struct ButeOptions options);

void Bute_destroy(struct Bute *bute);

#endif
