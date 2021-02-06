#ifndef BUTE_H
#define BUTE_H

#ifdef __cplusplus
extern "C" {
#endif

#define BUTE_OK              0
#define BUTE_OUT_OF_MEMORY   1
#define BUTE_INVALID_EDGE    2
#define BUTE_UNSAT         101

struct ButeOptions {
    int use_trie;
    int use_domination;
    int use_top_chain;
    int print_stats;
};

struct ButeResult {
    int return_code;
    int treedepth;
    unsigned long long queries;
    unsigned long long helper_calls;
    unsigned long long last_decision_problem_helper_calls;
    unsigned long long set_count;
};

struct ButeGraph;

struct ButeGraph *bute_new_graph(unsigned n);

struct ButeOptions bute_default_options();

// returns BUTE_INVALID_EDGE if v==w or either vertex is out of range
int bute_graph_add_edge(struct ButeGraph *G, unsigned v, unsigned w);

int bute_graph_node_count(struct ButeGraph *G);

void bute_free_graph(struct ButeGraph *G);

struct ButeResult bute_optimise(struct ButeGraph *G, struct ButeOptions *options, int *parent);

#ifdef __cplusplus
}
#endif

#endif
