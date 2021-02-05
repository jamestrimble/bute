#include "bitset.h"
#include "bute_solver.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static void set_first_k_bits(setword *bitset, int k)
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

void bute_bitset_intersect_with(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ww[i];
}

int bute_popcount_of_set_difference(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] & ~ww[i]);
    return count;
}

bool bute_intersection_is_empty(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] & ww[i])
            return false;
    return true;
}

bool bute_bitset_equals(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] != ww[i])
            return false;
    return true;
}

bool bute_bitset_is_superset(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~vv[i])
            return false;
    return true;
}

bool bute_bitset_union_is_superset(setword *vv, setword *uu, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~(vv[i] | uu[i]))
            return false;
    return true;
}

void bute_bitset_addall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] |= ww[i];
}

void bute_bitset_removeall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ~ww[i];
}

void bute_clear_bitset(setword *vv, int m)
{
    for (int i=0; i<m; i++)
        vv[i] = 0;
}

bool bute_bitset_is_empty(setword *vv, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i])
            return false;
    return true;
}

int bute_popcount_of_union(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] | ww[i]);
    return count;
}

int bute_bitset_compare(setword const *vv, setword const *ww, int m)
{
    for (int i=m; i--; ) {
        if (vv[i] != ww[i]) {
            return vv[i] < ww[i] ? -1 : 1;
        }
    }
    return 0;
}

int bute_popcount(setword const *vv, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i]);
    return count;
}

struct ButeBitset *get_Bitset(struct Bute *bute)
{
#ifdef USE_MALLOC_FOR_BITSETS
    return bute_xmalloc(sizeof(struct ButeBitset) + bute->m * sizeof(setword));
#endif
    if (bute->bitset_free_list_head == NULL) {
        struct ButeBitset *b = bute_xmalloc(sizeof(struct ButeBitset) + bute->m * sizeof(setword));
        b->next = NULL;
        bute->bitset_free_list_head = b;
    }
    struct ButeBitset *b = bute->bitset_free_list_head;
    bute->bitset_free_list_head = b->next;
    return b;
}

setword *bute_get_bitset(struct Bute *bute)
{
    return get_Bitset(bute)->bitset;
}

setword *bute_get_empty_bitset(struct Bute *bute)
{
    setword *b = bute_get_bitset(bute);
    for (int i=0; i<bute->m; i++)
        b[i] = 0;
    return b;
}

setword *bute_get_full_bitset(struct Bute *bute, int n)
{
    setword *b = bute_get_bitset(bute);
    set_first_k_bits(b, n);
    return b;
}

setword *bute_get_copy_of_bitset(struct Bute *bute, setword const *vv)
{
    setword *b = bute_get_bitset(bute);
    for (int i=0; i<bute->m; i++)
        b[i] = vv[i];
    return b;
}

setword *bute_get_union_of_bitsets(struct Bute *bute, setword const *vv, setword const *ww)
{
    setword *b = bute_get_bitset(bute);
    for (int i=0; i<bute->m; i++)
        b[i] = vv[i] | ww[i];
    return b;
}

void bute_free_Bitset(struct Bute *bute, struct ButeBitset *b)
{
#ifdef USE_MALLOC_FOR_BITSETS
    free(b);
    return;
#endif
    b->next = bute->bitset_free_list_head;
    bute->bitset_free_list_head = b;
}

void bute_free_bitset(struct Bute *bute, setword *bitset)
{
    struct ButeBitset *b = (struct ButeBitset *)((char *) bitset - offsetof(struct ButeBitset, bitset));
    bute_free_Bitset(bute, b);
}

void bute_free_Bitsets(struct Bute *bute, struct ButeBitset *b)
{
    while (b) {
        struct ButeBitset *next_to_free = b->next;
        bute_free_Bitset(bute, b);
        b = next_to_free;
    }
}

void bute_deallocate_Bitsets(struct Bute *bute)
{
    while (bute->bitset_free_list_head) {
        struct ButeBitset *next_to_free = bute->bitset_free_list_head->next;
        free(bute->bitset_free_list_head);
        bute->bitset_free_list_head = next_to_free;
    }
}
