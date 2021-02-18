#include "bute.h"
#include "bitset.h"
#include "bitset_arena.h"
#include "bute_solver.h"
#include "graph.h"
#include "hash_map.h"
#include "trie.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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

    struct ButeListOfBitsetArenas bitset_arenas;
};

static void SetAndNeighbourhoodVec_push(struct SetAndNeighbourhoodVec *vec, struct SetAndNeighbourhood val)
{
    if (vec->len == SIZE_MAX) {
        exit(1);
    }
    if (vec->len == vec->capacity) {
        vec->capacity = new_vec_capacity(vec->capacity);
        vec->vals = bute_xrealloc(vec->vals, vec->capacity * sizeof *vec->vals);
    }
    vec->vals[vec->len++] = val;
}

static void SetAndNeighbourhoodVec_init(struct SetAndNeighbourhoodVec *vec, int m)
{
    *vec = (struct SetAndNeighbourhoodVec) {0};
    bute_add_arena(&vec->bitset_arenas, m);
}

static void SetAndNeighbourhoodVec_destroy(struct SetAndNeighbourhoodVec *vec)
{
    free(vec->vals);
    bute_free_arenas(vec->bitset_arenas);
}

static int cmp_nd_popcount_desc(const void *a, const void *b) {
    const struct SetAndNeighbourhood sa = *(const struct SetAndNeighbourhood *) a;
    const struct SetAndNeighbourhood sb = *(const struct SetAndNeighbourhood *) b;
    int pca = bute_popcount(sa.nd, sa.m);
    int pcb = bute_popcount(sb.nd, sa.m);
    if (pca != pcb) {
        return pca > pcb ? -1 : 1;
    }
    int comp = bute_bitset_compare(sa.nd, sb.nd, sa.m);
    if (comp != 0)
        return comp;
    pca = bute_popcount(sa.set, sa.m);
    pcb = bute_popcount(sb.set, sa.m);
    if (pca != pcb) {
        return pca > pcb ? -1 : 1;
    }
    return bute_bitset_compare(sa.set, sb.set, sa.m);
}

static int cmp_sorted_position(const void *a, const void *b) {
    const struct SetAndNeighbourhood *pa = *((const struct SetAndNeighbourhood **) a);
    const struct SetAndNeighbourhood *pb = *((const struct SetAndNeighbourhood **) b);
    return (pa > pb) - (pa < pb);
}

static void try_adding_STS_root(struct Bute *bute, struct ButeGraph G, int w, setword *union_of_subtrees,
                         setword *nd_of_union_of_subtrees, int root_depth, struct ButeHashMap *set_root,
                         struct SetAndNeighbourhoodVec *new_STSs, setword *workspace)
{
    setword *adj_vv = workspace;
    bute_bitset_union(adj_vv, nd_of_union_of_subtrees, GRAPHROW(G.g, w, G.m), G.m);
    bute_bitset_removeall(adj_vv, union_of_subtrees, G.m);
    DELELEMENT(adj_vv, w);
    if (bute_popcount(adj_vv, G.m) < root_depth) {
        setword *STS = workspace + G.m;
        bute_bitset_copy(STS, union_of_subtrees, G.m);
        ADDELEMENT(STS, w);
        if (bute_intersection_is_empty(bute->vv_dominated_by[w], adj_vv, G.m) &&
            bute_intersection_is_empty(bute->vv_that_dominate[w], STS, G.m)) {
            if (bute_hash_add_or_update(set_root, STS, w, root_depth)) {
                setword *bitsets = bute_get_pair_of_bitsets(&new_STSs->bitset_arenas, G.m);
                setword *set = bitsets;
                setword *nd = bitsets + G.m;
                bute_bitset_copy(set, STS, G.m);
                bute_bitset_copy(nd, adj_vv, G.m);
                SetAndNeighbourhoodVec_push(new_STSs, (struct SetAndNeighbourhood) {set, nd, G.m});
            }
        }
    }
}

