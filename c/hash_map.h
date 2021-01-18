#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "bitset.h"
#include "bute.h"

struct hash_chain_element
{
    int val;
    int root_depth;
    struct hash_chain_element *next;
    setword key[];
};

struct hash_map
{
    int M;
    int sz;
    struct hash_chain_element **chain_heads;
    int m;
    struct Bute *bute;
};

void hash_init(struct hash_map *s, struct Bute *bute);

void hash_destroy(struct hash_map *s);

bool hash_add_or_update(struct hash_map *s, setword * key, int val, int root_depth);

setword ** hash_map_to_list(struct hash_map *s);

bool hash_get_val(struct hash_map *s, setword *key, int *val);

#endif
