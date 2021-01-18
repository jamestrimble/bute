#include <stdbool.h>
#include <stdlib.h>

#include "hash_map.h"
#include "util.h"

void hash_init(struct hash_map *s, struct Bute *bute)
{
    s->M = 1;
    s->sz = 0;
    s->chain_heads = bute_xcalloc(s->M, sizeof *s->chain_heads);
    s->m = bute->m;
    s->bute = bute;
}

// Based on https://gist.github.com/badboy/6267743
unsigned hash6432shift(setword key)
{
    key = (~key) + (key << 18); // key = (key << 18) - key - 1;
    key = key ^ (key >> 31);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return (unsigned) key;
}

unsigned hash(setword *x, int m)
{
    unsigned result = 0;
    for (int i=0; i<m; i++) {
        result ^= hash6432shift(x[i]);
    }
    return result;
}

void hash_grow(struct hash_map *s)
{
//    printf("growing from %d to %d\n", s->M, s->M * 2);
    // grow the table
    int new_M = s->M * 2;

    struct hash_chain_element **new_chain_heads = bute_xcalloc(new_M, sizeof new_chain_heads);
    // move the chain elements to the new chains
    for (int i=0; i<s->M; i++) {
        struct hash_chain_element *p = s->chain_heads[i];
        while (p) {
            struct hash_chain_element *next_in_old_list = p->next;
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

struct hash_chain_element *get_chain_element(struct hash_map *s, setword *key, unsigned h)
{
    struct hash_chain_element *p = s->chain_heads[h];
    while (p) {
        if (bitset_equals(p->key, key, s->m)) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// add key-val pair if key is not in the hash map.
// If key is already in the map, leave value unchanged but update root depth.
// Return true if key is added or root depth is updated.
bool hash_add_or_update(struct hash_map *s, setword * key, int val, int root_depth)
{
    unsigned h = hash(key, s->m) % s->M;
    struct hash_chain_element *elem = get_chain_element(s, key, h);
    if (elem == NULL) {
        struct hash_chain_element *elem = bute_xmalloc(sizeof *elem + s->m * sizeof(setword));
        for (int k=0; k<s->m; k++)
            elem->key[k] = key[k];
        elem->val = val;
        elem->root_depth = root_depth;
        elem->next = s->chain_heads[h];
        s->chain_heads[h] = elem;
        ++s->sz;
        if (s->sz > (s->M + 1) / 2) {
            hash_grow(s);
        }
        return true;
    } else if (elem->root_depth != root_depth) {
        elem->root_depth = root_depth;
        return true;
    }
    return false;
}

bool hash_get_val(struct hash_map *s, setword *key, int *val)
{
    unsigned h = hash(key, s->m) % s->M;
    struct hash_chain_element *chain_elem = get_chain_element(s, key, h);
    if (chain_elem) {
        *val = chain_elem->val;
    }
    return false;
}

setword ** hash_map_to_list(struct hash_map *s)
{
    setword **retval = bute_xmalloc(s->sz * sizeof *retval);
    int j = 0;
    for (int i=0; i<s->M; i++) {
        struct hash_chain_element *p = s->chain_heads[i];
        while (p) {
            retval[j] = get_bitset(s->bute);
            for (int k=0; k<s->m; k++) {
                retval[j][k] = p->key[k];
            }
            j++;
            p = p->next;
        }
    }
    return retval;
}

void hash_destroy(struct hash_map *s)
{
    for (int i=0; i<s->M; i++) {
        struct hash_chain_element *p = s->chain_heads[i];
        while (p) {
            struct hash_chain_element *next_p = p->next;
            free(p);
            p = next_p;
        }
    }
    free(s->chain_heads);
}


