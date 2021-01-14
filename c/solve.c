#include "bitset.h"
#include "bute.h"
#include "graph.h"
#include "hash_map.h"
#include "trie.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

long tmp_counter = 0;

struct SetAndNeighbourhood
{
    setword *set;
    setword *nd;
    int m;
    int sorted_position;
};

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
    int a_position = ((const struct SetAndNeighbourhood *) a)->sorted_position;
    int b_position = ((const struct SetAndNeighbourhood *) b)->sorted_position;
    return a_position - b_position;
}

// if filtered_STSs_len is small, use a simple filtering algorithm to
// avoid the overhead of the trie
#define MIN_LEN_FOR_TRIE 50

void make_STSs_helper(struct SetAndNeighbourhood *filtered_STSs, int filtered_STSs_len, struct hash_map *STSs_as_set,
        struct Bute *bute, struct Graph G, setword *possible_STS_roots, setword *union_of_subtrees, setword *nd_of_union_of_subtrees,
        int root_depth, struct hash_map *set_root, struct hash_map *new_STSs_hash_set)
{
    ++tmp_counter;
    FOR_EACH_IN_BITSET(w, possible_STS_roots, G.m)
        setword *adj_vv = get_copy_of_bitset(bute, nd_of_union_of_subtrees);
        bitset_addall(adj_vv, GRAPHROW(G.g, w, G.m), G.m);
        bitset_removeall(adj_vv, union_of_subtrees, G.m);
        DELELEMENT(adj_vv, w);
        if (popcount(adj_vv, G.m) < root_depth) {
            setword *STS = get_copy_of_bitset(bute, union_of_subtrees);
            ADDELEMENT(STS, w);
            if (intersection_is_empty(bute->vv_dominated_by[w], adj_vv, G.m) &&
                    intersection_is_empty(bute->vv_that_dominate[w], STS, G.m) &&
                    !hash_iselement(set_root, STS)) {
                hash_add(new_STSs_hash_set, STS, 1);
                hash_add(set_root, STS, w);
            }
            free_bitset(bute, STS);
        }
        free_bitset(bute, adj_vv);
    END_FOR_EACH_IN_BITSET

    struct Trie trie;
    if (filtered_STSs_len >= MIN_LEN_FOR_TRIE)
        trie_init(&trie, G.n, G.m, bute);
    int *almost_subset_end_positions = malloc(filtered_STSs_len * sizeof *almost_subset_end_positions);
    bool *STS_is_first_of_equal_nd_run = calloc(filtered_STSs_len, sizeof *STS_is_first_of_equal_nd_run);

    int *nd_arr = malloc((G.n+1) * sizeof *nd_arr);

    struct SetAndNeighbourhood *further_filtered_STSs = malloc(filtered_STSs_len * sizeof *further_filtered_STSs);

    for (int i=filtered_STSs_len-1; i>=0; i--) {
        setword *new_big_union = get_empty_bitset(bute);
        setword *s = filtered_STSs[i].set;
        setword *new_possible_STS_roots = get_copy_of_bitset(bute, possible_STS_roots);
        bitset_intersect_with(new_possible_STS_roots, filtered_STSs[i].nd, G.m);
        setword *new_union_of_subtrees = get_copy_of_bitset(bute, union_of_subtrees);
        bitset_addall(new_union_of_subtrees, s, G.m);

        setword *nd_of_new_union_of_subtrees = get_copy_of_bitset(bute, nd_of_union_of_subtrees);
        bitset_addall(nd_of_new_union_of_subtrees, filtered_STSs[i].nd, G.m);
        bitset_removeall(nd_of_new_union_of_subtrees, new_union_of_subtrees, G.m);

        setword *new_union_of_subtrees_and_nd = get_copy_of_bitset(bute, new_union_of_subtrees);
        bitset_addall(new_union_of_subtrees_and_nd, nd_of_new_union_of_subtrees, G.m);
        int further_filtered_STSs_len = 0;

        if (filtered_STSs_len >= MIN_LEN_FOR_TRIE) {
            int almost_subset_end_positions_len;
            trie_get_all_almost_subsets(&trie, filtered_STSs[i].nd, new_union_of_subtrees_and_nd, root_depth - popcount(filtered_STSs[i].nd, G.m),
                    almost_subset_end_positions, &almost_subset_end_positions_len);
            if (root_depth == popcount(filtered_STSs[i].nd, G.m)) {
                int it = i + 1;
                while (it < filtered_STSs_len && bitset_equals(filtered_STSs[i].nd, filtered_STSs[it].nd, G.m)) {
                    it++;
                }
                if (it > i + 1) {
                    almost_subset_end_positions[almost_subset_end_positions_len++] = it - 1;
                }
            }
            for (int j=0; j<almost_subset_end_positions_len; j++) {
                int iter = almost_subset_end_positions[j];
                struct SetAndNeighbourhood fl = filtered_STSs[iter];
                if (intersection_is_empty(fl.nd, new_possible_STS_roots, G.m) ||
                        popcount_of_union(nd_of_new_union_of_subtrees, fl.nd, G.m) > root_depth) {
                    continue;
                }
                for ( ; iter > i; iter--) {
                    struct SetAndNeighbourhood fl = filtered_STSs[iter];
                    if (intersection_is_empty(new_union_of_subtrees_and_nd, fl.set, G.m)) {
                        further_filtered_STSs[further_filtered_STSs_len++] = fl;
                        bitset_addall(new_big_union, fl.set, G.m);
                    }
                    if (STS_is_first_of_equal_nd_run[iter]) {
                        break;
                    }
                }
            }
        } else {
            for (int j=i+1; j<filtered_STSs_len; j++) {
                struct SetAndNeighbourhood fl = filtered_STSs[j];
                if (intersection_is_empty(fl.nd, new_possible_STS_roots, G.m))
                    continue;
                if (popcount_of_union(nd_of_new_union_of_subtrees, fl.nd, G.m) > root_depth)
                    continue;
                if (!intersection_is_empty(new_union_of_subtrees_and_nd, fl.set, G.m))
                    continue;
                further_filtered_STSs[further_filtered_STSs_len++] = fl;
                bitset_addall(new_big_union, fl.set, G.m);
            }
        }

        FOR_EACH_IN_BITSET(v, new_possible_STS_roots, G.m)
            setword *adj_vv = get_copy_of_bitset(bute, nd_of_new_union_of_subtrees);
            bitset_addall(adj_vv, GRAPHROW(G.g,v,G.m), G.m);
            bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
            DELELEMENT(adj_vv, v);
            bitset_removeall(adj_vv, new_big_union, G.m);
            if (popcount(adj_vv, G.m) >= root_depth || !bitset_union_is_superset(new_big_union, new_union_of_subtrees, bute->adj_vv_dominated_by[v], G.m)) {
                DELELEMENT(new_possible_STS_roots, v);
            }
            free_bitset(bute, adj_vv);
        END_FOR_EACH_IN_BITSET

        if (!isempty(new_possible_STS_roots, G.m)) {
            qsort(further_filtered_STSs, further_filtered_STSs_len, sizeof *further_filtered_STSs, cmp_sorted_position);
            make_STSs_helper(further_filtered_STSs, further_filtered_STSs_len, STSs_as_set,
                    bute, G, new_possible_STS_roots, new_union_of_subtrees, nd_of_new_union_of_subtrees, root_depth, set_root, new_STSs_hash_set);
        }

        free_bitset(bute, new_union_of_subtrees_and_nd);

        free_bitset(bute, nd_of_new_union_of_subtrees);
        free_bitset(bute, new_possible_STS_roots);

        free_bitset(bute, new_union_of_subtrees);
        free_bitset(bute, new_big_union);

        if (i == 0 || !bitset_equals(filtered_STSs[i].nd, filtered_STSs[i-1].nd, G.m)) {
            STS_is_first_of_equal_nd_run[i] = true;
        }
        if (filtered_STSs_len >= MIN_LEN_FOR_TRIE) {
            if (root_depth > popcount(filtered_STSs[i].nd, G.m)) {
                if (i == filtered_STSs_len-1 || !bitset_equals(filtered_STSs[i].nd, filtered_STSs[i+1].nd, G.m)) {
                    int pos = 0;
                    FOR_EACH_IN_BITSET(w, filtered_STSs[i].nd, G.m)
                        nd_arr[pos++] = w;
                    END_FOR_EACH_IN_BITSET
                    nd_arr[pos] = -1;
                    trie_add_key_val(&trie, nd_arr, filtered_STSs[i].nd, i);
                }
                // There's no need to consider a subtrie all of whose keys contain a vertex v that is
                // in the new union of sets or their neighbourhood
                trie_add_an_aux_bitset(&trie, nd_arr, filtered_STSs[i].set);
            }
        }
    }
    free(further_filtered_STSs);
    if (filtered_STSs_len >= MIN_LEN_FOR_TRIE)
        trie_destroy(&trie);
    free(almost_subset_end_positions);
    free(STS_is_first_of_equal_nd_run);
    free(nd_arr);
}

