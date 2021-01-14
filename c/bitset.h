#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stdlib.h>

// Use some of the same API as Nauty for bitsets

#define setword unsigned long long
#define graph unsigned long long

#define WORDSIZE 64
#define BIT(x) (1ull << (x))
#define POPCOUNT(x) __builtin_popcountll(x)
#define FIRSTBITNZ(x) __builtin_ctzll(x)
#define ADDELEMENT(s, x) ((s)[(x)>>6] |= BIT((x)&63))
#define DELELEMENT(s, x) ((s)[(x)>>6] &= ~BIT((x)&63))
#define ISELEMENT(s, x) (((s)[(x)>>6] & BIT((x)&63)) != 0)
#define TAKEBIT(x, sw) {(x)=FIRSTBITNZ(sw);((sw)^=BIT(x));}
#define GRAPHROW(g, v, m) ((g) + (v) * (size_t)(m))
#define ADDONEEDGE(g, v, w, m) {ADDELEMENT(GRAPHROW(g, v, m), w); ADDELEMENT(GRAPHROW(g, w, m), v);}
#define SETWORDSNEEDED(n) ((n + (64-1)) / 64)

void set_first_k_bits(setword *bitset, int k);

void bitset_intersect_with(setword *vv, setword const *ww, int m);

int popcount_of_set_difference(setword const *vv, setword const *ww, int m);

bool intersection_is_empty(setword *vv, setword *ww, int m);

bool bitset_equals(setword *vv, setword *ww, int m);

bool bitset_is_superset(setword *vv, setword *ww, int m);

bool bitset_union_is_superset(setword *vv, setword *uu, setword *ww, int m);

void bitset_addall(setword *vv, setword const *ww, int m);

void bitset_removeall(setword *vv, setword const *ww, int m);

void clear_bitset(setword *vv, int m);

bool isempty(setword *vv, int m);

int popcount_of_union(setword const *vv, setword const *ww, int m);

int bitset_compare(setword const *vv, setword const *ww, int m);

int popcount(setword const *vv, int m);

//https://stackoverflow.com/a/10380191/3347737
#define PASTE_HELPER(a,b) a ## b
#define PASTE(a,b) PASTE_HELPER(a,b)

// If you use these macros, don't modify bitset while iterating over it!
#define FOR_EACH_IN_BITSET_HELPER(v, bitset, m, i, sw, x) \
           for (int i=0;i<m;i++) {setword sw=bitset[i]; while (sw) {int x; TAKEBIT(x, sw); int v=i*WORDSIZE+x;
#define FOR_EACH_IN_BITSET(v, bitset, m) \
           FOR_EACH_IN_BITSET_HELPER(v, bitset, m, PASTE(i,__LINE__), PASTE(sw,__LINE__), PASTE(x,__LINE__))
#define END_FOR_EACH_IN_BITSET }}

#endif
