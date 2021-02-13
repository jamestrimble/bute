#include "bute_solver.h"
#include "bitset.h"
#include "graph.h"
#include "util.h"

static void determine_domination(struct Bute *bute, struct ButeGraph G)
{
    for (int v=0; v<G.n; v++) {
        for (int w=0; w<G.n; w++) {
            if (w != v) {
                setword *nd_of_v_and_v_and_w = bute_get_copy_of_bitset(bute, GRAPHROW(G.g, v, G.m));
                ADDELEMENT(nd_of_v_and_v_and_w, v);
                ADDELEMENT(nd_of_v_and_v_and_w, w);
                setword *nd_of_w_and_v_and_w = bute_get_copy_of_bitset(bute, GRAPHROW(G.g, w, G.m));
                ADDELEMENT(nd_of_w_and_v_and_w, v);
                ADDELEMENT(nd_of_w_and_v_and_w, w);
                if (bute_bitset_is_superset(nd_of_w_and_v_and_w, nd_of_v_and_v_and_w, G.m)) {
                    if (!bute_bitset_equals(nd_of_v_and_v_and_w, nd_of_w_and_v_and_w, G.m) || v < w) {
                        ADDELEMENT(bute->vv_dominated_by[w], v);
                        ADDELEMENT(bute->vv_that_dominate[v], w);
                        if (ISELEMENT(GRAPHROW(G.g, w, G.m), v)) {
                            ADDELEMENT(bute->adj_vv_dominated_by[w], v);
                        }
                    }
                }
                bute_free_bitset(nd_of_w_and_v_and_w);
                bute_free_bitset(nd_of_v_and_v_and_w);
            }
        }
    }
}

void Bute_init(struct Bute *bute, struct ButeGraph G, struct ButeOptions options)
{
    bute->options = options;
    bute->result = (struct ButeResult) {0};
    bute->m = G.m;
    bute->n = G.n;
    bute->adj_vv_dominated_by = bute_xmalloc(G.n * sizeof *bute->adj_vv_dominated_by);
    bute->vv_dominated_by = bute_xmalloc(G.n * sizeof *bute->vv_dominated_by);
    bute->vv_that_dominate = bute_xmalloc(G.n * sizeof *bute->vv_that_dominate);
    for (int v=0; v<G.n; v++) {
        bute->adj_vv_dominated_by[v] = bute_get_empty_bitset(bute);
        bute->vv_dominated_by[v] = bute_get_empty_bitset(bute);
        bute->vv_that_dominate[v] = bute_get_empty_bitset(bute);
    }
    if (options.use_domination) {
        determine_domination(bute, G);
    }
    bute->workspaces = bute_xmalloc(G.n * sizeof *bute->workspaces);
    for (int i=0; i<G.n; i++) {
        bute->workspaces[i] = NULL;
    }
}

void Bute_destroy(struct Bute *bute)
{
    for (int i=0; i<bute->n; i++) {
        bute_free_bitset(bute->adj_vv_dominated_by[i]);
        bute_free_bitset(bute->vv_dominated_by[i]);
        bute_free_bitset(bute->vv_that_dominate[i]);
    }
    free(bute->adj_vv_dominated_by);
    free(bute->vv_dominated_by);
    free(bute->vv_that_dominate);
    for (int i=0; i<bute->n; i++) {
        free(bute->workspaces[i]);
    }
    free(bute->workspaces);
}
