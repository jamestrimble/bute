#include "bitset.h"
#include "bute_solver.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

void bute_bitset_set_first_k_bits(setword *bitset, unsigned k)
{
    int wordnum = 0;
    while (k >= WORDSIZE) {
        bitset[wordnum] = ~0ull;
        ++wordnum;
        k -= WORDSIZE;
    }
    if (k) {
        bitset[wordnum] = (1ull << k) - 1;
    }
}

struct ButeBitsetListNode *bute_get_Bitset(int m)
{
    return bute_xmalloc(sizeof(struct ButeBitsetListNode) + m * sizeof(setword));
}

void bute_free_list_of_bitsets(struct ButeBitsetListNode *b)
{
    while (b) {
        struct ButeBitsetListNode *next_to_free = b->next;
        free(b);
        b = next_to_free;
    }
}
