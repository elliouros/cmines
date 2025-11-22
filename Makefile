CC = gcc

mines: main.c term.o
	gcc -o $@ $^

term.o: term.c
