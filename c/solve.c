#include "bitset.h"
#include "hash_map.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFERSIZE 1024

int m = 0;

//https://stackoverflow.com/a/10380191/3347737
#define PASTE_HELPER(a,b) a ## b
#define PASTE(a,b) PASTE_HELPER(a,b)

// If you use these macros, don't modify bitset while iterating over it!
#define FOR_EACH_IN_BITSET_HELPER(v, bitset, m, i, sw, x) \
           for (int i=0;i<m;i++) {setword sw=bitset[i]; while (sw) {int x; TAKEBIT(x, sw); int v=i*WORDSIZE+x;
#define FOR_EACH_IN_BITSET(v, bitset, m) \
           FOR_EACH_IN_BITSET_HELPER(v, bitset, m, PASTE(i,__LINE__), PASTE(sw,__LINE__), PASTE(x,__LINE__))
#define END_FOR_EACH_IN_BITSET }}

/* We have a free-list of bitsets */

struct Bitset
{
    struct Bitset *next;
    setword bitset[];
};

struct Bitset *bitset_free_list_head = NULL;

struct Bitset *get_Bitset()
{
    if (bitset_free_list_head == NULL) {
        struct Bitset *b = malloc(sizeof(struct Bitset) + m * sizeof(setword));
        if (b == NULL)
            exit(1);
        b->next = NULL;
        bitset_free_list_head = b;
    }
    struct Bitset *b = bitset_free_list_head;
    bitset_free_list_head = b->next;
    return b;
}

setword *get_bitset()
{
    return get_Bitset()->bitset;
}

setword *get_empty_bitset()
{
    setword *b = get_bitset();
    for (int i=0; i<m; i++)
        b[i] = 0;
    return b;
}

setword *get_copy_of_bitset(setword const *vv)
{
    setword *b = get_bitset();
    for (int i=0; i<m; i++)
        b[i] = vv[i];
    return b;
}

void free_Bitset(struct Bitset *b)
{
    b->next = bitset_free_list_head;
    bitset_free_list_head = b;
}

void free_bitset(setword *bitset)
{
    struct Bitset *b = (struct Bitset *)((char *) bitset - offsetof(struct Bitset, bitset));
    free_Bitset(b);
}

void free_Bitsets(struct Bitset *b)
{
    while (b) {
        struct Bitset *next_to_free = b->next;
        free_Bitset(b);
        b = next_to_free;
    }
}

void deallocate_Bitsets()
{
    while (bitset_free_list_head) {
        struct Bitset *next_to_free = bitset_free_list_head->next;
        free(bitset_free_list_head);
        bitset_free_list_head = next_to_free;
    }
}

/** Trie *****************************/

// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

struct TrieNode
{
    int *successor_node_num;

    // The intersection of sets in the subtree below this node.
    // To simplify the code, there is a special case: initially, for the root node,
    // this contains the set of all vertices.
    setword *subtree_intersection;

    setword *subtree_intersection_of_aux_bitsets;

    int key;
    int successor_len;
    int val;
};

struct Trie
{
    struct TrieNode *nodes;
    struct TrieNode root;
    int nodes_len;
    int graph_n;
    int m;
};

void trie_node_init(struct TrieNode *node, int key)
{
    node->key = key;
    node->val = -1;
    node->successor_node_num = NULL;
    node->successor_len = 0;
}

void trie_init(struct Trie *trie, int n, int m)
{
    trie->graph_n = n;
    trie->m = m;
    trie->nodes_len = 0;
    trie->nodes = NULL;

    trie_node_init(&trie->root, -1);
    trie->root.subtree_intersection = get_bitset();
    trie->root.subtree_intersection_of_aux_bitsets = get_bitset();
    set_first_k_bits(trie->root.subtree_intersection, trie->graph_n);
    set_first_k_bits(trie->root.subtree_intersection_of_aux_bitsets, trie->graph_n);
}

void trie_destroy(struct Trie *trie)
{
    for (int i=0; i<trie->nodes_len; i++) {
        struct TrieNode *node = &trie->nodes[i];
        free(node->successor_node_num);
        free_bitset(node->subtree_intersection);
        free_bitset(node->subtree_intersection_of_aux_bitsets);
    }
    free(trie->root.successor_node_num);
    free_bitset(trie->root.subtree_intersection);
    free_bitset(trie->root.subtree_intersection_of_aux_bitsets);
    free(trie->nodes);
}

int trie_get_successor_node_num(struct Trie *trie, struct TrieNode *node, int key)
{
    for (int i=0; i<node->successor_len; i++) {
        int succ_node_num = node->successor_node_num[i];
        struct TrieNode *succ_node = &trie->nodes[succ_node_num];
        if (succ_node->key == key) {
            return succ_node_num;
        }
    }
    return -1;
}

struct TrieNode * trie_add_successor(struct Trie *trie, struct TrieNode *node, int key)
{
    if (node->successor_len == 0) {
        node->successor_node_num = malloc(sizeof *node->successor_node_num);
    } else if (__builtin_popcount(node->successor_len) == 1) {
        node->successor_node_num = realloc(node->successor_node_num, node->successor_len * 2 * sizeof *node->successor_node_num);
    }
    node->successor_node_num[node->successor_len++] = trie->nodes_len;

    if (trie->nodes_len == 0) {
        trie->nodes = malloc(1 * sizeof *trie->nodes);
    } else if (__builtin_popcount(trie->nodes_len) == 1) {
        trie->nodes = realloc(trie->nodes, trie->nodes_len * 2 * sizeof *trie->nodes);
    }
    struct TrieNode *succ = &trie->nodes[trie->nodes_len++];

    trie_node_init(succ, key);

    return succ;
}

void trie_get_all_almost_subsets_helper(struct Trie *trie, struct TrieNode *node, setword *set,
        setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len)
{
    if (node->val != -1) {
        arr_out[(*arr_out_len)++] = node->val;
    }
    for (int i=0; i<node->successor_len; i++) {
        int succ_node_num = node->successor_node_num[i];
        struct TrieNode *succ = &trie->nodes[succ_node_num];
        if (popcount_of_set_difference(succ->subtree_intersection, set, trie->m) <= num_additions_permitted &&
                intersection_is_empty(aux_set, succ->subtree_intersection_of_aux_bitsets, trie->m)) {
            trie_get_all_almost_subsets_helper(trie, succ, set, aux_set, num_additions_permitted,
                    arr_out, arr_out_len);
        }
    }
}

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len)
{
    *arr_out_len = 0;
    trie_get_all_almost_subsets_helper(trie, &trie->root, set, aux_set, num_additions_permitted, arr_out, arr_out_len);
}

// key is an array terminated by -1
void trie_add_key_val(struct Trie *trie, int *key, setword *key_bitset, int val)
{
    struct TrieNode *node = &trie->root;
    bitset_intersect_with(node->subtree_intersection, key_bitset, trie->m);
    while (*key != -1) {
        int succ_node_num = trie_get_successor_node_num(trie, node, *key);
        if (succ_node_num != -1) {
            node = &trie->nodes[succ_node_num];
        } else {
            node = trie_add_successor(trie, node, *key);
            node->subtree_intersection = get_bitset();
            node->subtree_intersection_of_aux_bitsets = get_bitset();
            set_first_k_bits(node->subtree_intersection, trie->graph_n);
            set_first_k_bits(node->subtree_intersection_of_aux_bitsets, trie->graph_n);
        }
        bitset_intersect_with(node->subtree_intersection, key_bitset, trie->m);
        ++key;
    }
    node->val = val;
}

void trie_add_an_aux_bitset(struct Trie *trie, int *key, setword *aux_bitset)
{
    struct TrieNode *node = &trie->root;
    bitset_intersect_with(node->subtree_intersection_of_aux_bitsets, aux_bitset, trie->m);
    while (*key != -1) {
        int succ_node_num = trie_get_successor_node_num(trie, node, *key);
        node = &trie->nodes[succ_node_num];
        bitset_intersect_with(node->subtree_intersection_of_aux_bitsets, aux_bitset, trie->m);
        ++key;
    }
}


/*************************************/

struct Graph
{
    graph *g;
    int n;
    int m;   // number of words needed for a bitset containing n elements
};

/*************************************/

struct Dom
{
    setword **vv_dominated_by;
    setword **vv_that_dominate;
    setword **adj_vv_dominated_by;
    int n;
};

void Dom_init(struct Dom *dom, struct Graph G)
{
    dom->n = G.n;
    dom->adj_vv_dominated_by = malloc(G.n * sizeof *dom->adj_vv_dominated_by);
    dom->vv_dominated_by = malloc(G.n * sizeof *dom->vv_dominated_by);
    dom->vv_that_dominate = malloc(G.n * sizeof *dom->vv_that_dominate);
    for (int v=0; v<G.n; v++) {
        dom->adj_vv_dominated_by[v] = get_empty_bitset();
        dom->vv_dominated_by[v] = get_empty_bitset();
        dom->vv_that_dominate[v] = get_empty_bitset();
    }
    for (int v=0; v<G.n; v++) {
        for (int w=0; w<G.n; w++) {
            if (w != v) {
                setword *nd_of_v_and_v_and_w = get_copy_of_bitset(GRAPHROW(G.g, v, G.m));
                ADDELEMENT(nd_of_v_and_v_and_w, v);
                ADDELEMENT(nd_of_v_and_v_and_w, w);
                setword *nd_of_w_and_v_and_w = get_copy_of_bitset(GRAPHROW(G.g, w, G.m));
                ADDELEMENT(nd_of_w_and_v_and_w, v);
                ADDELEMENT(nd_of_w_and_v_and_w, w);
                if (bitset_is_superset(nd_of_w_and_v_and_w, nd_of_v_and_v_and_w, G.m)) {
                    if (!bitset_equals(nd_of_v_and_v_and_w, nd_of_w_and_v_and_w, G.m) || v < w) {
                        ADDELEMENT(dom->vv_dominated_by[w], v);
                        ADDELEMENT(dom->vv_that_dominate[v], w);
                        if (ISELEMENT(GRAPHROW(G.g, w, G.m), v)) {
                            ADDELEMENT(dom->adj_vv_dominated_by[w], v);
                        }
                    }
                }
                free_bitset(nd_of_w_and_v_and_w);
                free_bitset(nd_of_v_and_v_and_w);
            }
        }
    }
}

void Dom_destroy(struct Dom *dom)
{
    for (int i=0; i<dom->n; i++) {
        free_bitset(dom->adj_vv_dominated_by[i]);
        free_bitset(dom->vv_dominated_by[i]);
        free_bitset(dom->vv_that_dominate[i]);
    }
    free(dom->adj_vv_dominated_by);
    free(dom->vv_dominated_by);
    free(dom->vv_that_dominate);
}

/*************************************/

// Returns a pointer to the first bitset in a linked list
struct Bitset *make_connected_components(setword *vv, struct Graph G)
{
    struct Bitset *retval = NULL;
    setword *visited = get_empty_bitset();
    setword *vv_in_prev_components = get_empty_bitset();
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
        struct Bitset *bitset = get_Bitset();
        bitset->next = retval;
        retval = bitset;
        setword *component = bitset->bitset;
        for (int k=0; k<G.m; k++)
            component[k] = visited[k] & ~vv_in_prev_components[k];

        bitset_addall(vv_in_prev_components, visited, G.m);
    END_FOR_EACH_IN_BITSET

    free_bitset(vv_in_prev_components);
    free_bitset(visited);
    free(queue);
    return retval;
}

void find_adjacent_vv(setword *s, struct Graph G, setword *adj_vv)
{
    clear_bitset(adj_vv, G.m);
    FOR_EACH_IN_BITSET(v, s, G.m)
        for (int k=0; k<G.m; k++) {
            adj_vv[k] |= GRAPHROW(G.g, v, G.m)[k];
        }
    END_FOR_EACH_IN_BITSET
    for (int k=0; k<G.m; k++) {
        adj_vv[k] &= ~s[k];
    }
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
        struct Graph G, struct Dom *dom, setword *possible_leafyset_roots, setword *union_of_subtrees, setword *nd_of_union_of_subtrees,
        int root_depth, struct hash_map *set_root, struct hash_map *new_leafysets_hash_map)
{
    if (!filtered_leafysets_len) {
        return;
    }
    struct Trie trie;
    if (filtered_leafysets_len >= MIN_LEN_FOR_TRIE)
        trie_init(&trie, G.n, G.m);
    int *almost_subset_end_positions = malloc(filtered_leafysets_len * sizeof *almost_subset_end_positions);
    bool *leafyset_is_first_of_equal_nd_run = calloc(filtered_leafysets_len, sizeof *leafyset_is_first_of_equal_nd_run);

    int *nd_arr = malloc((G.n+1) * sizeof *nd_arr);

    struct SetAndNeighbourhood *further_filtered_leafysets = malloc(filtered_leafysets_len * sizeof *further_filtered_leafysets);

    for (int i=filtered_leafysets_len-1; i>=0; i--) {
        setword *new_big_union = get_empty_bitset();
        setword *s = filtered_leafysets[i].set;
        setword *new_possible_leafyset_roots = get_copy_of_bitset(possible_leafyset_roots);
        bitset_intersect_with(new_possible_leafyset_roots, filtered_leafysets[i].nd, G.m);
        setword *new_union_of_subtrees = get_copy_of_bitset(union_of_subtrees);
        bitset_addall(new_union_of_subtrees, s, G.m);

        setword *nd_of_new_union_of_subtrees = get_copy_of_bitset(nd_of_union_of_subtrees);
        bitset_addall(nd_of_new_union_of_subtrees, filtered_leafysets[i].nd, G.m);
        bitset_removeall(nd_of_new_union_of_subtrees, new_union_of_subtrees, G.m);

        FOR_EACH_IN_BITSET(w, new_possible_leafyset_roots, G.m)
            setword *adj_vv = get_copy_of_bitset(nd_of_new_union_of_subtrees);
            bitset_addall(adj_vv, GRAPHROW(G.g, w, G.m), G.m);
            bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
            if (popcount(adj_vv, G.m) <= root_depth) {
                setword *leafyset = get_copy_of_bitset(new_union_of_subtrees);
                ADDELEMENT(leafyset, w);
                if (intersection_is_empty(dom->vv_dominated_by[w], adj_vv, G.m) &&
                        intersection_is_empty(dom->vv_that_dominate[w], leafyset, G.m) &&
                        !hash_iselement(set_root, leafyset)) {
                    hash_add(new_leafysets_hash_map, leafyset, 1);
                    hash_add(set_root, leafyset, w);
                }
                free_bitset(leafyset);
            }
            free_bitset(adj_vv);
        END_FOR_EACH_IN_BITSET

        setword *new_union_of_subtrees_and_nd = get_copy_of_bitset(new_union_of_subtrees);
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
            setword *adj_vv = get_copy_of_bitset(nd_of_new_union_of_subtrees);
            bitset_addall(adj_vv, GRAPHROW(G.g,v,G.m), G.m);
            bitset_removeall(adj_vv, new_union_of_subtrees, G.m);
            DELELEMENT(adj_vv, v);
            bitset_removeall(adj_vv, new_big_union, G.m);
            if (popcount(adj_vv, G.m) >= root_depth || !bitset_union_is_superset(new_big_union, new_union_of_subtrees, dom->adj_vv_dominated_by[v], G.m)) {
                DELELEMENT(new_possible_leafyset_roots, v);
            }
            free_bitset(adj_vv);
        END_FOR_EACH_IN_BITSET

        if (!isempty(new_possible_leafyset_roots, G.m)) {
            make_leafysets_helper(further_filtered_leafysets, further_filtered_leafysets_len, leafysets_as_set,
                    G, dom, new_possible_leafyset_roots, new_union_of_subtrees, nd_of_new_union_of_subtrees, root_depth, set_root, new_leafysets_hash_map);
        }

        free_bitset(new_union_of_subtrees_and_nd);

        free_bitset(nd_of_new_union_of_subtrees);
        free_bitset(new_possible_leafyset_roots);

        free_bitset(new_union_of_subtrees);
        free_bitset(new_big_union);

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

setword **make_leafysets(setword **leafysets, int leafysets_len, struct Graph G,
        struct Dom *dom,
        int root_depth, struct hash_map *set_root, int *new_leafysets_len)
{
    struct hash_map new_leafysets_hash_map;
    hash_init(&new_leafysets_hash_map, G.m);
    struct hash_map leafysets_as_set;
    hash_init(&leafysets_as_set, G.m);
    struct SetAndNeighbourhood *leafysets_and_nds = malloc(leafysets_len * sizeof *leafysets_and_nds);
    for (int i=0; i<leafysets_len; i++) {
        hash_add(&leafysets_as_set, leafysets[i], 1);
        setword *nd = get_bitset();
        find_adjacent_vv(leafysets[i], G, nd);
        leafysets_and_nds[i] = (struct SetAndNeighbourhood) {leafysets[i], nd, G.m};
    }

    qsort(leafysets_and_nds, leafysets_len, sizeof *leafysets_and_nds, cmp_nd_popcount_desc);
    for (int i=0; i<leafysets_len; i++) {
        leafysets_and_nds[i].sorted_position = i;
    }

    for (int v=0; v<G.n; v++) {
        setword *single_vtx_leafyset = get_empty_bitset();
        ADDELEMENT(single_vtx_leafyset, v);
        if (popcount(GRAPHROW(G.g, v, G.m), G.m) < root_depth && isempty(dom->adj_vv_dominated_by[v], G.m)
                && !hash_iselement(set_root, single_vtx_leafyset)) {
            hash_add(&new_leafysets_hash_map, single_vtx_leafyset, 1);
            hash_add(set_root, single_vtx_leafyset, v);
        }
        free_bitset(single_vtx_leafyset);
    }

    setword *empty_set = get_empty_bitset();
    setword *full_set = get_bitset();
    set_first_k_bits(full_set, G.n);
    struct SetAndNeighbourhood *filtered_leafysets = malloc(leafysets_len * sizeof *filtered_leafysets);
    int filtered_leafysets_len = 0;
    for (int i=0; i<leafysets_len; i++) {
        filtered_leafysets[filtered_leafysets_len++] = leafysets_and_nds[i];
    }

    make_leafysets_helper(filtered_leafysets, filtered_leafysets_len, &leafysets_as_set,
            G, dom, full_set, empty_set, empty_set, root_depth, set_root, &new_leafysets_hash_map);
    free(filtered_leafysets);
    free_bitset(empty_set);
    free_bitset(full_set);

    for (int i=0; i<leafysets_len; i++) {
        free_bitset(leafysets_and_nds[i].nd);
    }
    free(leafysets_and_nds);
    setword **retval = hash_map_to_list(&new_leafysets_hash_map, get_bitset);
    *new_leafysets_len = new_leafysets_hash_map.sz;
    hash_destroy(&new_leafysets_hash_map);
    hash_destroy(&leafysets_as_set);
    return retval;
}

void add_parents(int *parent, struct Graph G, struct hash_map *set_root, setword *s, int parent_vertex)
{
    int v;
    if (!hash_get_val(set_root, s, &v)) {
        printf("Something went wrong.\n");
        exit(1);
    }
    parent[v] = parent_vertex;
    setword *vv_in_child_subtrees = get_copy_of_bitset(s);
    DELELEMENT(vv_in_child_subtrees, v);
    struct Bitset * components = make_connected_components(vv_in_child_subtrees, G);
    for (struct Bitset *component=components; component!=NULL; component=component->next) {
        add_parents(parent, G, set_root, component->bitset, v);
    }
    free_Bitsets(components);
    free_bitset(vv_in_child_subtrees);
}

bool solve(struct Graph G, struct Dom *dom, int target, int *parent)
{
    bool retval = false;
    struct hash_map set_root;
    hash_init(&set_root, G.m);

    setword **leafysets = malloc(G.n * sizeof *leafysets);
    int leafysets_len = 0;

    for (int root_depth=target; root_depth>=1; root_depth--) {
//        printf("root depth %d\n", root_depth);
//        printf(" %d\n", leafysets_len);
        int new_leafysets_len = 0;
        setword **new_leafysets = make_leafysets(leafysets, leafysets_len, G, dom, root_depth, &set_root, &new_leafysets_len);
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
            setword *adj_vv = get_bitset();
            find_adjacent_vv(leafysets[i], G, adj_vv);
            if (popcount(adj_vv, G.m) < root_depth) {
                if (!retval && popcount(leafysets[i], G.m) + popcount(adj_vv, G.m) == G.n) {
                    retval = true;
                    int parent_vertex = -1;
                    FOR_EACH_IN_BITSET(w, adj_vv, G.m)
                        parent[w] = parent_vertex;
                        parent_vertex = w;
                    END_FOR_EACH_IN_BITSET
                    add_parents(parent, G, &set_root, leafysets[i], parent_vertex);
                }
                leafysets[k++] = leafysets[i];
            } else {
                free_bitset(leafysets[i]);
            }
            free_bitset(adj_vv);
        }
        leafysets_len = k;
        free(new_leafysets);
        if (retval) {
            break;
        }
    }
    
    for (int i=0; i<leafysets_len; i++) {
        free_bitset(leafysets[i]);
    }
    free(leafysets);
    hash_destroy(&set_root);
    return retval;
}

struct Graph read_graph()
{
    char s[BUFFERSIZE], s1[32], s2[32];
    int n, edge_count;
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

int optimise(struct Graph G, int *parent)
{
    struct Dom dom;
    Dom_init(&dom, G);

    int target = 0;
    for ( ; target<=G.n; target++) {
//        printf("target %d\n", target);
        bool result = solve(G, &dom, target, parent);
        if (result) {
            break;
        }
    }
    Dom_destroy(&dom);
    return target;
}

int main(int argc, char *argv[])
{
    struct Graph G = read_graph();
    int *parent = calloc(G.n, sizeof *parent);
    int treedepth = optimise(G, parent);

    printf("%d\n", treedepth);
    for (int i=0; i<G.n; i++) {
        printf("%d\n", parent[i] + 1);
    }

    free(parent);
    free(G.g);
    deallocate_Bitsets();
}
