#ifndef GRAPH_H
#define GRAPH_H

#include "bitset.h"

struct Graph
{
    graph *g;
    int n;
    int m;   // number of words needed for a bitset containing n elements
};

void find_adjacent_vv(setword *s, struct Graph G, setword *adj_vv);

struct Graph read_graph();

struct Graph create_empty_graph(int n);

// Returns a pointer to the first bitset in a linked list
struct Bitset *make_connected_components(setword *vv, struct Graph G, struct Bute *bute);

#endif
