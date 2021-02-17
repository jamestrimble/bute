#include "bitset_arena.h"
#include "util.h"

void bute_add_arena(struct ButeListOfBitsetArenas *list, int m)
{
    struct ButeBitsetArena *arena = bute_xmalloc(sizeof *arena + 2 * m * ARENA_SIZE * sizeof(setword));
    arena->next = list->head;
    arena->sz = 0;
    list->head = arena;
}

setword *bute_get_pair_of_bitsets(struct ButeListOfBitsetArenas *list, int m)
{
    if (list->head->sz == ARENA_SIZE) {
        bute_add_arena(list, m);
    }
    return list->head->bitsets + 2 * m * list->head->sz++;
}

void bute_free_arenas(struct ButeListOfBitsetArenas list)
{
    struct ButeBitsetArena *arena = list.head;
    while (arena) {
        struct ButeBitsetArena *next_arena = arena->next;
        free(arena);
        arena = next_arena;
    }
}


