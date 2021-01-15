// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

#include "trie.h"

#include <stdlib.h>

#define SUBTREE_INTERSECTION(trie, node) node->bitsets
#define SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node) (node->bitsets + trie->m)

struct TrieNode
{
    struct TrieNode *first_child;
    struct TrieNode *next_sibling;

    int key;
    int val;

    setword bitsets[];
};

void trie_node_init(struct Trie *trie, struct TrieNode *node, int key,
        setword *initial_subtrie_intersection,
        setword *initial_subtrie_intersection_of_aux_sets,
        struct TrieNode *next_sibling)
{
    node->key = key;
    node->val = -1;
    node->first_child = NULL;
    node->next_sibling = next_sibling;
    for (int i=0; i<trie->m; i++) {
        SUBTREE_INTERSECTION(trie, node)[i] = initial_subtrie_intersection[i];
        SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node)[i] = initial_subtrie_intersection_of_aux_sets[i];
    }
}

void trie_init(struct Trie *trie, int n, int m, struct Bute *bute)
{
    trie->graph_n = n;
    trie->m = m;
    trie->bute = bute;
    setword * full_bitset = get_full_bitset(bute, trie->graph_n);
    trie->root = malloc(sizeof *trie->root + 2 * trie->m * sizeof(setword));
    trie_node_init(trie, trie->root, -1, full_bitset, full_bitset, NULL);
}

void trie_node_destroy(struct Trie * trie, struct TrieNode * node)
{
    struct TrieNode *child = node->first_child;
    while (child) {
        struct TrieNode *next_sibling = child->next_sibling;
        trie_node_destroy(trie, child);
        free(child);
        child = next_sibling;
    }
}

void trie_destroy(struct Trie *trie)
{
    trie_node_destroy(trie, trie->root);
    free(trie->root);
}

struct TrieNode *trie_get_child_node(struct Trie *trie, struct TrieNode *node, int key)
{
    struct TrieNode *child = node->first_child;
    while (child) {
        if (child->key == key) {
            return child;
        }
        child = child->next_sibling;
    }
    return NULL;
}

void trie_get_all_almost_subsets_helper(struct Trie *trie, struct TrieNode *node, setword *set,
        setword *aux_set, int num_additions_permitted, int remaining_num_additions_permitted, int *arr_out, int *arr_out_len)
{
    if (popcount_of_set_difference(SUBTREE_INTERSECTION(trie, node), set, trie->m) > num_additions_permitted) {
        return;
    }
    if (!intersection_is_empty(aux_set, SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node), trie->m)) {
        return;
    }
    if (node->val != -1) {
        arr_out[(*arr_out_len)++] = node->val;
    }
    struct TrieNode *child = node->first_child;
    while (child) {
        int new_remaining_num_additions_permitted = remaining_num_additions_permitted - !ISELEMENT(set, child->key);
        if (new_remaining_num_additions_permitted >= 0) {
            trie_get_all_almost_subsets_helper(trie, child, set, aux_set, num_additions_permitted,
                    new_remaining_num_additions_permitted, arr_out, arr_out_len);
        }
        child = child->next_sibling;
    }
}

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len)
{
    *arr_out_len = 0;
    trie_get_all_almost_subsets_helper(trie, trie->root, set, aux_set, num_additions_permitted, num_additions_permitted, arr_out, arr_out_len);
}

void trie_add_element(struct Trie *trie, setword *key_bitset, setword *aux_bitset, int val)
{
    struct TrieNode *node = trie->root;
    bitset_intersect_with(SUBTREE_INTERSECTION(trie, node), key_bitset, trie->m);
    bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node), aux_bitset, trie->m);
    FOR_EACH_IN_BITSET(v, key_bitset, trie->m)
        struct TrieNode * child = trie_get_child_node(trie, node, v);
        if (child) {
            node = child;
            bitset_intersect_with(SUBTREE_INTERSECTION(trie, node), key_bitset, trie->m);
            bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node), aux_bitset, trie->m);
        } else {
            struct TrieNode *new_node = malloc(sizeof *new_node + 2 * trie->m * sizeof(setword));
            trie_node_init(trie, new_node, v, key_bitset, aux_bitset, node->first_child);
            node->first_child = new_node;
            node = new_node;
        }
    END_FOR_EACH_IN_BITSET
    if (node->val == -1) {   // if this leaf node did not already exist
        node->val = val;
    }
}
