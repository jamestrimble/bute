#include "bitset.h"
#include "bute.h"
#include "graph.h"
#include "hash_map.h"
#include "trie.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFERSIZE 1024

/*************************************/

// Returns a pointer to the first bitset in a linked list
struct Bitset *make_connected_components(setword *vv, struct Graph G, struct Bute *bute)
{
    struct Bitset *retval = NULL;
    setword *visited = get_empty_bitset(bute);
    setword *vv_in_prev_components = get_empty_bitset(bute);
    int *queue = malloc(G.n * sizeof *queue);
    FOR_EACH_IN_BITSET(v, vv, G.m)
        if (ISELEMENT(visited, v))
            continue;
        int queue_len = 0;
        ADDELEMENT(visited, v);
        queue[queue_len++] = v;
        while (queue_len) {
            int u = queue[--queue_len];
            FOR_EACH_IN_BITSET(w, GRAPHROW(G.g, u, G.m), G.m)
                if (ISELEMENT(vv, w) && !ISELEMENT(visited, w)) {
                    ADDELEMENT(visited, w);
                    queue[queue_len++] = w;
                }
            END_FOR_EACH_IN_BITSET
        }
        struct Bitset *bitset = get_Bitset(bute);
        bitset->next = retval;
        retval = bitset;
        setword *component = bitset->bitset;
        for (int k=0; k<G.m; k++)
            component[k] = visited[k] & ~vv_in_prev_components[k];

        bitset_addall(vv_in_prev_components, visited, G.m);
    END_FOR_EACH_IN_BITSET

    free_bitset(bute, vv_in_prev_components);
    free_bitset(bute, visited);
    free(queue);
    return retval;
}

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

// if filtered_leafysets_len is small, use a simple filtering algorithm to
// avoid the overhead of the trie
#define MIN_LEN_FOR_TRIE 50

