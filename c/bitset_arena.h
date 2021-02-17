#ifndef BITSET_ARENA_H
#define BITSET_ARENA_H

#include <stddef.h>

#include "bitset.h"

// ARENA_SIZE is the number of pairs of bitsets that can be stored in one arena.
// I've attempted to set this in such a way that almost all the space in a
// chunk of heap memory will be used.  I'm sure this could be improved.
#define ARENA_SIZE 2045

struct ButeBitsetArena
{
    struct ButeBitsetArena *next;
    size_t sz;
    setword bitsets[];
};

struct ButeListOfBitsetArenas
{
    struct ButeBitsetArena *head;
};

void bute_add_arena(struct ButeListOfBitsetArenas *list, int m);

setword *bute_get_pair_of_bitsets(struct ButeListOfBitsetArenas *list, int m);

void bute_free_arenas(struct ButeListOfBitsetArenas list);

#endif
