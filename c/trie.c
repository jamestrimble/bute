// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

#include "trie.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

#define SMALL_SET_SIZE 2
#define SUBTREE_INTERSECTION(node) (trie->m <= SMALL_SET_SIZE ? node->small_sets : node->bitsets)
#define SUBTREE_INTERSECTION_OF_AUX_BITSETS(node) \
        (trie->m <= SMALL_SET_SIZE ? node->small_sets + SMALL_SET_SIZE : node->bitsets + trie->m)

struct TrieNode
{
    unsigned children_len;
    unsigned children_capacity;
    struct TrieNode *children;

    int key;
    size_t val;

    union {
        setword *bitsets;
        setword small_sets[SMALL_SET_SIZE * 2];
    };
};

#define NO_VALUE SIZE_MAX

void trie_node_init(struct Trie *trie, struct TrieNode *node, int key,
        setword *initial_subtrie_intersection,
        setword *initial_subtrie_intersection_of_aux_sets)
{
    node->key = key;
    node->val = NO_VALUE;
    node->children = NULL;
    node->children_len = 0;
    node->children_capacity = 0;
    if (trie->m > SMALL_SET_SIZE) {
        node->bitsets = bute_xmalloc(trie->m * 2 * sizeof(setword));
    }
    for (int i=0; i<trie->m; i++) {
        SUBTREE_INTERSECTION(node)[i] = initial_subtrie_intersection[i];
        SUBTREE_INTERSECTION_OF_AUX_BITSETS(node)[i] = initial_subtrie_intersection_of_aux_sets[i];
    }
}

void trie_init(struct Trie *trie, int n, int m, struct Bute *bute)
{
    trie->graph_n = n;
    trie->m = m;
    trie->bute = bute;
    setword * full_bitset = get_full_bitset(bute, trie->graph_n);
    trie->root = bute_xmalloc(sizeof(struct TrieNode) + 2 * trie->m * sizeof(setword));
    trie_node_init(trie, trie->root, -1, full_bitset, full_bitset);
    free_bitset(trie->bute, full_bitset);
}

void trie_node_destroy(struct Trie *trie, struct TrieNode *node)
{
    for (int i=0; i<node->children_len; i++) {
        trie_node_destroy(trie, &node->children[i]);
    }
    if (trie->m > SMALL_SET_SIZE) {
        free(node->bitsets);
    }
    free(node->children);
}

void trie_destroy(struct Trie *trie)
{
    trie_node_destroy(trie, trie->root);
    free(trie->root);
}

struct TrieNode *trie_get_child_node(struct Trie *trie, struct TrieNode *node, int key)
{
    for (int i=0; i<node->children_len; i++) {
        if (node->children[i].key == key) {
            return &node->children[i];
        }
    }
    return NULL;
}

void trie_get_all_almost_subsets_helper(struct Trie *trie, struct TrieNode *node, setword *set,
        setword *aux_set, int num_additions_permitted, int remaining_num_additions_permitted, size_t *arr_out, size_t *arr_out_len)
{
    if (popcount_of_set_difference(SUBTREE_INTERSECTION(node), set, trie->m) > num_additions_permitted) {
        return;
    }
    if (!intersection_is_empty(aux_set, SUBTREE_INTERSECTION_OF_AUX_BITSETS(node), trie->m)) {
        return;
    }
    if (node->val != NO_VALUE) {
        arr_out[(*arr_out_len)++] = node->val;
    }
    for (int i=0; i<node->children_len; i++) {
        struct TrieNode *child = &node->children[i];
        int new_remaining_num_additions_permitted = remaining_num_additions_permitted - !ISELEMENT(set, child->key);
        if (new_remaining_num_additions_permitted >= 0) {
            trie_get_all_almost_subsets_helper(trie, child, set, aux_set, num_additions_permitted,
                    new_remaining_num_additions_permitted, arr_out, arr_out_len);
        }
    }
}

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, size_t *arr_out, size_t *arr_out_len)
{
    *arr_out_len = 0;
    trie_get_all_almost_subsets_helper(trie, trie->root, set, aux_set, num_additions_permitted, num_additions_permitted, arr_out, arr_out_len);
}

struct TrieNode *trie_node_add_child(struct TrieNode *node)
{
    if (node->children_len == node->children_capacity) {
        node->children_capacity = node->children_capacity == 0 ? 1 :
                                  node->children_capacity == 1 ? 2 :
                                  node->children_capacity + node->children_capacity / 2;
        node->children = bute_xrealloc(node->children, node->children_capacity * sizeof(struct TrieNode));
    }
    ++node->children_len;
    return &node->children[node->children_len - 1];
}

void trie_add_element(struct Trie *trie, setword *key_bitset, setword *aux_bitset, int val)
{
    struct TrieNode *node = trie->root;
    bitset_intersect_with(SUBTREE_INTERSECTION(node), key_bitset, trie->m);
    bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(node), aux_bitset, trie->m);
    FOR_EACH_IN_BITSET(v, key_bitset, trie->m)
        struct TrieNode * child = trie_get_child_node(trie, node, v);
        if (child) {
            node = child;
            bitset_intersect_with(SUBTREE_INTERSECTION(node), key_bitset, trie->m);
            bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(node), aux_bitset, trie->m);
        } else {
            struct TrieNode *new_node = trie_node_add_child(node);
            trie_node_init(trie, new_node, v, key_bitset, aux_bitset);
            node = new_node;
        }
    END_FOR_EACH_IN_BITSET
    if (node->val == NO_VALUE) {   // if this leaf node did not already exist
        node->val = val;
    }
}