void make_leafysets_helper(struct SetAndNeighbourhood *filtered_leafysets, int filtered_leafysets_len, struct hash_map *leafysets_as_set,
        struct Bute *bute, struct Graph G, setword *possible_leafyset_roots, setword *union_of_subtrees, setword *nd_of_union_of_subtrees,
        int root_depth, struct hash_map *set_root, struct hash_map *new_leafysets_hash_map)
{
    if (!filtered_leafysets_len) {
        return;
    }
    struct Trie trie;
    if (filtered_leafysets_len >= MIN_LEN_FOR_TRIE)
        trie_init(&trie, G.n, G.m, bute);
    int *almost_subset_end_positions = malloc(filtered_leafysets_len * sizeof *almost_subset_end_positions);
    bool *leafyset_is_first_of_equal_nd_run = calloc(filtered_leafysets_len, sizeof *leafyset_is_first_of_equal_nd_run);

    int *nd_arr = malloc((G.n+1) * sizeof *nd_arr);

    struct SetAndNeighbourhood *further_filtered_leafysets = malloc(filtered_leafysets_len * sizeof *further_filtered_leafysets);

    for (int i=filtered_leafysets_len-1; i>=0; i--) {
        setword *new_big_union = get_empty_bitset(bute);
        setword *s = filtered_leafysets[i].set;
        setword *new_possible_leafyset_roots = get_copy_of_bitset(bute, possible_leafyset_roots);
        bitset_intersect_with(new_possible_leafyset_roots, filtered_leafysets[i].nd, G.m);
        setword *new_union_of_subtrees = get_copy_of_bitset(bute, union_of_subtrees);
        bitset_addall(new_union_of_subtrees, s, G.m);

        setword *nd_of_new_union_of_subtrees = get_copy_of_bitset(bute, nd_of_union_of_subtrees);
        bitset_addall(nd_of_new_union_of_subtrees, filtered_leafysets[i].nd, G.m);
        bitset_removeall(nd_of_new_union_of_subtrees, new_union_of_subtrees, G.m);

        FOR_EACH_IN_BITSET(w, new_possible_leafyset_roots, G.m)
            setword *adj_vv = get_copy_of_bitset(bute, nd_of_new_union_of_subtrees);
            bitset_addall(adj_vv, GRAPHROW(G.g, w, G.m), G.m);
            bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
            if (popcount(adj_vv, G.m) <= root_depth) {
                setword *leafyset = get_copy_of_bitset(bute, new_union_of_subtrees);
                ADDELEMENT(leafyset, w);
                if (intersection_is_empty(bute->vv_dominated_by[w], adj_vv, G.m) &&
                        intersection_is_empty(bute->vv_that_dominate[w], leafyset, G.m) &&
                        !hash_iselement(set_root, leafyset)) {
                    hash_add(new_leafysets_hash_map, leafyset, 1);
                    hash_add(set_root, leafyset, w);
                }
                free_bitset(bute, leafyset);
            }
            free_bitset(bute, adj_vv);
        END_FOR_EACH_IN_BITSET

        setword *new_union_of_subtrees_and_nd = get_copy_of_bitset(bute, new_union_of_subtrees);
        bitset_addall(new_union_of_subtrees_and_nd, nd_of_new_union_of_subtrees, G.m);
        int further_filtered_leafysets_len = 0;

        if (filtered_leafysets_len >= MIN_LEN_FOR_TRIE) {
            int almost_subset_end_positions_len;
            trie_get_all_almost_subsets(&trie, filtered_leafysets[i].nd, new_union_of_subtrees_and_nd, root_depth - popcount(filtered_leafysets[i].nd, G.m),
                    almost_subset_end_positions, &almost_subset_end_positions_len);
            if (root_depth == popcount(filtered_leafysets[i].nd, G.m)) {
                int it = i + 1;
                while (it < filtered_leafysets_len && bitset_equals(filtered_leafysets[i].nd, filtered_leafysets[it].nd, G.m)) {
                    it++;
                }
                if (it > i + 1) {
                    almost_subset_end_positions[almost_subset_end_positions_len++] = it - 1;
                }
            }
            for (int j=0; j<almost_subset_end_positions_len; j++) {
                int iter = almost_subset_end_positions[j];
                struct SetAndNeighbourhood fl = filtered_leafysets[iter];
                if (intersection_is_empty(fl.nd, new_possible_leafyset_roots, G.m) ||
                        popcount_of_union(nd_of_new_union_of_subtrees, fl.nd, G.m) > root_depth) {
                    continue;
                }
                for ( ; iter > i; iter--) {
                    struct SetAndNeighbourhood fl = filtered_leafysets[iter];
                    if (intersection_is_empty(new_union_of_subtrees_and_nd, fl.set, G.m)) {
                        further_filtered_leafysets[further_filtered_leafysets_len++] = fl;
                        bitset_addall(new_big_union, fl.set, G.m);
                    }
                    if (leafyset_is_first_of_equal_nd_run[iter]) {
                        break;
                    }
                }
            }
        } else {
            for (int j=i+1; j<filtered_leafysets_len; j++) {
                struct SetAndNeighbourhood fl = filtered_leafysets[j];
                if (intersection_is_empty(fl.nd, new_possible_leafyset_roots, G.m))
                    continue;
                if (popcount_of_union(nd_of_new_union_of_subtrees, fl.nd, G.m) > root_depth)
                    continue;
                if (!intersection_is_empty(new_union_of_subtrees_and_nd, fl.set, G.m))
                    continue;
                further_filtered_leafysets[further_filtered_leafysets_len++] = fl;
                bitset_addall(new_big_union, fl.set, G.m);
            }
        }

        qsort(further_filtered_leafysets, further_filtered_leafysets_len, sizeof *further_filtered_leafysets, cmp_sorted_position);

        FOR_EACH_IN_BITSET(v, new_possible_leafyset_roots, G.m)
            setword *adj_vv = get_copy_of_bitset(bute, nd_of_new_union_of_subtrees);
            bitset_addall(adj_vv, GRAPHROW(G.g,v,G.m), G.m);
            bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
            DELELEMENT(adj_vv, v);
            bitset_removeall(adj_vv, new_big_union, G.m);
            if (popcount(adj_vv, G.m) >= root_depth || !bitset_union_is_superset(new_big_union, new_union_of_subtrees, bute->adj_vv_dominated_by[v], G.m)) {
                DELELEMENT(new_possible_leafyset_roots, v);
            }
            free_bitset(bute, adj_vv);
        END_FOR_EACH_IN_BITSET

        if (!isempty(new_possible_leafyset_roots, G.m)) {
            make_leafysets_helper(further_filtered_leafysets, further_filtered_leafysets_len, leafysets_as_set,
                    bute, G, new_possible_leafyset_roots, new_union_of_subtrees, nd_of_new_union_of_subtrees, root_depth, set_root, new_leafysets_hash_map);
        }

        free_bitset(bute, new_union_of_subtrees_and_nd);

        free_bitset(bute, nd_of_new_union_of_subtrees);
        free_bitset(bute, new_possible_leafyset_roots);

        free_bitset(bute, new_union_of_subtrees);
        free_bitset(bute, new_big_union);

        if (i == 0 || !bitset_equals(filtered_leafysets[i].nd, filtered_leafysets[i-1].nd, G.m)) {
            leafyset_is_first_of_equal_nd_run[i] = true;
        }
        if (filtered_leafysets_len >= MIN_LEN_FOR_TRIE) {
            if (root_depth > popcount(filtered_leafysets[i].nd, G.m)) {
                if (i == filtered_leafysets_len-1 || !bitset_equals(filtered_leafysets[i].nd, filtered_leafysets[i+1].nd, G.m)) {
                    int pos = 0;
                    FOR_EACH_IN_BITSET(w, filtered_leafysets[i].nd, G.m)
                        nd_arr[pos++] = w;
                    END_FOR_EACH_IN_BITSET
                    nd_arr[pos] = -1;
                    trie_add_key_val(&trie, nd_arr, filtered_leafysets[i].nd, i);
                }
                // There's no need to consider a subtrie all of whose keys contain a vertex v that is
                // in the new union of sets or their neighbourhood
                trie_add_an_aux_bitset(&trie, nd_arr, filtered_leafysets[i].set);
            }
        }
    }
    free(further_filtered_leafysets);
    if (filtered_leafysets_len >= MIN_LEN_FOR_TRIE)
        trie_destroy(&trie);
    free(almost_subset_end_positions);
    free(leafyset_is_first_of_equal_nd_run);
    free(nd_arr);
}

