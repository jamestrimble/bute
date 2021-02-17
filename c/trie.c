// This data structure is largely based on https://stackoverflow.com/a/6514445/3347737

#include "trie.h"
#include "util.h"

#include <stdint.h>
#include <stdlib.h>

#define SUBTREE_INTERSECTION(node) (node->bitsets)
#define SUBTREE_INTERSECTION_OF_AUX_BITSETS(node) (node->bitsets + trie->m)

#define ARENA_SIZE 2045

// An arena for bitsets
struct ButeTrieArena
{
    struct ButeTrieArena *next;
    size_t sz;
    setword bitsets[];
};

static void add_arena(struct ButeTrie *trie)
{
    struct ButeTrieArena *arena = bute_xmalloc(sizeof *arena + 2 * trie->m * ARENA_SIZE * sizeof(setword));
    arena->next = trie->bitset_arenas;
    arena->sz = 0;
    trie->bitset_arenas = arena;
}

static setword *get_pair_of_bitsets(struct ButeTrie *trie)
{
    if (trie->bitset_arenas->sz == ARENA_SIZE) {
        add_arena(trie);
    }
    return trie->bitset_arenas->bitsets + 2 * trie->m * trie->bitset_arenas->sz++;
}

struct ButeTrieNode
{
    unsigned children_len;
    unsigned children_capacity;
    struct ButeTrieNode *children;

    int key;
    size_t val;

    setword *bitsets;
};

#define NO_VALUE SIZE_MAX

static void trie_node_init(struct ButeTrie *trie, struct ButeTrieNode *node, int key,
                    setword *initial_subtrie_intersection,
                    setword *initial_subtrie_intersection_of_aux_sets)
{
    node->key = key;
    node->val = NO_VALUE;
    node->children = NULL;
    node->children_len = 0;
    node->children_capacity = 0;
    node->bitsets = get_pair_of_bitsets(trie);
    for (int i=0; i<trie->m; i++) {
        SUBTREE_INTERSECTION(node)[i] = initial_subtrie_intersection[i];
        SUBTREE_INTERSECTION_OF_AUX_BITSETS(node)[i] = initial_subtrie_intersection_of_aux_sets[i];
    }
}

void bute_trie_init(struct ButeTrie *trie, int n, int m, struct Bute *bute)
{
    trie->graph_n = n;
    trie->m = m;
    trie->bute = bute;
    trie->bitset_arenas = NULL;
    add_arena(trie);
    setword *full_bitset = bute_xmalloc(m * sizeof(setword));
    bute_bitset_set_first_k_bits(full_bitset, trie->graph_n);
    trie->root = bute_xmalloc(sizeof(struct ButeTrieNode));
    trie_node_init(trie, trie->root, -1, full_bitset, full_bitset);
    free(full_bitset);
}

static void trie_node_destroy(struct ButeTrie *trie, struct ButeTrieNode *node)
{
    for (unsigned i=0; i<node->children_len; i++) {
        trie_node_destroy(trie, &node->children[i]);
    }
    free(node->children);
}

void bute_trie_destroy(struct ButeTrie *trie)
{
    trie_node_destroy(trie, trie->root);
    free(trie->root);
    struct ButeTrieArena *arena = trie->bitset_arenas;
    while (arena) {
        struct ButeTrieArena *next_arena = arena->next;
        free(arena);
        arena = next_arena;
    }
}

static struct ButeTrieNode *trie_get_child_node(struct ButeTrieNode *node, int key)
{
    for (unsigned i=0; i<node->children_len; i++) {
        if (node->children[i].key == key) {
            return &node->children[i];
        }
    }
    return NULL;
}

static void trie_get_all_almost_subsets_helper(struct ButeTrie *trie, struct ButeTrieNode *node, setword *set,
                                        setword *aux_set, int num_additions_permitted, int remaining_num_additions_permitted, size_t *arr_out, size_t *arr_out_len)
{
    if (node->val != NO_VALUE) {
        arr_out[(*arr_out_len)++] = node->val;
    }
    for (unsigned i=0; i<node->children_len; i++) {
        struct ButeTrieNode *child = &node->children[i];
        int new_remaining_num_additions_permitted = remaining_num_additions_permitted - !ISELEMENT(set, child->key);
        if (new_remaining_num_additions_permitted < 0 ||
                !bute_intersection_is_empty(aux_set, SUBTREE_INTERSECTION_OF_AUX_BITSETS(child), trie->m) ||
                bute_popcount_of_set_difference(SUBTREE_INTERSECTION(child), set, trie->m) > num_additions_permitted) {
            continue;
        }
        trie_get_all_almost_subsets_helper(trie, child, set, aux_set, num_additions_permitted,
                new_remaining_num_additions_permitted, arr_out, arr_out_len);
    }
}

void bute_trie_get_all_almost_subsets(struct ButeTrie *trie, setword *set, setword *aux_set, int num_additions_permitted, size_t *arr_out, size_t *arr_out_len)
{
    *arr_out_len = 0;
    trie_get_all_almost_subsets_helper(trie, trie->root, set, aux_set, num_additions_permitted, num_additions_permitted, arr_out, arr_out_len);
}

static struct ButeTrieNode *trie_node_add_child(struct ButeTrieNode *node)
{
    if (node->children_len == node->children_capacity) {
        node->children_capacity = new_vec_capacity(node->children_capacity);
        node->children = bute_xrealloc(node->children, node->children_capacity * sizeof(struct ButeTrieNode));
    }
    ++node->children_len;
    return &node->children[node->children_len - 1];
}

void bute_trie_add_element(struct ButeTrie *trie, setword *key_bitset, setword *aux_bitset, size_t val)
{
    struct ButeTrieNode *node = trie->root;
    bute_bitset_intersect_with(SUBTREE_INTERSECTION(node), key_bitset, trie->m);
    bute_bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(node), aux_bitset, trie->m);
    FOR_EACH_IN_BITSET(v, key_bitset, trie->m)
        struct ButeTrieNode * child = trie_get_child_node(node, v);
        if (child) {
            node = child;
            bute_bitset_intersect_with(SUBTREE_INTERSECTION(node), key_bitset, trie->m);
            bute_bitset_intersect_with(SUBTREE_INTERSECTION_OF_AUX_BITSETS(node), aux_bitset, trie->m);
        } else {
            struct ButeTrieNode *new_node = trie_node_add_child(node);
            trie_node_init(trie, new_node, v, key_bitset, aux_bitset);
            node = new_node;
        }
    END_FOR_EACH_IN_BITSET
    if (node->val == NO_VALUE) {   // if this leaf node did not already exist
        node->val = val;
    }
}
