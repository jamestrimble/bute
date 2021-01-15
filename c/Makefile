all: solve

C_FILES := bitset.c bute.c graph.c hash_map.c solve.c trie.c util.c
H_FILES := bitset.h bute.h graph.h hash_map.h trie.h util.h
solve: $(C_FILES) $(H_FILES)
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve $(C_FILES) -mpopcnt

clean:
	rm -f solve
