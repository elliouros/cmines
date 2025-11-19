all: mines

mines: main.c
	cc -o mines main.c

.PHONY: all
