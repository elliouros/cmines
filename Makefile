mines: main.c term.o
	$(CC) -o $@ $^

term.o: term.c
