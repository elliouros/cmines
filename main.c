#include <bits/types/stack_t.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "term.h"

typedef struct Cell {
	bool is_mine: 1;
	bool revealed: 1;
	bool flagged: 1;
	uint adjacent: 4;
} Cell;

struct State {
	Cell *board;
	uint board_x;
	uint board_y;
	uint width;
	uint height;
	uint size;
	uint mines;
	uint start_index;
} State;

void print_cell(Cell cell);
void print_grid(void);
void scatter_mines(void);
void init_game(int width, int height, int num_mines);
void init_screen(void);
void run_game(void);

// TODO: Absolute spaghetti, reimplement probably (thanks goto)
// maybe wait until using ncurses (or otherwise) to redo?
void print_cell(Cell cell)
{
	if (cell.is_mine || cell.flagged)
	{
		fputs("\033[", stdout);
	} else
	{
		fputs("\033[0m", stdout);
		goto end_escape; // bypass next checks (assume false) and do not print 'm'
	}
	if (cell.is_mine)
	{
		putchar('1');
		if (cell.flagged)
		{
			putchar(';');
			goto flagged; // bypass second check, assume true
		}
	}
	if (cell.flagged)
	{
flagged:
		putchar('9');
	}
	putchar('m');
end_escape:
	if (cell.revealed)
	{
		printf(" %1d ",cell.adjacent);
	} else
	{
		printf("[%1d]",cell.adjacent);
	}
}

void scatter_mines(void)
{
	int mines_left = State.mines;
	while (mines_left > 0)
	{
		uint index;
		retry:
		index = random() % State.size;
		if (State.board[index].is_mine) goto retry;
		int x = index % State.width;
		int y = index / State.width;
		int min_dx = -1;
		int min_dy = -1;
		int max_dx = 1;
		int max_dy = 1;
		if (x == 0) {min_dx = 0;}
		if (x == State.width - 1) {max_dx = 0;}
		if (y == 0) {min_dy = 0;}
		if (y == State.height - 1) {max_dy = 0;}
		// 8^ overflow prevention
		for (int dx = min_dx; dx <= max_dx; ++dx)
		{
			for (int dy = min_dy; dy <= max_dy; ++dy)
			{
				if (index + dx + dy*State.width == State.start_index) {goto retry;}
			}
		}
		State.board[index].is_mine = true;
		for (int dx = min_dx; dx <= max_dx; ++dx)
		{
			for (int dy = min_dy; dy <= max_dy; ++dy)
			{
				State.board[index + dx + dy*State.width].adjacent += 1;
			}
		}
		--mines_left;
	}

}

void init_game(int width, int height, int mines)
{
	State.width = width;
	State.height = height;
	State.size = width * height;
	init_screen();
	// State.board = malloc(sizeof(Cell) * State.size); // does not zero memory
	// memset(State.board, 0, State.size); // requires <string.h>
	State.board = calloc(State.size, sizeof(Cell)); // zeros memory and usually safer
	State.mines = mines;
	scatter_mines();
}

void init_screen(void)
{
	struct winsize T;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&T);
	State.board_x = (T.ws_col - 3 * State.width) / 2 + 1;
	State.board_y = (T.ws_row - State.height) / 2 + 1;
}

// TODO: actual game loop
void run_game(void)
{
	print_grid();
}

void print_grid(void)
{
	enter_screen();
	enter_rawmode();
	fputs("\033[?25l",stdout); // hides cursor
	atexit(exit_rawmode); // because god forbid you get stuck in rawmode
	for (uint y = 0; y < State.height; ++y)
	{
		printf("\033[%d;%dH",State.board_y + y, State.board_x);
		for (uint x = 0; x < State.width; ++x)
		{
			print_cell(State.board[x + y*State.width]);
		}
	}
	fflush(stdout);
	getchar();
	exit_rawmode();
	exit_screen();
	fputs("\033[?25h",stdout); // shows cursor
}

int main(int argc, char** argv)
{
	int seed = time(NULL);
	srandom(seed);
	if (argc < 4)
	{
		fprintf(stderr, "Usage: mines <width> <height> <mines>\n");
		return EXIT_FAILURE;
	}
	int width = atoi(argv[1]);
	if (width <= 0)
	{
		fprintf(stderr, "Please provide a valid integer > 0\n");
		return EXIT_FAILURE;
	}
	int height = atoi(argv[2]);
	if (height <= 0)
	{
		fprintf(stderr, "Please provide a valid integer > 0\n");
		return EXIT_FAILURE;
	}
	int num_mines = atoi(argv[3]);
	if (num_mines <= 0)
	{
		fprintf(stderr, "Please provide a valid integer > 0\n");
		return EXIT_FAILURE;
	} else if (num_mines > width * height - 9)
	{
		fprintf(stderr, "Cannot have more mines than available cells.\n");
		// Eventually, start will guarantee that the selected space has 0 adjacent-
		// that will require that mines < size - 9 or so.
		return EXIT_FAILURE;
	}
	State.start_index = 252; // hard coded for now; in 24x30, 12,10
	init_game(width, height, num_mines);
	run_game();
	free(State.board);
	return EXIT_SUCCESS;
}
