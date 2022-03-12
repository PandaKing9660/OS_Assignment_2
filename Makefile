all: test

test: 
	gcc main.cpp -o main -lncurses

clean:
	rm -rvf *.out *.exe main