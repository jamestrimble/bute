#include "bitset.h"
#include "graph.h"
#include "util.h"

#include <stdio.h>

struct ButeGraph bute_create_empty_graph(int n)
{
    int m = SETWORDSNEEDED(n);
    graph *g = bute_xcalloc(n * m, sizeof(graph));
    return (struct ButeGraph) {g, n, m};
}

struct ButeBitset *bute_make_connected_components(setword *vv, struct ButeGraph G)
{
    struct ButeBitset *retval = NULL;
    setword *bitsets = bute_xcalloc(2 * G.m, sizeof *bitsets);
    setword *visited = bitsets;
    setword *vv_in_prev_components = bitsets + G.m;
    int *queue = bute_xmalloc(G.n * sizeof *queue);
    FOR_EACH_IN_BITSET(v, vv, G.m)
        if (ISELEMENT(visited, v))
            continue;
        int queue_len = 0;
        ADDELEMENT(visited, v);
        queue[queue_len++] = v;
        while (queue_len) {
            int u = queue[--queue_len];
            FOR_EACH_IN_BITSET(w, GRAPHROW(G.g, u, G.m), G.m)
                if (ISELEMENT(vv, w) && !ISELEMENT(visited, w)) {
                    ADDELEMENT(visited, w);
                    queue[queue_len++] = w;
                }
            END_FOR_EACH_IN_BITSET
        }
        struct ButeBitset *bitset = bute_get_Bitset(G.m);
        bitset->next = retval;
        retval = bitset;
        setword *component = bitset->bitset;
        for (int k=0; k<G.m; k++)
            component[k] = visited[k] & ~vv_in_prev_components[k];

        bute_bitset_addall(vv_in_prev_components, visited, G.m);
    END_FOR_EACH_IN_BITSET

    free(bitsets);
    free(queue);
    return retval;
}
