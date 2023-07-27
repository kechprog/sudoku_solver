all: solver
	./solver
solver: solver.c
	gcc solver.c -O3 -march=native -Wall -Wextra -o solver