setword **make_leafysets(setword **leafysets, int leafysets_len, struct Bute *bute, struct Graph G,
        int root_depth, struct hash_map *set_root, int *new_leafysets_len)
{
    struct hash_map new_leafysets_hash_map;
    hash_init(&new_leafysets_hash_map, bute);
    struct hash_map leafysets_as_set;
    hash_init(&leafysets_as_set, bute);
    struct SetAndNeighbourhood *leafysets_and_nds = malloc(leafysets_len * sizeof *leafysets_and_nds);
    for (int i=0; i<leafysets_len; i++) {
        hash_add(&leafysets_as_set, leafysets[i], 1);
        setword *nd = get_bitset(bute);
        find_adjacent_vv(leafysets[i], G, nd);
        leafysets_and_nds[i] = (struct SetAndNeighbourhood) {leafysets[i], nd, G.m};
    }

    qsort(leafysets_and_nds, leafysets_len, sizeof *leafysets_and_nds, cmp_nd_popcount_desc);
    for (int i=0; i<leafysets_len; i++) {
        leafysets_and_nds[i].sorted_position = i;
    }

    for (int v=0; v<G.n; v++) {
        setword *single_vtx_leafyset = get_empty_bitset(bute);
        ADDELEMENT(single_vtx_leafyset, v);
        if (popcount(GRAPHROW(G.g, v, G.m), G.m) < root_depth && isempty(bute->adj_vv_dominated_by[v], G.m)
                && !hash_iselement(set_root, single_vtx_leafyset)) {
            hash_add(&new_leafysets_hash_map, single_vtx_leafyset, 1);
            hash_add(set_root, single_vtx_leafyset, v);
        }
        free_bitset(bute, single_vtx_leafyset);
    }

    setword *empty_set = get_empty_bitset(bute);
    setword *full_set = get_bitset(bute);
    set_first_k_bits(full_set, G.n);
    struct SetAndNeighbourhood *filtered_leafysets = malloc(leafysets_len * sizeof *filtered_leafysets);
    int filtered_leafysets_len = 0;
    for (int i=0; i<leafysets_len; i++) {
        filtered_leafysets[filtered_leafysets_len++] = leafysets_and_nds[i];
    }

    make_leafysets_helper(filtered_leafysets, filtered_leafysets_len, &leafysets_as_set,
            bute, G, full_set, empty_set, empty_set, root_depth, set_root, &new_leafysets_hash_map);
    free(filtered_leafysets);
    free_bitset(bute, empty_set);
    free_bitset(bute, full_set);

    for (int i=0; i<leafysets_len; i++) {
        free_bitset(bute, leafysets_and_nds[i].nd);
    }
    free(leafysets_and_nds);
    setword **retval = hash_map_to_list(&new_leafysets_hash_map);
    *new_leafysets_len = new_leafysets_hash_map.sz;
    hash_destroy(&new_leafysets_hash_map);
    hash_destroy(&leafysets_as_set);
    return retval;
}

