#include <stdio.h>
#include <stdlib.h>
#include "bute.h"

#define BUFFERSIZE 1024

struct Graph *read_graph_from_stdin()
{
    struct Graph *G = NULL;
    char s[BUFFERSIZE], s1[32], s2[32];
    int n, edge_count;
    int v, w;
    int num_edges_read = 0;
    while (1) {
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
            G = new_graph(n);
            break;
        default:
            if (sscanf(s, "%d %d", &v, &w) != 2)
                exit(1);
            if (v == w)
                continue;
            --v;
            --w;
            graph_add_edge(G, v, w);
            ++num_edges_read;
        }
    }
    return G;
}

int main(int argc, char *argv[])
{
    struct Graph *G = read_graph_from_stdin();
    if (G == NULL) {
        return 1;
    }
    int *parent = malloc(graph_node_count(G) * sizeof *parent);

    int treedepth = bute_optimise(G, parent);

    printf("%d\n", treedepth);
    for (int i=0; i<graph_node_count(G); i++) {
        printf("%d\n", parent[i] + 1);
    }

    free_graph(G);
    free(parent);
}
