#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "bitset.h"

struct hash_chain_element
{
    int val;
    struct hash_chain_element *next;
    setword key[];
};

struct hash_map
{
    int M;
    int sz;
    struct hash_chain_element **chain_heads;
    int m;
};

void hash_init(struct hash_map *s, int m);

void hash_destroy(struct hash_map *s);

bool hash_iselement(struct hash_map *s, setword *key);

void hash_add(struct hash_map *s, setword * key, int val);

setword ** hash_map_to_list(struct hash_map *s, setword *(*alloc_bitset)());

bool hash_get_val(struct hash_map *s, setword *key, int *val);

#endif
