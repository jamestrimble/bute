#ifndef BITSET_H
#define BITSET_H

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

// Use some of the same API as Nauty for bitsets

#define setword unsigned long long
#define graph unsigned long long

#define WORDSIZE (sizeof(unsigned long long) * CHAR_BIT)
#define BIT(x) (1ull << (x))
#define POPCOUNT(x) __builtin_popcountll(x)
#define FIRSTBITNZ(x) __builtin_ctzll(x)
#define ADDELEMENT(s, x) ((s)[(x)/WORDSIZE] |= BIT((x)%WORDSIZE))
#define DELELEMENT(s, x) ((s)[(x)/WORDSIZE] &= ~BIT((x)%WORDSIZE))
#define ISELEMENT(s, x) (((s)[(x)/WORDSIZE] & BIT((x)%WORDSIZE)) != 0)
#define TAKEBIT(x, sw) {(x)=FIRSTBITNZ(sw);((sw)^=BIT(x));}
#define GRAPHROW(g, v, m) ((g) + (v) * (size_t)(m))
#define ADDONEEDGE(g, v, w, m) {ADDELEMENT(GRAPHROW(g, v, m), w); ADDELEMENT(GRAPHROW(g, w, m), v);}
#define SETWORDSNEEDED(n) ((n + (WORDSIZE-1)) / WORDSIZE)

//https://stackoverflow.com/a/10380191/3347737
#define PASTE_HELPER(a,b) a ## b
#define PASTE(a,b) PASTE_HELPER(a,b)

// If you use these macros, don't modify bitset while iterating over it!
#define FOR_EACH_IN_BITSET_HELPER(v, bitset, m, i, sw, x) \
           for (int i=0;i<m;i++) {setword sw=bitset[i]; while (sw) {int x; TAKEBIT(x, sw); int v=i*WORDSIZE+x;
#define FOR_EACH_IN_BITSET(v, bitset, m) \
           FOR_EACH_IN_BITSET_HELPER(v, bitset, m, PASTE(i,__LINE__), PASTE(sw,__LINE__), PASTE(x,__LINE__))
#define END_FOR_EACH_IN_BITSET }}

void bute_bitset_set_first_k_bits(setword *bitset, unsigned k);

struct ButeBitsetListNode
{
    struct ButeBitsetListNode *next;
    setword bitset[];
};

struct ButeBitsetListNode *bute_get_Bitset(int m);

void bute_free_list_of_bitsets(struct ButeBitsetListNode *b);

static inline void bute_clear_bitset(setword *bitset, int m)
{
    for (int i=0; i<m; i++)
        bitset[i] = 0;
}

static inline void bute_bitset_copy(setword *dest, setword const *src, int m)
{
    for (int i=0; i<m; i++)
        dest[i] = src[i];
}

static inline void bute_bitset_union(setword *dest, setword const *src1, setword const *src2, int m)
{
    for (int i=0; i<m; i++)
        dest[i] = src1[i] | src2[i];
}

static inline void bute_bitset_intersect_with(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ww[i];
}

static inline bool bute_intersection_is_empty(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] & ww[i])
            return false;
    return true;
}

static inline bool bute_bitset_equals(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i] != ww[i])
            return false;
    return true;
}

static inline bool bute_bitset_is_superset(setword *vv, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~vv[i])
            return false;
    return true;
}

static inline bool bute_bitset_union_is_superset(setword *vv, setword *uu, setword *ww, int m)
{
    for (int i=0; i<m; i++)
        if (ww[i] & ~(vv[i] | uu[i]))
            return false;
    return true;
}

static inline void bute_bitset_addall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] |= ww[i];
}

static inline void bute_bitset_removeall(setword *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++)
        vv[i] &= ~ww[i];
}

static inline bool bute_bitset_is_empty(setword *vv, int m)
{
    for (int i=0; i<m; i++)
        if (vv[i])
            return false;
    return true;
}

static inline int bute_popcount_of_union(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] | ww[i]);
    return count;
}

static inline int bute_popcount_of_set_difference(setword const *vv, setword const *ww, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i] & ~ww[i]);
    return count;
}

static inline int bute_bitset_compare(setword const *vv, setword const *ww, int m)
{
    for (int i=m; i--; ) {
        if (vv[i] != ww[i]) {
            return vv[i] < ww[i] ? -1 : 1;
        }
    }
    return 0;
}

static inline int bute_popcount(setword const *vv, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i]);
    return count;
}

#endif
