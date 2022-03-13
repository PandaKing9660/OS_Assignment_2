all: test

test: 
	g++ main.cpp -o main -lncurses

clean:
	rm -rvf *.out *.exe main