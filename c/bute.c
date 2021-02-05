#include "bute.h"
#include "bitset.h"
#include "bute_solver.h"
#include "graph.h"
#include "hash_map.h"
#include "trie.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

struct SetAndNeighbourhood
{
    setword *set;
    setword *nd;
    int m;
};

struct SetAndNeighbourhoodVec
{
    struct SetAndNeighbourhood *vals;
    size_t capacity;
    size_t len;
};

void SetAndNeighbourhoodVec_push(struct SetAndNeighbourhoodVec *vec, struct SetAndNeighbourhood val)
{
    if (vec->len == vec->capacity) {
        vec->capacity = vec->capacity == 0 ? 1 :
                        vec->capacity == 1 ? 2 :
                        vec->capacity + vec->capacity / 2 + 1;
        vec->vals = bute_xrealloc(vec->vals, vec->capacity * sizeof *vec->vals);
    }
    vec->vals[vec->len++] = val;
}

void SetAndNeighbourhoodVec_destroy(struct Bute *bute, struct SetAndNeighbourhoodVec *vec)
{
    for (size_t i=0; i<vec->len; i++) {
        free_bitset(bute, vec->vals[i].set);
        free_bitset(bute, vec->vals[i].nd);
    }
    free(vec->vals);
}

int cmp_nd_popcount_desc(const void *a, const void *b) {
    const struct SetAndNeighbourhood sa = *(const struct SetAndNeighbourhood *) a;
    const struct SetAndNeighbourhood sb = *(const struct SetAndNeighbourhood *) b;
    int pca = popcount(sa.nd, sa.m);
    int pcb = popcount(sb.nd, sa.m);
    if (pca != pcb) {
        return pca > pcb ? -1 : 1;
    }
    int comp = bitset_compare(sa.nd, sb.nd, sa.m);
    if (comp != 0)
        return comp;
    pca = popcount(sa.set, sa.m);
    pcb = popcount(sb.set, sa.m);
    if (pca != pcb) {
        return pca > pcb ? -1 : 1;
    }
    return bitset_compare(sa.set, sb.set, sa.m);
}

int cmp_sorted_position(const void *a, const void *b) {
    const struct SetAndNeighbourhood *pa = *((const struct SetAndNeighbourhood **) a);
    const struct SetAndNeighbourhood *pb = *((const struct SetAndNeighbourhood **) b);
    if (pa < pb)
        return -1;
    return pa > pb;
}

void try_adding_STS_root(struct Bute *bute, struct Graph G, int w, setword *union_of_subtrees,
        setword *nd_of_union_of_subtrees, int root_depth, struct hash_map *set_root,
        struct SetAndNeighbourhoodVec *new_STSs)
{
    setword *adj_vv = get_union_of_bitsets(bute, nd_of_union_of_subtrees, GRAPHROW(G.g, w, G.m));
    bitset_removeall(adj_vv, union_of_subtrees, G.m);
    DELELEMENT(adj_vv, w);
    if (popcount(adj_vv, G.m) < root_depth) {
        setword *STS = get_copy_of_bitset(bute, union_of_subtrees);
        ADDELEMENT(STS, w);
        if (intersection_is_empty(bute->vv_dominated_by[w], adj_vv, G.m) &&
                intersection_is_empty(bute->vv_that_dominate[w], STS, G.m)) {
            if (hash_add_or_update(set_root, STS, w, root_depth)) {
                SetAndNeighbourhoodVec_push(new_STSs, (struct SetAndNeighbourhood)
                        {get_copy_of_bitset(bute, STS), get_copy_of_bitset(bute, adj_vv), G.m});
            }
        }
        free_bitset(bute, STS);
    }
    free_bitset(bute, adj_vv);
}

