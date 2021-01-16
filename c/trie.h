#ifndef TRIE_H
#define TRIE_H

#include "bitset.h"

struct TrieNode;
struct TrieNodePool;

struct Trie
{
    struct TrieNode *root;
    struct TrieNodePool *first_pool;
    int pool_len;
    int pool_sz;
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
