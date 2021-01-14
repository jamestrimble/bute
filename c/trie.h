#ifndef TRIE_H
#define TRIE_H

#include "bitset.h"

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
    setword *(*alloc_bitset)();
    void (*dealloc_bitset)(setword *);
};

void trie_init(struct Trie *trie, int n, int m, setword *(*alloc_bitset)(), void (*dealloc_bitset)(setword *));

void trie_destroy(struct Trie *trie);

void trie_add_key_val(struct Trie *trie, int *key, setword *key_bitset, int val);

void trie_add_an_aux_bitset(struct Trie *trie, int *key, setword *aux_bitset);

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len);

#endif
