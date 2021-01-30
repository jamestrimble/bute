#ifndef BUTE_H
#define BUTE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Graph;

struct Graph *new_graph(unsigned n);

// returns a nonzero value if v==w or either vertex is out of range
int graph_add_edge(struct Graph *G, unsigned v, unsigned w);

int graph_node_count(struct Graph *G);

void free_graph(struct Graph *G);

int bute_optimise(struct Graph *G, int *parent);

#ifdef __cplusplus
}
#endif

#endif
