all: bute

C_FILES := bitset.c bute.c bute_main.c bute_solver.c graph.c hash_map.c trie.c util.c
H_FILES := bitset.h bute.h bute_solver.h graph.h hash_map.h trie.h util.h
bute: $(C_FILES) $(H_FILES)
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o bute $(C_FILES) -march=native -flto

clean:
	rm -f bute
