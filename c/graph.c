#include "bitset.h"
#include "graph.h"

#include <stdio.h>

void find_adjacent_vv(setword *s, struct Graph G, setword *adj_vv)
{
    clear_bitset(adj_vv, G.m);
    FOR_EACH_IN_BITSET(v, s, G.m)
        bitset_addall(adj_vv, GRAPHROW(G.g, v, G.m), G.m);
    END_FOR_EACH_IN_BITSET
    for (int k=0; k<G.m; k++)
        bitset_removeall(adj_vv, s, G.m);
}

#define BUFFERSIZE 1024

struct Graph read_graph()
{
    char s[BUFFERSIZE], s1[32], s2[32];
    int n, edge_count;
    int m = 0;   // number of setwords needed to store n bits
    int v, w;
    int num_edges_read = 0;
    graph *g = NULL;
    while (true) {
        if (fgets(s, BUFFERSIZE, stdin) == NULL)
            break;

        switch (s[0]) {
        case '\n':
            break;
        case 'c':
            break;
        case 'p':
            if(sscanf(s, "%s %s %d %d", s1, s2, &n, &edge_count) != 4)
                exit(1);
            m = SETWORDSNEEDED(n);
            g = calloc(n * m, sizeof(graph));
            break;
        default:
            if (sscanf(s, "%d %d", &v, &w) != 2)
                exit(1);
            if (v == w)
                continue;
            --v;
            --w;
            ADDONEEDGE(g, v, w, m);
            ++num_edges_read;
        }
    }
    return (struct Graph) {g, n, m};
}

struct Graph create_empty_graph(int n)
{
    int m = SETWORDSNEEDED(n);
    graph *g = calloc(n * m, sizeof(graph));
    return (struct Graph) {g, n, m};
}
