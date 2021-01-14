all: solve

solve: solve.c bitset.c hash_map.c bitset.h hash_map.h
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve solve.c hash_map.c bitset.c -mpopcnt

clean:
	rm -f solve
