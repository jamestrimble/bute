all: bute bute.a

OBJECTS := bitset.o bitset_arena.o bute.o bute_solver.o graph.o hash_map.o trie.o util.o
BUTE_MAIN_C := bute_main.c
CFLAGS = -std=c11 -O3 -march=native -g -ggdb -Wall -Wextra -Wpedantic -Werror -Werror=vla

bute: $(OBJECTS) $(BUTE_MAIN_C)
	${CC} -o $@ $(CFLAGS) $(OBJECTS) $(BUTE_MAIN_C)

bute.a: $(OBJECTS)
	ar crs $@ $(OBJECTS)

clean:
	rm -f bute bute.a *.o

bitset_arena.o: bitset_arena.c bitset_arena.h bitset.h util.h
bitset.o: bitset.c bitset.h bute_solver.h bute.h util.h
bute.o: bute.c bute.h bitset.h bitset_arena.h bute_solver.h graph.h \
 hash_map.h trie.h util.h sort_r.h
bute_main.o: bute_main.c bute.h
bute_solver.o: bute_solver.c bute_solver.h bitset.h bute.h graph.h util.h
graph.o: graph.c bitset.h graph.h util.h
hash_map.o: hash_map.c hash_map.h bitset.h bute_solver.h bute.h util.h
trie.o: trie.c trie.h bitset.h bitset_arena.h util.h
util.o: util.c util.h
