CC=g++
CFLAGS= -O3 -std=c++0x -Wall

all: solver

solver: solver.cpp
	$(CC) solver.cpp -o solver $(CFLAGS)