setword **make_STSs(setword **STSs, int STSs_len, struct Bute *bute, struct Graph G,
        int root_depth, struct hash_map *set_root, int *new_STSs_len)
{
    struct hash_map new_STSs_hash_set;
    hash_init(&new_STSs_hash_set, bute);
    struct hash_map STSs_as_set;
    hash_init(&STSs_as_set, bute);
    struct SetAndNeighbourhood *STSs_and_nds = malloc(STSs_len * sizeof *STSs_and_nds);
    for (int i=0; i<STSs_len; i++) {
        hash_add(&STSs_as_set, STSs[i], 1);
        setword *nd = get_bitset(bute);
        find_adjacent_vv(STSs[i], G, nd);
        STSs_and_nds[i] = (struct SetAndNeighbourhood) {STSs[i], nd, G.m};
    }

    qsort(STSs_and_nds, STSs_len, sizeof *STSs_and_nds, cmp_nd_popcount_desc);
    for (int i=0; i<STSs_len; i++) {
        STSs_and_nds[i].sorted_position = i;
    }

    setword *empty_set = get_empty_bitset(bute);
    setword *full_set = get_empty_bitset(bute);
    set_first_k_bits(full_set, G.n);

    make_STSs_helper(STSs_and_nds, STSs_len, &STSs_as_set,
            bute, G, full_set, empty_set, empty_set, root_depth, set_root, &new_STSs_hash_set);
    free_bitset(bute, empty_set);
    free_bitset(bute, full_set);

    for (int i=0; i<STSs_len; i++) {
        free_bitset(bute, STSs_and_nds[i].nd);
    }
    free(STSs_and_nds);
    setword **retval = hash_map_to_list(&new_STSs_hash_set);
    *new_STSs_len = new_STSs_hash_set.sz;
    hash_destroy(&new_STSs_hash_set);
    hash_destroy(&STSs_as_set);
    return retval;
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

    setword **STSs = malloc(G.n * sizeof *STSs);
    int STSs_len = 0;

    for (int root_depth=target; root_depth>=1; root_depth--) {
//        printf("target %d  root depth %d\n", target, root_depth);
//        printf(" %d\n", STSs_len);
        int new_STSs_len = 0;
        setword **new_STSs = make_STSs(STSs, STSs_len, bute, G, root_depth, &set_root, &new_STSs_len);
        if (new_STSs_len == 0) {
            free(new_STSs);
            break;
        }
        STSs = realloc(STSs, (STSs_len + new_STSs_len) * sizeof *STSs);
        for(int i=0; i<new_STSs_len; i++) {
            STSs[STSs_len + i] = new_STSs[i];
        }
        STSs_len = STSs_len + new_STSs_len;

        int k = 0;
        for (int i=0; i<STSs_len; i++) {
            setword *adj_vv = get_bitset(bute);
            find_adjacent_vv(STSs[i], G, adj_vv);
            if (popcount(adj_vv, G.m) < root_depth) {
                if (!retval && popcount(STSs[i], G.m) + popcount(adj_vv, G.m) == G.n) {
                    retval = true;
                    int parent_vertex = -1;
                    FOR_EACH_IN_BITSET(w, adj_vv, G.m)
                        parent[w] = parent_vertex;
                        parent_vertex = w;
                    END_FOR_EACH_IN_BITSET
                    add_parents(bute, parent, G, &set_root, STSs[i], parent_vertex);
                }
                STSs[k++] = STSs[i];
            } else {
                free_bitset(bute, STSs[i]);
            }
            free_bitset(bute, adj_vv);
        }
        STSs_len = k;
        free(new_STSs);
        if (retval) {
            break;
        }
    }
    
    for (int i=0; i<STSs_len; i++) {
        free_bitset(bute, STSs[i]);
    }
    free(STSs);
    hash_destroy(&set_root);
    return retval;
}

int optimise(struct Graph G, int *parent, struct Bute *bute)
{
    int target = 0;
    for ( ; target<=G.n; target++) {
//        printf("target %d\n", target);
        bool result = solve(bute, G, target, parent);
        if (result) {
            break;
        }
    }
    return target;
}

int main(int argc, char *argv[])
{
    struct Graph G = read_graph();
    struct Bute bute;
    Bute_init(&bute, G);
    int *parent = calloc(G.n, sizeof *parent);
    int treedepth = optimise(G, parent, &bute);

    printf("%d\n", treedepth);
    for (int i=0; i<G.n; i++) {
        printf("%d\n", parent[i] + 1);
    }
//    printf("tmp counter %ld\n", tmp_counter);

    free(parent);
    free(G.g);
    deallocate_Bitsets(&bute);
    Bute_destroy(&bute);
}
