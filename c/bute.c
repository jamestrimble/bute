#include "bute.h"
#include "bitset.h"
#include "graph.h"

void Bute_init(struct Bute *bute, struct Graph G)
{
    bute->m = G.m;
    bute->bitset_free_list_head = NULL;
    bute->n = G.n;
    bute->adj_vv_dominated_by = malloc(G.n * sizeof *bute->adj_vv_dominated_by);
    bute->vv_dominated_by = malloc(G.n * sizeof *bute->vv_dominated_by);
    bute->vv_that_dominate = malloc(G.n * sizeof *bute->vv_that_dominate);
    for (int v=0; v<G.n; v++) {
        bute->adj_vv_dominated_by[v] = get_empty_bitset(bute);
        bute->vv_dominated_by[v] = get_empty_bitset(bute);
        bute->vv_that_dominate[v] = get_empty_bitset(bute);
    }
    for (int v=0; v<G.n; v++) {
        for (int w=0; w<G.n; w++) {
            if (w != v) {
                setword *nd_of_v_and_v_and_w = get_copy_of_bitset(bute, GRAPHROW(G.g, v, G.m));
                ADDELEMENT(nd_of_v_and_v_and_w, v);
                ADDELEMENT(nd_of_v_and_v_and_w, w);
                setword *nd_of_w_and_v_and_w = get_copy_of_bitset(bute, GRAPHROW(G.g, w, G.m));
                ADDELEMENT(nd_of_w_and_v_and_w, v);
                ADDELEMENT(nd_of_w_and_v_and_w, w);
                if (bitset_is_superset(nd_of_w_and_v_and_w, nd_of_v_and_v_and_w, G.m)) {
                    if (!bitset_equals(nd_of_v_and_v_and_w, nd_of_w_and_v_and_w, G.m) || v < w) {
                        ADDELEMENT(bute->vv_dominated_by[w], v);
                        ADDELEMENT(bute->vv_that_dominate[v], w);
                        if (ISELEMENT(GRAPHROW(G.g, w, G.m), v)) {
                            ADDELEMENT(bute->adj_vv_dominated_by[w], v);
                        }
                    }
                }
                free_bitset(bute, nd_of_w_and_v_and_w);
                free_bitset(bute, nd_of_v_and_v_and_w);
            }
        }
    }
}

void Bute_destroy(struct Bute *bute)
{
    for (int i=0; i<bute->n; i++) {
        free_bitset(bute, bute->adj_vv_dominated_by[i]);
        free_bitset(bute, bute->vv_dominated_by[i]);
        free_bitset(bute, bute->vv_that_dominate[i]);
    }
    free(bute->adj_vv_dominated_by);
    free(bute->vv_dominated_by);
    free(bute->vv_that_dominate);
    deallocate_Bitsets(bute);
}
