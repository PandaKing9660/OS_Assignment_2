all: test

test: 
	c++ main.cpp -o main -lncurses

clean:
	rm -rvf *.out *.exe main
