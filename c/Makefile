all: bute bute.a

OBJECTS := bitset.o bute.o bute_solver.o graph.o hash_map.o trie.o util.o
BUTE_MAIN_C := bute_main.c
CFLAGS = -O3 -march=native -flto -g -ggdb -Wall -Wpedantic -Werror=vla 

bute: $(OBJECTS) $(BUTE_MAIN_C)
	${CC} -o $@ $(OBJECTS) $(BUTE_MAIN_C)

bute.a: $(OBJECTS)
	ar crs $@ $(OBJECTS)

clean:
	rm -f bute bute.a *.o

bitset.o: bitset.c bitset.h bute.h util.h
bute.o: bute.c bute.h bitset.h graph.h util.h
bute_main.o: bute_main.c bute_solver.h
bute_solver.o: bute_solver.c bute_solver.h bitset.h bute.h graph.h \
 hash_map.h trie.h util.h
graph.o: graph.c bitset.h graph.h util.h
hash_map.o: hash_map.c hash_map.h bitset.h bute.h util.h
trie.o: trie.c trie.h bitset.h util.h
util.o: util.c util.h
