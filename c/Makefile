all: solve

solve: solve.c bitset.c bitset.h
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve solve.c bitset.c -mpopcnt

clean:
	rm -f solve
