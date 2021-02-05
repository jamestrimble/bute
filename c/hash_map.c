#include <stdbool.h>
#include <stdlib.h>

#include "hash_map.h"
#include "util.h"

struct ButeHashChainElement
{
    int val;
    int root_depth;
    struct ButeHashChainElement *next;
    setword key[];
};

void bute_hash_init(struct ButeHashMap *s, struct Bute *bute)
{
    s->M = 3;
    s->sz = 0;
    s->chain_heads = bute_xmalloc(s->M * sizeof *s->chain_heads);
    for (size_t i=0; i<s->M; i++)
        s->chain_heads[i] = NULL;
    s->m = bute->m;
    s->bute = bute;
}

// Based on https://gist.github.com/badboy/6267743
static unsigned hash6432shift(setword key)
{
    key = (~key) + (key << 18); // key = (key << 18) - key - 1;
    key = key ^ (key >> 31);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return (unsigned) key;
}

static unsigned hash(setword *x, int m)
{
    unsigned result = 0;
    for (int i=0; i<m; i++) {
        result ^= hash6432shift(x[i]);
    }
    return result;
}

static void hash_grow(struct ButeHashMap *s)
{
//    printf("growing from %d to %d\n", s->M, s->M * 2);
    // grow the table
    size_t new_M = s->M * 2 + 1;

    struct ButeHashChainElement **new_chain_heads = bute_xmalloc(new_M * sizeof *new_chain_heads);
    for (size_t i=0; i<new_M; i++)
        new_chain_heads[i] = NULL;
    // move the chain elements to the new chains
    for (size_t i=0; i<s->M; i++) {
        struct ButeHashChainElement *p = s->chain_heads[i];
        while (p) {
            struct ButeHashChainElement *next_in_old_list = p->next;
            unsigned h = hash(p->key, s->m) % new_M;
            p->next = new_chain_heads[h];
            new_chain_heads[h] = p;
            p = next_in_old_list;
        }
    }
    free(s->chain_heads);
    s->chain_heads = new_chain_heads;
    s->M = new_M;
}

static struct ButeHashChainElement *get_chain_element(struct ButeHashMap *s, setword *key, unsigned h)
{
    struct ButeHashChainElement *p = s->chain_heads[h];
    while (p) {
        if (bute_bitset_equals(p->key, key, s->m)) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// add key-val pair if key is not in the hash map.
// If key is already in the map, leave value unchanged but update root depth.
// Return true if key is added or root depth is updated.
bool bute_hash_add_or_update(struct ButeHashMap *s, setword * key, int val, int root_depth)
{
    unsigned h = hash(key, s->m) % s->M;
    struct ButeHashChainElement *elem = get_chain_element(s, key, h);
    if (elem == NULL) {
        struct ButeHashChainElement *elem = bute_xmalloc(sizeof *elem + s->m * sizeof(setword));
        for (int k=0; k<s->m; k++)
            elem->key[k] = key[k];
        elem->val = val;
        elem->root_depth = root_depth;
        elem->next = s->chain_heads[h];
        s->chain_heads[h] = elem;
        ++s->sz;
        if (s->sz > s->M) {
            hash_grow(s);
        }
        return true;
    } else if (elem->root_depth != root_depth) {
        elem->root_depth = root_depth;
        return true;
    }
    return false;
}

bool bute_hash_get_val(struct ButeHashMap *s, setword *key, int *val)
{
    unsigned h = hash(key, s->m) % s->M;
    struct ButeHashChainElement *chain_elem = get_chain_element(s, key, h);
    if (chain_elem) {
        *val = chain_elem->val;
    }
    return false;
}

void bute_hash_destroy(struct ButeHashMap *s)
{
    for (size_t i=0; i<s->M; i++) {
        struct ButeHashChainElement *p = s->chain_heads[i];
        while (p) {
            struct ButeHashChainElement *next_p = p->next;
            free(p);
            p = next_p;
        }
    }
    free(s->chain_heads);
}
