#ifndef BUTE_H
#define BUTE_H

#ifdef __cplusplus
extern "C" {
#endif

struct ButeOptions {
    int use_trie;
    int use_domination;
    int use_top_chain;
    int print_stats;
};

struct ButeResult {
    int treedepth;
    unsigned long long queries;
    unsigned long long helper_calls;
    unsigned long long last_decision_problem_helper_calls;
    unsigned long long set_count;
};

struct Graph;

struct Graph *new_graph(unsigned n);

struct ButeOptions bute_default_options();

// returns a nonzero value if v==w or either vertex is out of range
int graph_add_edge(struct Graph *G, unsigned v, unsigned w);

int graph_node_count(struct Graph *G);

void free_graph(struct Graph *G);

struct ButeResult bute_optimise(struct Graph *G, struct ButeOptions *options, int *parent);

#ifdef __cplusplus
}
#endif

#endif
