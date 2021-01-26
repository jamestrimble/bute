#ifndef BUTE_H
#define BUTE_H

struct Graph;

struct Graph *new_graph(int n);
void graph_add_edge(struct Graph *G, int v, int w);
int graph_node_count(struct Graph *G);
void free_graph(struct Graph *G);

int bute_optimise(struct Graph *G, int *parent);

#endif
