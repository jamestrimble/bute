all: solve

C_FILES := solve.c bitset.c hash_map.c trie.c
H_FILES := bitset.h graph.h hash_map.h trie.h
solve: $(C_FILES) $(H_FILES)
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve $(C_FILES) -mpopcnt

clean:
	rm -f solve
