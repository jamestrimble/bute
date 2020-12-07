all:	bute-solver

bute-solver:
	if ! [ -f src/tw/exact/Graph.java ]; then echo '*** Graph.java is missing; see README ***'; false; fi
	javac -d "bin" src/bute/*.java src/tw/exact/*.java

clean:
	rm -f bin/bute/*.class bute/*.class
