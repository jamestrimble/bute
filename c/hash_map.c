#include <stdbool.h>
#include <stdlib.h>

#include "hash_map.h"

void hash_init(struct hash_map *s, struct Bute *bute)
{
    s->M = 1;
    s->sz = 0;
    s->chain_heads = calloc(s->M, sizeof *s->chain_heads);
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

    struct hash_chain_element **new_chain_heads = calloc(new_M, sizeof new_chain_heads);
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

// Assumption: key is not in s already
void hash_add(struct hash_map *s, setword * key, int val)
{
//    printf("adding\n");
//    printf("HASH %d\n", (int)hash(key));
    unsigned h = hash(key, s->m) % s->M;
    struct hash_chain_element *elem = malloc(sizeof *elem + s->m * sizeof(setword));
    for (int k=0; k<s->m; k++)
        elem->key[k] = key[k];
    elem->val = val;
    elem->next = s->chain_heads[h];
    s->chain_heads[h] = elem;
    ++s->sz;
    
    if (s->sz > (s->M + 1) / 2) {
        hash_grow(s);
    }
}

bool hash_get_val(struct hash_map *s, setword *key, int *val)
{
    unsigned h = hash(key, s->m) % s->M;
    struct hash_chain_element *p = s->chain_heads[h];
    while (p) {
        if (bitset_equals(p->key, key, s->m)) {
            *val = p->val;
            return true;
        }
        p = p->next;
    }
    return false;
}

bool hash_iselement(struct hash_map *s, setword *key)
{
    int junk;
    return hash_get_val(s, key, &junk);
}

setword ** hash_map_to_list(struct hash_map *s)
{
    setword **retval = malloc(s->sz * sizeof *retval);
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


