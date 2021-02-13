#ifndef GRAPH_H
#define GRAPH_H

#include "bitset.h"

struct ButeGraph
{
    graph *g;
    int n;
    int m;   // number of words needed for a bitset containing n elements
};

struct ButeGraph bute_create_empty_graph(int n);

// Returns a pointer to the first bitset in a linked list
struct ButeBitsetListNode *bute_make_connected_components(setword *vv, struct ButeGraph G);

#endif
