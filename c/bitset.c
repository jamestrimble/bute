#include "bitset.h"

#include <stdbool.h>

void set_first_k_bits(setword *bitset, int k)
{
    int wordnum = 0;
    while (k > 63) {
        bitset[wordnum] = ~0ull;
        ++wordnum;
        k -= 64;
    }
    for (int i=0; i<k; i++) {
        ADDELEMENT(bitset + wordnum, i);
    }
}

void bitset_intersect_with(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ww[i];
}

int popcount_of_set_difference(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] & ~ww[i]);
    return count;
}

bool intersection_is_empty(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] & ww[i])
            return false;
    return true;
}

bool bitset_equals(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] != ww[i])
            return false;
    return true;
}

bool bitset_is_superset(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~vv[i])
            return false;
    return true;
}

bool bitset_union_is_superset(setword *vv, setword *uu, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~(vv[i] | uu[i]))
            return false;
    return true;
}

void bitset_addall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] |= ww[i];
}

void bitset_removeall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ~ww[i];
}

void clear_bitset(setword *vv, int m)
{
    for (int i=0; i<m; i++)
        vv[i] = 0;
}

bool isempty(setword *vv, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i])
            return false;
    return true;
}

int popcount_of_union(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] | ww[i]);
    return count;
}

