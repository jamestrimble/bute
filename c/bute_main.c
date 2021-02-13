#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bute.h"

#define BUFFERSIZE 1024

struct ButeGraph *read_graph_from_stdin()
{
    struct ButeGraph *G = NULL;
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
            G = bute_new_graph(n);
            break;
        default:
            if (sscanf(s, "%d %d", &v, &w) != 2)
                exit(1);
            if (v == w)
                continue;
            --v;
            --w;
                bute_graph_add_edge(G, v, w);
            ++num_edges_read;
        }
    }
    return G;
}

int main(int argc, char *argv[])
{
    struct ButeOptions options = bute_default_options();
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--no-trie")) {
            options.use_trie = 0;
        } else if (!strcmp(argv[i], "--no-domination")) {
            options.use_domination = 0;
        } else if (!strcmp(argv[i], "--no-top_chain")) {
            options.use_top_chain = 0;
        } else if (!strcmp(argv[i], "--print-stats")) {
            options.print_stats = 1;
        }
    }
    struct ButeGraph *G = read_graph_from_stdin();
    if (G == NULL) {
        return 1;
    }
    int *parent = malloc(bute_graph_node_count(G) * sizeof *parent);

    struct ButeResult result = bute_optimise(G, &options, parent);

    if (result.return_code == BUTE_OK) {
        int treedepth = result.treedepth;

        if (options.print_stats) {
            printf("# queries %llu\n", result.queries);
            printf("# helperCalls %llu\n", result.helper_calls);
            printf("# lastDecisionProblemHelperCalls %llu\n", result.last_decision_problem_helper_calls);
            printf("# setCount %llu\n", result.set_count);
        }
        printf("%d\n", treedepth);
        for (int i=0; i < bute_graph_node_count(G); i++) {
            printf("%d\n", parent[i] + 1);
        }
    } else {
        printf("%s: error %d\n", argv[0], result.return_code);
    }

    bute_free_graph(G);
    free(parent);
}
