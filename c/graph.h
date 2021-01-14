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

#endif