void filter_roots(struct Bute *bute, struct Graph G, setword *new_possible_STS_roots,
        struct SetAndNeighbourhood **filtered_STSs, int filtered_STSs_len,
        setword *new_union_of_subtrees, setword *nd_of_new_union_of_subtrees, int root_depth)
{
    setword *new_big_union = get_empty_bitset(bute);
    for (size_t i=0; i<filtered_STSs_len; i++) {
        bitset_addall(new_big_union, filtered_STSs[i]->set, G.m);
    }
    FOR_EACH_IN_BITSET(v, new_possible_STS_roots, G.m)
        setword *adj_vv = get_union_of_bitsets(bute, nd_of_new_union_of_subtrees, GRAPHROW(G.g,v,G.m));
        bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
        DELELEMENT(adj_vv, v);
        bitset_removeall(adj_vv, new_big_union, G.m);
        if (popcount(adj_vv, G.m) >= root_depth || !bitset_union_is_superset(new_big_union, new_union_of_subtrees, bute->adj_vv_dominated_by[v], G.m)) {
            DELELEMENT(new_possible_STS_roots, v);
        }
        free_bitset(bute, adj_vv);
    END_FOR_EACH_IN_BITSET
    free_bitset(bute, new_big_union);
}

// if filtered_STSs_len is small, use a simple filtering algorithm to
// avoid the overhead of the trie
#define MIN_LEN_FOR_TRIE 1000

void make_STSs_helper(struct SetAndNeighbourhood **STSs, size_t STSs_len,
        struct Bute *bute, struct Graph G, setword *possible_STS_roots, setword *union_of_subtrees, setword *nd_of_union_of_subtrees,
        int root_depth, struct hash_map *set_root, struct SetAndNeighbourhoodVec *new_STSs)
{
    ++bute->result.helper_calls;
    FOR_EACH_IN_BITSET(w, possible_STS_roots, G.m)
        try_adding_STS_root(bute, G, w, union_of_subtrees, nd_of_union_of_subtrees, root_depth,
                set_root, new_STSs);
    END_FOR_EACH_IN_BITSET

    struct Trie trie;
    bool use_trie = bute->options.use_trie && STSs_len >= MIN_LEN_FOR_TRIE;
    if (use_trie)
        trie_init(&trie, G.n, G.m, bute);
    size_t *almost_subset_end_positions = bute_xmalloc(STSs_len * sizeof *almost_subset_end_positions);

    struct SetAndNeighbourhood **filtered_STSs = bute_xmalloc(STSs_len * sizeof *filtered_STSs);

    for (size_t i=STSs_len; i--; ) {
        setword *s = STSs[i]->set;
        setword *nd = STSs[i]->nd;
        int nd_popcount = popcount(nd, G.m);
        setword *new_possible_STS_roots = get_copy_of_bitset(bute, possible_STS_roots);
        bitset_intersect_with(new_possible_STS_roots, nd, G.m);
        setword *new_union_of_subtrees = get_union_of_bitsets(bute, union_of_subtrees, s);

        setword *nd_of_new_union_of_subtrees = get_union_of_bitsets(bute, nd_of_union_of_subtrees, nd);
        bitset_removeall(nd_of_new_union_of_subtrees, new_union_of_subtrees, G.m);

        setword *new_union_of_subtrees_and_nd = get_union_of_bitsets(bute,
                new_union_of_subtrees, nd_of_new_union_of_subtrees);

        struct SetAndNeighbourhood **candidates;
        size_t candidates_len = 0;
        ++bute->result.queries;
        if (use_trie) {
            candidates = filtered_STSs;         // we'll filter in place afterwards
            size_t almost_subset_end_positions_len;
            trie_get_all_almost_subsets(&trie, nd, new_union_of_subtrees_and_nd, root_depth - nd_popcount,
                    almost_subset_end_positions, &almost_subset_end_positions_len);
            if (root_depth == nd_popcount) {
                size_t it = i + 1;
                while (it < STSs_len && bitset_equals(nd, STSs[it]->nd, G.m)) {
                    candidates[candidates_len++] = STSs[it++];
                }
            }
            for (size_t j=0; j<almost_subset_end_positions_len; j++) {
                size_t iter = almost_subset_end_positions[j];
                setword *first_nd_in_run = STSs[iter]->nd;
                do {
                    candidates[candidates_len++] = STSs[iter];
                    --iter;
                } while (iter > i && bitset_equals(first_nd_in_run, STSs[iter]->nd, G.m));
            }
        } else {
            candidates = STSs + i+1;
            candidates_len = STSs_len - (i+1);
        }
        size_t filtered_STSs_len = 0;
        for (size_t j=0; j<candidates_len; j++) {
            struct SetAndNeighbourhood candidate = *candidates[j];
            if (intersection_is_empty(candidate.nd, new_possible_STS_roots, G.m))
                continue;
            if (popcount_of_union(nd_of_new_union_of_subtrees, candidate.nd, G.m) > root_depth)
                continue;
            if (!intersection_is_empty(new_union_of_subtrees_and_nd, candidate.set, G.m))
                continue;
            filtered_STSs[filtered_STSs_len++] = candidates[j];
        }

        filter_roots(bute, G, new_possible_STS_roots, filtered_STSs, filtered_STSs_len,
                new_union_of_subtrees, nd_of_new_union_of_subtrees, root_depth);

        if (!isempty(new_possible_STS_roots, G.m)) {
            qsort(filtered_STSs, filtered_STSs_len, sizeof *filtered_STSs, cmp_sorted_position);
            make_STSs_helper(filtered_STSs, filtered_STSs_len, bute, G,
                    new_possible_STS_roots, new_union_of_subtrees,
                    nd_of_new_union_of_subtrees, root_depth, set_root, new_STSs);
        }

        free_bitset(bute, new_union_of_subtrees_and_nd);
        free_bitset(bute, nd_of_new_union_of_subtrees);
        free_bitset(bute, new_possible_STS_roots);
        free_bitset(bute, new_union_of_subtrees);

        if (use_trie && root_depth > nd_popcount) {
            trie_add_element(&trie, nd, s, i);
        }
    }
    free(filtered_STSs);
    if (use_trie)
        trie_destroy(&trie);
    free(almost_subset_end_positions);
}

