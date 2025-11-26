// #include <bits/types/stack_t.h> // ???? where the fuck did this come from
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
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
	uint cursor_index;
	int status; // 0 for quit, 1 for win, -1 for lose
	uint total_revealed;
} State;

void flag (uint index);
void reveal_near(uint index);
uint adjacent_flagged(uint index);
void reveal(uint index);
void move_cursor(uint index);
void print_cell(Cell cell);
void print_grid(void);
void scatter_mines(uint start_index);
void run_game(void);
void init_screen(void);
void init_game(int width, int height, int mines);

void flag(uint index)
{
	Cell *cell = &State.board[index];
	if (cell->revealed) return;
	cell->flagged ^= true;
}

void reveal_near(uint index)
{
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
	for (int dx = min_dx; dx <= max_dx; ++dx)
	{
		for (int dy = min_dy; dy <= max_dy; ++dy)
		{
			// fprintf(stderr, "%d, %d\n", dx, dy);
			if (dx == 0 && dy == 0) continue;
			if (State.board[index + dx + dy*State.width].revealed) continue;
			reveal(index + dx + dy * State.width);
		}
	}
}

uint adjacent_flagged(uint index)
{
	uint adj_flag = 0;
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
	for (int dx = min_dx; dx <= max_dx; ++dx)
	{
		for (int dy = min_dy; dy <= max_dy; ++dy)
		{
			if (dx == 0 && dy == 0) continue;
			if (State.board[index + dx + dy*State.width].flagged)
			{
				adj_flag += 1;
			}
		}
	}
	return adj_flag;
}

void reveal(uint index)
{
	Cell *cell = &State.board[index];
	if (cell->flagged) return;
	if (cell->is_mine)
	{
		State.status = -1;
	} else if (!cell->revealed)
	{
		cell->revealed = true;
		State.total_revealed += 1;
		if (cell->adjacent == 0)
		{
			reveal_near(index);
		}
	} else if (cell->adjacent > 0
	           && adjacent_flagged(index) == cell->adjacent)
	{
		reveal_near(index);
	}
}

void move_cursor(uint index)
{
	printf("\033[%d;%dH",
		State.board_y + (index / State.width),
		State.board_x + 3*(index % State.width) + 1
	);
	fflush(stdout);
}

void print_cell(Cell cell)
{
	fputs("\033[", stdout);
	if (cell.flagged) fputs("9", stdout);
	else fputs("29", stdout);
	putchar('m');
	if (cell.revealed)
	{
		printf(" %1d ",cell.adjacent);
	} else
	{
		printf("[ ]");
	}
}

void print_grid(void)
{
	for (uint y = 0; y < State.height; ++y)
	{
		printf("\033[%d;%dH",State.board_y + y, State.board_x);
		for (uint x = 0; x < State.width; ++x)
		{
			print_cell(State.board[x + y*State.width]);
		}
	}
	fflush(stdout);
}

void scatter_mines(uint start_index)
{
	int mines_left = State.mines;
	while (mines_left > 0)
	{
		uint index;
retry: // because `continue` would not work in `for` blocks
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
				if (index + dx + dy*State.width == start_index) goto retry;
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

void left(void) {
	if (State.cursor_index % State.width == 0) return;
	State.cursor_index -= 1;
}
void down(void) {
	if (State.cursor_index / State.width == State.height - 1) return;
	State.cursor_index += State.width;
}
void up(void) {
	if (State.cursor_index < State.width) return;
	State.cursor_index -= State.width;
}
void right(void) {
	if (State.cursor_index % State.width == State.width - 1) return;
	State.cursor_index += 1;
}
void run_game(void)
{
	enter_screen();
	enter_rawmode();
	atexit(exit_rawmode);
	print_grid();
	char ch;
	while (true)
	{
		move_cursor(State.cursor_index);
		ch = getc_escsafe();
		switch (ch) {
		case 'q':
			goto exit;
		case 'h':
			left();
			break;
		case 'j':
			down();
			break;
		case 'k':
			up();
			break;
		case 'l':
			right();
			break;
		case 'y':
			up(); left();
			break;
		case 'u':
			up(); right();
			break;
		case 'b':
			down(); left();
			break;
		case 'n':
			down(); right();
			break;
		case 'a':
		case 's':
		case 'd':
		case ' ':
			scatter_mines(State.cursor_index);
			reveal(State.cursor_index);
			goto after_mines;
		}
	}
after_mines:
	while (State.status == 0)
	{
		if (State.total_revealed == State.size - State.mines)
		{
			State.status = 1;
			goto exit;
		}
		print_grid();
		move_cursor(State.cursor_index);
		ch = getc_escsafe();
		switch (ch) {
		case 'q':
			goto exit;
		case 'h':
			left();
			break;
		case 'j':
			down();
			break;
		case 'k':
			up();
			break;
		case 'l':
			right();
			break;
		case 'y':
			up(); left();
			break;
		case 'u':
			up(); right();
			break;
		case 'b':
			down(); left();
			break;
		case 'n':
			down(); right();
			break;
		case 'a':
		case 's':
		case 'd':
		case ' ':
			reveal(State.cursor_index);
			break;
		case 'f':
			flag(State.cursor_index);
		}
	}
exit:
	exit_rawmode();
	exit_screen();
	switch (State.status) {
	case -1:
		puts("You lost!");
		break;
	case 0:
		puts("You quit!");
		break;
	case 1:
		puts("You won!");
	}
}

void init_screen(void)
{
	struct winsize T;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&T);
	State.board_x = (T.ws_col - 3 * State.width) / 2 + 1;
	State.board_y = (T.ws_row - State.height) / 2 + 1;
}

void init_game(int width, int height, int mines)
{
	State.width = width;
	State.height = height;
	State.size = width * height;
	init_screen();
	State.board = calloc(State.size, sizeof(Cell));
	State.mines = mines;
	State.cursor_index = 0;
	State.status = 0;
	State.total_revealed = 0;
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
	int mines = atoi(argv[3]);
	if (mines <= 0)
	{
		fprintf(stderr, "Please provide a valid integer > 0\n");
		return EXIT_FAILURE;
	} else if (mines > width * height - 9)
	{
		fprintf(stderr,
		        "Cannot have more mines than available cells.\n"
		);
		/* Eventually, start will guarantee that the selected space
		has 0 adjacent- that will require that mines < size - 9 or so. */
		return EXIT_FAILURE;
	}
	init_game(width, height, mines);
	run_game();
	free(State.board);
	// ^uneccesary since program is already at exit but whatever
	return EXIT_SUCCESS;
}