void add_parents(struct Bute *bute, int *parent, struct Graph G, struct hash_map *set_root, setword *s, int parent_vertex)
{
    int v;
    if (!hash_get_val(set_root, s, &v)) {
        printf("Something went wrong.\n");
        exit(1);
    }
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

    setword **leafysets = malloc(G.n * sizeof *leafysets);
    int leafysets_len = 0;

    for (int root_depth=target; root_depth>=1; root_depth--) {
//        printf("root depth %d\n", root_depth);
//        printf(" %d\n", leafysets_len);
        int new_leafysets_len = 0;
        setword **new_leafysets = make_leafysets(leafysets, leafysets_len, bute, G, root_depth, &set_root, &new_leafysets_len);
        if (new_leafysets_len == 0) {
            free(new_leafysets);
            break;
        }
        leafysets = realloc(leafysets, (leafysets_len + new_leafysets_len) * sizeof *leafysets);
        for(int i=0; i<new_leafysets_len; i++) {
            leafysets[leafysets_len + i] = new_leafysets[i];
        }
        leafysets_len = leafysets_len + new_leafysets_len;

        int k = 0;
        for (int i=0; i<leafysets_len; i++) {
            setword *adj_vv = get_bitset(bute);
            find_adjacent_vv(leafysets[i], G, adj_vv);
            if (popcount(adj_vv, G.m) < root_depth) {
                if (!retval && popcount(leafysets[i], G.m) + popcount(adj_vv, G.m) == G.n) {
                    retval = true;
                    int parent_vertex = -1;
                    FOR_EACH_IN_BITSET(w, adj_vv, G.m)
                        parent[w] = parent_vertex;
                        parent_vertex = w;
                    END_FOR_EACH_IN_BITSET
                    add_parents(bute, parent, G, &set_root, leafysets[i], parent_vertex);
                }
                leafysets[k++] = leafysets[i];
            } else {
                free_bitset(bute, leafysets[i]);
            }
            free_bitset(bute, adj_vv);
        }
        leafysets_len = k;
        free(new_leafysets);
        if (retval) {
            break;
        }
    }
    
    for (int i=0; i<leafysets_len; i++) {
        free_bitset(bute, leafysets[i]);
    }
    free(leafysets);
    hash_destroy(&set_root);
    return retval;
}

struct Graph read_graph()
{
    char s[BUFFERSIZE], s1[32], s2[32];
    int n, edge_count;
    int m = 0;   // number of setwords needed to store n bits
    int v, w;
    int num_edges_read = 0;
    graph *g = NULL;
    while (true) {
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
            m = SETWORDSNEEDED(n);
            g = calloc(n * m, sizeof(graph));
            break;
        default:
            if (sscanf(s, "%d %d", &v, &w) != 2)
                exit(1);
            if (v == w)
                continue;
            --v;
            --w;
            ADDONEEDGE(g, v, w, m);
            ++num_edges_read;
        }
    }
    return (struct Graph) {g, n, m};
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

    free(parent);
    free(G.g);
    deallocate_Bitsets(&bute);
    Bute_destroy(&bute);
}