static void filter_roots(struct Bute *bute, struct ButeGraph G, setword *new_possible_STS_roots,
                  struct SetAndNeighbourhood **filtered_STSs, size_t filtered_STSs_len,
                  setword *new_union_of_subtrees, setword *nd_of_new_union_of_subtrees, int root_depth,
                  setword *workspace)
{
    setword *new_big_union = workspace;
    bute_clear_bitset(new_big_union, G.m);
    for (size_t i=0; i<filtered_STSs_len; i++) {
        bute_bitset_addall(new_big_union, filtered_STSs[i]->set, G.m);
    }
    FOR_EACH_IN_BITSET(v, new_possible_STS_roots, G.m)
        setword *adj_vv = workspace + G.m;
        bute_bitset_union(adj_vv, nd_of_new_union_of_subtrees, GRAPHROW(G.g, v, G.m), G.m);
        bute_bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
        DELELEMENT(adj_vv, v);
        bute_bitset_removeall(adj_vv, new_big_union, G.m);
        if (bute_popcount(adj_vv, G.m) >= root_depth || !bute_bitset_union_is_superset(new_big_union, new_union_of_subtrees,
                                                                                       bute->adj_vv_dominated_by[v], G.m)) {
            DELELEMENT(new_possible_STS_roots, v);
        }
    END_FOR_EACH_IN_BITSET
}

// if filtered_STSs_len is small, use a simple filtering algorithm to
// avoid the overhead of the trie
#define MIN_LEN_FOR_TRIE 1000

struct STS_collection
{
    bool use_trie;

    // The following fields are only needed if use_trie==true.  Otherwise,
    // the query method simply returns a slice of the STSs array.
    struct ButeTrie trie;
    size_t *almost_subset_end_positions;
    int m;
    int root_depth;
};

void STS_collection_init(struct STS_collection *collection, struct Bute *bute,
        size_t STSs_len, int root_depth)
{
    collection->use_trie = bute->options.use_trie && STSs_len >= MIN_LEN_FOR_TRIE;
    if (collection->use_trie) {
        bute_trie_init(&collection->trie, bute->n, bute->m, bute);
        collection->almost_subset_end_positions = bute_xmalloc(
                STSs_len * sizeof *collection->almost_subset_end_positions);
        collection->m = bute->m;
        collection->root_depth = root_depth;
    }
}

void STS_collection_destroy(struct STS_collection *collection)
{
    if (collection->use_trie) {
        bute_trie_destroy(&collection->trie);
        free(collection->almost_subset_end_positions);
    }
}

void STS_collection_add(struct STS_collection *collection,
        struct SetAndNeighbourhood *STS, size_t pos_in_array)
{
    if (collection->use_trie && collection->root_depth > bute_popcount(STS->nd, collection->trie.m)) {
        bute_trie_add_element(&collection->trie, STS->nd, STS->set, pos_in_array);
    }
}

struct SetAndNeighbourhoodPtrArr
{
    struct SetAndNeighbourhood **arr;
    size_t len;
};

struct SetAndNeighbourhoodPtrArr STS_collection_query(struct STS_collection *collection,
        struct SetAndNeighbourhood **STSs, size_t STSs_len, size_t current_STS_index,
        setword *nd, setword *aux_set, int nd_popcount, struct SetAndNeighbourhood **filtered_STSs)
{
    if (!collection->use_trie) {
        return (struct SetAndNeighbourhoodPtrArr) {STSs + current_STS_index + 1,
                STSs_len - (current_STS_index + 1)};
    }

    size_t almost_subset_end_positions_len;
    bute_trie_get_all_almost_subsets(&collection->trie, nd, aux_set, collection->root_depth - nd_popcount,
            collection->almost_subset_end_positions, &almost_subset_end_positions_len);

    size_t filtered_STSs_len = 0;
    if (collection->root_depth == nd_popcount) {
        size_t it = current_STS_index + 1;
        while (it < STSs_len && bute_bitset_equals(nd, STSs[it]->nd, collection->m)) {
            filtered_STSs[filtered_STSs_len++] = STSs[it++];
        }
    }
    for (size_t j=0; j<almost_subset_end_positions_len; j++) {
        size_t iter = collection->almost_subset_end_positions[j];
        setword *first_nd_in_run = STSs[iter]->nd;
        do {
            filtered_STSs[filtered_STSs_len++] = STSs[iter];
            --iter;
        } while (iter > current_STS_index && bute_bitset_equals(first_nd_in_run, STSs[iter]->nd, collection->m));
    }
    return (struct SetAndNeighbourhoodPtrArr) {filtered_STSs, filtered_STSs_len};
}

