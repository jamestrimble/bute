#include "bitset.h"
#include "graph.h"

void find_adjacent_vv(setword *s, struct Graph G, setword *adj_vv)
{
    clear_bitset(adj_vv, G.m);
    FOR_EACH_IN_BITSET(v, s, G.m)
        bitset_addall(adj_vv, GRAPHROW(G.g, v, G.m), G.m);
    END_FOR_EACH_IN_BITSET
    for (int k=0; k<G.m; k++)
        bitset_removeall(adj_vv, s, G.m);
}