struct SetAndNeighbourhoodVec make_STSs(struct SetAndNeighbourhoodVec *STSs, struct Bute *bute, struct Graph G,
        int root_depth, struct hash_map *set_root)
{
    struct SetAndNeighbourhoodVec new_STSs = {NULL, 0, 0};

    struct SetAndNeighbourhood **STSs_pointers = bute_xmalloc(STSs->len * sizeof *STSs_pointers);
    for (size_t i=0; i<STSs->len; i++) {
        STSs_pointers[i] = &STSs->vals[i];
    }

    setword *empty_set = get_empty_bitset(bute);
    setword *full_set = get_full_bitset(bute, G.n);

    make_STSs_helper(STSs_pointers, STSs->len, bute, G, full_set, empty_set,
            empty_set, root_depth, set_root, &new_STSs);
    free_bitset(bute, empty_set);
    free_bitset(bute, full_set);

    free(STSs_pointers);
    qsort(new_STSs.vals, new_STSs.len, sizeof(struct SetAndNeighbourhood), cmp_nd_popcount_desc);
    return new_STSs;
}

void add_parents(struct Bute *bute, int *parent, struct Graph G, struct hash_map *set_root, setword *s, int parent_vertex)
{
    int v;
    hash_get_val(set_root, s, &v);
    parent[v] = parent_vertex;
    setword *vv_in_child_subtrees = get_copy_of_bitset(bute, s);
    DELELEMENT(vv_in_child_subtrees, v);
    struct Bitset * components = make_connected_components(vv_in_child_subtrees, G, bute);
    for (struct Bitset *component=components; component!=NULL; component=component->next) {
        add_parents(bute, parent, G, set_root, component->bitset, v);
    }
    free_Bitsets(bute, components);
    free_bitset(bute, vv_in_child_subtrees);
}