static void make_STSs_helper(int depth, struct SetAndNeighbourhood **STSs, size_t STSs_len,
                             struct Bute *bute, struct ButeGraph G, setword *possible_STS_roots, setword *union_of_subtrees, setword *nd_of_union_of_subtrees,
                             int root_depth, struct ButeHashMap *set_root, struct SetAndNeighbourhoodVec *new_STSs)
{
    ++bute->result.helper_calls;

    if (!bute->workspaces[depth]) {
        bute->workspaces[depth] = bute_xmalloc(6 * G.m * sizeof(setword));
    }
    setword *workspace = bute->workspaces[depth];

    FOR_EACH_IN_BITSET(w, possible_STS_roots, G.m)
        try_adding_STS_root(bute, G, w, union_of_subtrees, nd_of_union_of_subtrees, root_depth,
                set_root, new_STSs, workspace);
    END_FOR_EACH_IN_BITSET

    struct STS_collection STS_collection;
    STS_collection_init(&STS_collection, bute, STSs_len, root_depth);

    struct SetAndNeighbourhood **filtered_STSs = bute_xmalloc(STSs_len * sizeof *filtered_STSs);

    for (size_t i=STSs_len; i--; ) {
        setword *s = STSs[i]->set;
        setword *nd = STSs[i]->nd;
        int nd_popcount = bute_popcount(nd, G.m);
        setword *new_possible_STS_roots = workspace;
        bute_bitset_copy(new_possible_STS_roots, possible_STS_roots, G.m);
        bute_bitset_intersect_with(new_possible_STS_roots, nd, G.m);
        setword *new_union_of_subtrees = workspace + G.m;
        bute_bitset_union(new_union_of_subtrees, union_of_subtrees, s, G.m);

        setword *nd_of_new_union_of_subtrees = workspace + G.m * 2;
        bute_bitset_union(nd_of_new_union_of_subtrees, nd_of_union_of_subtrees, nd, G.m);
        bute_bitset_removeall(nd_of_new_union_of_subtrees, new_union_of_subtrees, G.m);

        setword *new_union_of_subtrees_and_nd = workspace + G.m * 3;
        bute_bitset_union(new_union_of_subtrees_and_nd, new_union_of_subtrees, nd_of_new_union_of_subtrees, G.m);

        struct SetAndNeighbourhoodPtrArr query_result = STS_collection_query(&STS_collection,
                STSs, STSs_len, i, nd, new_union_of_subtrees_and_nd, nd_popcount, filtered_STSs);
        ++bute->result.queries;

        size_t filtered_STSs_len = 0;
        for (size_t j=0; j<query_result.len; j++) {
            struct SetAndNeighbourhood candidate = *query_result.arr[j];
            if (bute_intersection_is_empty(candidate.nd, new_possible_STS_roots, G.m))
                continue;
            if (bute_popcount_of_union(nd_of_new_union_of_subtrees, candidate.nd, G.m) > root_depth)
                continue;
            if (!bute_intersection_is_empty(new_union_of_subtrees_and_nd, candidate.set, G.m))
                continue;
            filtered_STSs[filtered_STSs_len++] = query_result.arr[j];
        }

        filter_roots(bute, G, new_possible_STS_roots, filtered_STSs, filtered_STSs_len,
                new_union_of_subtrees, nd_of_new_union_of_subtrees, root_depth, workspace + 4 * G.m);

        if (!bute_bitset_is_empty(new_possible_STS_roots, G.m)) {
            qsort(filtered_STSs, filtered_STSs_len, sizeof *filtered_STSs, cmp_sorted_position);
            make_STSs_helper(depth+1, filtered_STSs, filtered_STSs_len, bute, G,
                    new_possible_STS_roots, new_union_of_subtrees,
                    nd_of_new_union_of_subtrees, root_depth, set_root, new_STSs);
        }

        STS_collection_add(&STS_collection, STSs[i], i);
    }
    free(filtered_STSs);
    STS_collection_destroy(&STS_collection);
}

static struct SetAndNeighbourhoodVec make_STSs(struct SetAndNeighbourhoodVec *STSs, struct Bute *bute, struct ButeGraph G,
        int root_depth, struct ButeHashMap *set_root)
{
    struct SetAndNeighbourhoodVec new_STSs;
    SetAndNeighbourhoodVec_init(&new_STSs, bute->m);

    struct SetAndNeighbourhood **STSs_pointers = bute_xmalloc(STSs->len * sizeof *STSs_pointers);
    for (size_t i=0; i<STSs->len; i++) {
        STSs_pointers[i] = &STSs->vals[i];
    }

    setword *bitsets = bute_xmalloc(2 * G.m * sizeof(setword));
    setword *empty_set = bitsets;
    setword *full_set = bitsets + G.m;
    bute_clear_bitset(empty_set, G.m);
    bute_bitset_set_first_k_bits(full_set, G.n);

    make_STSs_helper(0, STSs_pointers, STSs->len, bute, G, full_set, empty_set,
            empty_set, root_depth, set_root, &new_STSs);

    qsort(new_STSs.vals, new_STSs.len, sizeof(struct SetAndNeighbourhood), cmp_nd_popcount_desc);
    free(bitsets);
    free(STSs_pointers);
    return new_STSs;
}

