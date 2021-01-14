all: solve

solve: solve.c bitset.c hash_map.c trie.c bitset.h hash_map.h trie.h
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve solve.c bitset.c hash_map.c trie.c -mpopcnt

clean:
	rm -f solve