bool solve(struct Bute *bute, struct Graph G, int target, int *parent)
{
    bool retval = false;
    struct hash_map set_root;
    hash_init(&set_root, bute);

    struct SetAndNeighbourhoodVec STSs = {NULL, 0, 0};

    for (int root_depth=target; root_depth>=1; root_depth--) {
        int prev_set_root_size = set_root.sz;
//        printf("target %d  root depth %d\n", target, root_depth);
//        printf(" %d\n", STSs_len);

        struct SetAndNeighbourhoodVec new_STSs = make_STSs(&STSs, bute, G, root_depth, &set_root);
        SetAndNeighbourhoodVec_destroy(bute, &STSs);
        STSs = new_STSs;
        if (set_root.sz == prev_set_root_size) {
            break;
        }

        bute->result.set_count += set_root.sz - prev_set_root_size;

        if (root_depth == 1) {
            int total_size = 0;
            for (size_t i=0; i<STSs.len; i++) {
                total_size += popcount(STSs.vals[i].set, G.m);
            }
            if (total_size == G.n) {
                retval = true;
                for (size_t i=0; i<STSs.len; i++) {
                    add_parents(bute, parent, G, &set_root, STSs.vals[i].set, -1);
                }
            }
        } else if (bute->options.use_top_chain) {
            for (size_t i=0; i<STSs.len; i++) {
                if (!retval && popcount(STSs.vals[i].set, G.m) + popcount(STSs.vals[i].nd, G.m) == G.n) {
                    retval = true;
                    int parent_vertex = -1;
                    FOR_EACH_IN_BITSET(w, STSs.vals[i].nd, G.m)
                        parent[w] = parent_vertex;
                        parent_vertex = w;
                    END_FOR_EACH_IN_BITSET
                    add_parents(bute, parent, G, &set_root, STSs.vals[i].set, parent_vertex);
                }
                if (retval) {
                    break;
                }
            }
        }
        if (retval) {
            break;
        }
    }
    
    SetAndNeighbourhoodVec_destroy(bute, &STSs);
    hash_destroy(&set_root);
    return retval;
}

void optimise(struct Graph G, int *parent, struct Bute *bute)
{
    if (G.n == 0) {
        return;
    }
    for (int target=1 ; target<=G.n; target++) {
//        printf("target %d\n", target);
        unsigned long long prev_helper_calls = bute->result.helper_calls;
        bool result = solve(bute, G, target, parent);
        if (result) {
            bute->result.treedepth = target;
            bute->result.last_decision_problem_helper_calls =
                    bute->result.helper_calls - prev_helper_calls;
            return;
        }
    }
}

struct ButeOptions bute_default_options()
{
    return (struct ButeOptions) {
        .use_trie=1,
        .use_domination=1,
        .use_top_chain=1,
        .print_stats=0
    };
}

struct Graph *new_graph(unsigned n)
{
    struct Graph *G = bute_xmalloc(sizeof *G);
    *G = create_empty_graph(n);
    return G;
}

int graph_add_edge(struct Graph *G, unsigned v, unsigned w)
{
    if (v == w || v >= G->n || w >= G->n)
        return -1;
    ADDONEEDGE(G->g, v, w, G->m);
    return 0;
}

int graph_node_count(struct Graph *G)
{
    return G->n;
}

void free_graph(struct Graph *G)
{
    free(G->g);
    free(G);
}

struct ButeResult bute_optimise(struct Graph *G, struct ButeOptions *options, int *parent)
{
    struct Bute bute;
    struct ButeOptions opts = options==NULL ? bute_default_options() : *options;
    Bute_init(&bute, *G, opts);
    optimise(*G, parent, &bute);
    Bute_destroy(&bute);
    return bute.result;
}
