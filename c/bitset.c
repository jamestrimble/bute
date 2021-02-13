#include "bitset.h"
#include "bute_solver.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

void bute_clear_bitset(setword *bitset, int m)
{
    for (int i=0; i<m; i++)
        bitset[i] = 0;
}

void bute_bitset_copy(setword *dest, setword const *src, int m)
{
    for (int i=0; i<m; i++)
        dest[i] = src[i];
}

void bute_bitset_union(setword *dest, setword const *src1, setword const *src2, int m)
{
    for (int i=0; i<m; i++)
        dest[i] = src1[i] | src2[i];
}

static void set_first_k_bits(setword *bitset, unsigned k)
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

struct ButeBitset *bute_get_Bitset(struct Bute *bute)
{
    return bute_xmalloc(sizeof(struct ButeBitset) + bute->m * sizeof(setword));
}

setword *bute_get_bitset(struct Bute *bute)
{
    return bute_get_Bitset(bute)->bitset;
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

void bute_free_bitset(setword *bitset)
{
    struct ButeBitset *b = (struct ButeBitset *)((char *) bitset - offsetof(struct ButeBitset, bitset));
    free(b);
}

void bute_free_Bitsets(struct ButeBitset *b)
{
    while (b) {
        struct ButeBitset *next_to_free = b->next;
        free(b);
        b = next_to_free;
    }
}