static void add_parents(struct Bute *bute, int *parent, struct ButeGraph G, struct ButeHashMap *set_root, setword *s, int parent_vertex)
{
    int v = -1;
    bute_hash_get_val(set_root, s, &v);
    parent[v] = parent_vertex;
    DELELEMENT(s, v);   // temporarily remove root of subtree
    struct ButeBitsetListNode * components = bute_make_connected_components(s, G);
    for (struct ButeBitsetListNode *component=components; component != NULL; component=component->next) {
        add_parents(bute, parent, G, set_root, component->bitset, v);
    }
    bute_free_list_of_bitsets(components);
    ADDELEMENT(s, v);   // restore root of subtree
}

static bool solve(struct Bute *bute, struct ButeGraph G, int target, int *parent)
{
    bool retval = false;
    struct ButeHashMap set_root;
    bute_hash_init(&set_root, bute);

    struct SetAndNeighbourhoodVec STSs;
    SetAndNeighbourhoodVec_init(&STSs, bute->m);

    for (int root_depth=target; root_depth>=1; root_depth--) {
        size_t prev_set_root_size = set_root.sz;
//        printf("target %d  root depth %d\n", target, root_depth);
//        printf(" %d\n", STSs_len);

        struct SetAndNeighbourhoodVec new_STSs = make_STSs(&STSs, bute, G, root_depth, &set_root);
        SetAndNeighbourhoodVec_destroy(&STSs);
        STSs = new_STSs;
        if (set_root.sz == prev_set_root_size) {
            break;
        }

        bute->result.set_count += set_root.sz - prev_set_root_size;

        if (root_depth == 1) {
            int total_size = 0;
            for (size_t i=0; i<STSs.len; i++) {
                total_size += bute_popcount(STSs.vals[i].set, G.m);
            }
            if (total_size == G.n) {
                retval = true;
                for (size_t i=0; i<STSs.len; i++) {
                    add_parents(bute, parent, G, &set_root, STSs.vals[i].set, -1);
                }
            }
        } else if (bute->options.use_top_chain) {
            for (size_t i=0; i<STSs.len; i++) {
                if (!retval && bute_popcount(STSs.vals[i].set, G.m) + bute_popcount(STSs.vals[i].nd, G.m) == G.n) {
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
    
    SetAndNeighbourhoodVec_destroy(&STSs);
    bute_hash_destroy(&set_root);
    return retval;
}

static void optimise(struct ButeGraph G, int *parent, struct Bute *bute)
{
    if (G.n == 0) {
        return;
    }
    int target = 1;
    for ( ; target<=G.n; target++) {
        if (target == bute->options.upper_bound) {
            break;
        }
        unsigned long long prev_helper_calls = bute->result.helper_calls;
        bool result = solve(bute, G, target, parent);
        bute->result.last_decision_problem_helper_calls =
                bute->result.helper_calls - prev_helper_calls;
        if (result) {
            break;
        }
    }
    bute->result.treedepth = target;
}

struct ButeOptions bute_default_options()
{
    return (struct ButeOptions) {
        .use_trie=1,
        .use_domination=1,
        .use_top_chain=1,
        .print_stats=0,
        .upper_bound=-1
    };
}

struct ButeGraph *bute_new_graph(unsigned n)
{
    struct ButeGraph *G = bute_xmalloc(sizeof *G);
    *G = bute_create_empty_graph(n);
    return G;
}

int bute_graph_add_edge(struct ButeGraph *G, unsigned v, unsigned w)
{
    unsigned n = G->n;
    if (v == w || v >= n || w >= n)
        return BUTE_INVALID_EDGE;
    ADDONEEDGE(G->g, v, w, G->m);
    return BUTE_OK;
}

int bute_graph_node_count(struct ButeGraph *G)
{
    return G->n;
}

void bute_free_graph(struct ButeGraph *G)
{
    free(G->g);
    free(G);
}

struct ButeResult bute_optimise(struct ButeGraph *G, struct ButeOptions *options, int *parent)
{
    struct Bute bute;
    struct ButeOptions opts = options==NULL ? bute_default_options() : *options;
    Bute_init(&bute, *G, opts);
    optimise(*G, parent, &bute);
    Bute_destroy(&bute);
    return bute.result;
}
