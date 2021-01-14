// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

#include "trie.h"

#include <stdlib.h>

void trie_node_init(struct TrieNode *node, int key)
{
    node->key = key;
    node->val = -1;
    node->successor_node_num = NULL;
    node->successor_len = 0;
}

void trie_init(struct Trie *trie, int n, int m, struct Bute *bute)
{
    trie->graph_n = n;
    trie->m = m;
    trie->nodes_len = 0;
    trie->nodes = NULL;
    trie->bute = bute;

    trie_node_init(&trie->root, -1);
    trie->root.subtree_intersection = get_bitset(bute);
    trie->root.subtree_intersection_of_aux_bitsets = get_bitset(bute);
    set_first_k_bits(trie->root.subtree_intersection, trie->graph_n);
    set_first_k_bits(trie->root.subtree_intersection_of_aux_bitsets, trie->graph_n);
}

void trie_destroy(struct Trie *trie)
{
    for (int i=0; i<trie->nodes_len; i++) {
        struct TrieNode *node = &trie->nodes[i];
        free(node->successor_node_num);
        free_bitset(trie->bute, node->subtree_intersection);
        free_bitset(trie->bute, node->subtree_intersection_of_aux_bitsets);
    }
    free(trie->root.successor_node_num);
    free_bitset(trie->bute, trie->root.subtree_intersection);
    free_bitset(trie->bute, trie->root.subtree_intersection_of_aux_bitsets);
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
        setword *aux_set, int num_additions_permitted, int remaining_num_additions_permitted, int *arr_out, int *arr_out_len)
{
    if (popcount_of_set_difference(node->subtree_intersection, set, trie->m) > num_additions_permitted) {
        return;
    }
    if (!intersection_is_empty(aux_set, node->subtree_intersection_of_aux_bitsets, trie->m)) {
        return;
    }
    if (node->val != -1) {
        arr_out[(*arr_out_len)++] = node->val;
    }
    for (int i=0; i<node->successor_len; i++) {
        int succ_node_num = node->successor_node_num[i];
        struct TrieNode *succ = &trie->nodes[succ_node_num];
        int new_remaining_num_additions_permitted = remaining_num_additions_permitted - !ISELEMENT(set, succ->key);
        if (new_remaining_num_additions_permitted >= 0) {
            trie_get_all_almost_subsets_helper(trie, succ, set, aux_set, num_additions_permitted,
                    new_remaining_num_additions_permitted, arr_out, arr_out_len);
        }
    }
}

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len)
{
    *arr_out_len = 0;
    trie_get_all_almost_subsets_helper(trie, &trie->root, set, aux_set, num_additions_permitted, num_additions_permitted, arr_out, arr_out_len);
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
            node->subtree_intersection = get_bitset(trie->bute);
            node->subtree_intersection_of_aux_bitsets = get_bitset(trie->bute);
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


