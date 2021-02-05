#ifndef TRIE_H
#define TRIE_H

#include "bitset.h"

struct ButeTrieNode;

struct ButeTrie
{
    struct ButeTrieNode *root;
    int nodes_len;
    int graph_n;
    int m;
    struct Bute *bute;
};

void bute_trie_init(struct ButeTrie *trie, int n, int m, struct Bute *bute);

void bute_trie_destroy(struct ButeTrie *trie);

void bute_trie_add_element(struct ButeTrie *trie, setword *key_bitset, setword *aux_bitset, int val);

void bute_trie_get_all_almost_subsets(struct ButeTrie *trie, setword *set, setword *aux_set, int num_additions_permitted, size_t *arr_out, size_t *arr_out_len);

#endif
