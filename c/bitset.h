#ifndef BITSET_H
#define BITSET_H

// Use the same API as Nauty for bitsets

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

#endif
