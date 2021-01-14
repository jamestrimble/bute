#include "bitset.h"
#include "bute.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

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

int bitset_compare(setword const *vv, setword const *ww, int m)
{
    for (int i=0; i<m; i++) {
        if (vv[i] != ww[i]) {
            return vv[i] < ww[i] ? -1 : 1;
        }
    }
    return 0;
}

int popcount(setword const *vv, int m)
{
    int count = 0;
    for (int i=0; i<m; i++)
        count += POPCOUNT(vv[i]);
    return count;
}

struct Bitset *get_Bitset(struct Bute *bute)
{
    if (bute->bitset_free_list_head == NULL) {
        struct Bitset *b = malloc(sizeof(struct Bitset) + bute->m * sizeof(setword));
        if (b == NULL)
            exit(1);
        b->next = NULL;
        bute->bitset_free_list_head = b;
    }
    struct Bitset *b = bute->bitset_free_list_head;
    bute->bitset_free_list_head = b->next;
    return b;
}

setword *get_bitset(struct Bute *bute)
{
    return get_Bitset(bute)->bitset;
}

setword *get_empty_bitset(struct Bute *bute)
{
    setword *b = get_bitset(bute);
    for (int i=0; i<bute->m; i++)
        b[i] = 0;
    return b;
}

setword *get_copy_of_bitset(struct Bute *bute, setword const *vv)
{
    setword *b = get_bitset(bute);
    for (int i=0; i<bute->m; i++)
        b[i] = vv[i];
    return b;
}

void free_Bitset(struct Bute *bute, struct Bitset *b)
{
    b->next = bute->bitset_free_list_head;
    bute->bitset_free_list_head = b;
}

void free_bitset(struct Bute *bute, setword *bitset)
{
    struct Bitset *b = (struct Bitset *)((char *) bitset - offsetof(struct Bitset, bitset));
    free_Bitset(bute, b);
}

void free_Bitsets(struct Bute *bute, struct Bitset *b)
{
    while (b) {
        struct Bitset *next_to_free = b->next;
        free_Bitset(bute, b);
        b = next_to_free;
    }
}

void deallocate_Bitsets(struct Bute *bute)
{
    while (bute->bitset_free_list_head) {
        struct Bitset *next_to_free = bute->bitset_free_list_head->next;
        free(bute->bitset_free_list_head);
        bute->bitset_free_list_head = next_to_free;
    }
}
