all: solve

solve: solve.c bitset.h
	${CC} -O3 -g -ggdb -Wall -Wpedantic -Werror=vla -o solve solve.c -mpopcnt

clean:
	rm -f solve
