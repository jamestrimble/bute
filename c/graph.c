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

struct Bitset *make_connected_components(setword *vv, struct Graph G, struct Bute *bute)
{
    struct Bitset *retval = NULL;
    setword *visited = get_empty_bitset(bute);
    setword *vv_in_prev_components = get_empty_bitset(bute);
    int *queue = malloc(G.n * sizeof *queue);
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
        struct Bitset *bitset = get_Bitset(bute);
        bitset->next = retval;
        retval = bitset;
        setword *component = bitset->bitset;
        for (int k=0; k<G.m; k++)
            component[k] = visited[k] & ~vv_in_prev_components[k];

        bitset_addall(vv_in_prev_components, visited, G.m);
    END_FOR_EACH_IN_BITSET

    free_bitset(bute, vv_in_prev_components);
    free_bitset(bute, visited);
    free(queue);
    return retval;
}

