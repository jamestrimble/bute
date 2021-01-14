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
    struct Bute *bute;
};

void trie_init(struct Trie *trie, int n, int m, struct Bute *bute);

void trie_destroy(struct Trie *trie);

void trie_add_element(struct Trie *trie, setword *key_bitset, setword *aux_bitset, int val);

void trie_get_all_almost_subsets(struct Trie *trie, setword *set, setword *aux_set, int num_additions_permitted, int *arr_out, int *arr_out_len);

#endif
