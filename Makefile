all:	bute-solver

bute-solver:
	javac -d "bin" src/bute/*.java

clean:
	rm -f bin/bute/*.class
