// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

#include "trie.h"
#include "util.h"

#include <stdlib.h>

#define SUBTREE_INTERSECTION(trie, node) node->bitsets
#define SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node) (node->bitsets + trie->m)

struct TrieNode
{
    unsigned char *children;
    int children_len;

    int key;
    int val;

    setword bitsets[];
};

void trie_node_init(struct Trie *trie, struct TrieNode *node, int key,
        setword *initial_subtrie_intersection,
        setword *initial_subtrie_intersection_of_aux_sets)
{
    node->key = key;
    node->val = -1;
    node->children = NULL;
    node->children_len = 0;
    for (int i=0; i<trie->m; i++) {
        SUBTREE_INTERSECTION(trie, node)[i] = initial_subtrie_intersection[i];
        SUBTREE_INTERSECTION_OF_AUX_BITSETS(trie, node)[i] = initial_subtrie_intersection_of_aux_sets[i];
    }
}

void trie_init(struct Trie *trie, int n, int m, struct Bute *bute)
{
    trie->graph_n = n;
    trie->m = m;
    trie->node_size = sizeof(struct TrieNode) + 2 * trie->m * sizeof(setword);
    trie->bute = bute;
    setword * full_bitset = get_full_bitset(bute, trie->graph_n);
    trie->root = bute_xmalloc(sizeof(struct TrieNode) + 2 * trie->m * sizeof(setword));
    trie_node_init(trie, trie->root, -1, full_bitset, full_bitset);
    free_bitset(trie->bute, full_bitset);
}

void trie_node_destroy(struct Trie *trie, struct TrieNode *node)
{
    for (int i=0; i<node->children_len; i++) {
        struct TrieNode *child =
                (struct TrieNode *) (node->children + i * trie->node_size);
        trie_node_destroy(trie, child);
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
    unsigned char *p = node->children;
    for (int i=0; i<node->children_len; i++) {
        struct TrieNode *child = (struct TrieNode *) p;
        if (child->key == key) {
            return child;
        }
        p += trie->node_size;
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
    unsigned char *p = node->children;
    for (int i=0; i<node->children_len; i++) {
        struct TrieNode *child = (struct TrieNode *) p;
        int new_remaining_num_additions_permitted = remaining_num_additions_permitted - !ISELEMENT(set, child->key);
        if (new_remaining_num_additions_permitted >= 0) {
            trie_get_all_almost_subsets_helper(trie, child, set, aux_set, num_additions_permitted,
                    new_remaining_num_additions_permitted, arr_out, arr_out_len);
        }
        p += trie->node_size;
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
            ++node->children_len;
            node->children = bute_xrealloc(node->children, node->children_len * trie->node_size);
            struct TrieNode *new_node = (struct TrieNode *) (node->children + (node->children_len - 1) * trie->node_size);
            trie_node_init(trie, new_node, v, key_bitset, aux_bitset);
            node = new_node;
        }
    END_FOR_EACH_IN_BITSET
    if (node->val == -1) {   // if this leaf node did not already exist
        node->val = val;
    }
}
