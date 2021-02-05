#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "bitset.h"
#include "bute_solver.h"

struct ButeHashChainElement;

struct ButeHashMap
{
    size_t M;
    size_t sz;
    struct ButeHashChainElement **chain_heads;
    int m;
    struct Bute *bute;
};

void bute_hash_init(struct ButeHashMap *s, struct Bute *bute);

void bute_hash_destroy(struct ButeHashMap *s);

bool bute_hash_add_or_update(struct ButeHashMap *s, setword * key, int val, int root_depth);

bool bute_hash_get_val(struct ButeHashMap *s, setword *key, int *val);

#endif
