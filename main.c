#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct Cell {
	bool is_mine: 1;
	bool revealed: 1;
	bool flagged: 1;
	uint adjacent: 4;
} Cell;

struct Game {
	Cell *board;
	uint width;
	uint height;
	uint size;
	uint mines;
} Game;

void print_cell(Cell cell);
void scatter_mines(void);
void init_game(int width, int height, int num_mines);
void run_game(void);

// TODO: Needs work. flagged/revealed/just implementation in general
void print_cell(Cell cell)
{
	char *escape;
	if (cell.is_mine)
	{
		escape = "\x1b[1m";
	} else {
		escape = "\x1b[0m";
	}
	printf("%s[%d]",escape,cell.adjacent);
}

void scatter_mines(void)
{
	int mines_left = Game.mines;
	while (mines_left > 0)
	{
		uint index = random() % Game.size;
		if (Game.board[index].is_mine) continue;
		Game.board[index].is_mine = true;
		int x = index % Game.width;
		int y = index / Game.width;
		int min_dx = -1;
		int min_dy = -1;
		int max_dx = 1;
		int max_dy = 1;
		if (x == 0) {min_dx = 0;}
		if (x == Game.width - 1) {max_dx = 0;}
		if (y == 0) {min_dy = 0;}
		if (y == Game.height - 1) {max_dy = 0;}
		// 8^ overflow prevention
		for (int dx = min_dx; dx <= max_dx; ++dx)
		{
			for (int dy = min_dy; dy <= max_dy; ++dy)
			{
				Game.board[index + dx + dy*Game.width].adjacent += 1;
			}
		}
		--mines_left;
	}

}

void init_game(int width, int height, int mines)
{
	Game.width = width;
	Game.height = height;
	Game.size = width * height;
	// Game.board = malloc(sizeof(Cell) * Game.size); // does not zero memory
	// memset(Game.board, 0, Game.size); // requires <string.h>
	Game.board = calloc(Game.size, sizeof(Cell)); // zeros memory and usually safer
	Game.mines = mines;
	scatter_mines();
}

void run_game(void)
{
	for (uint y = 0; y < Game.height; ++y)
	{
		for (uint x = 0; x < Game.width; ++x)
		{
			print_cell(Game.board[x + y*Game.width]);
		}
		putchar('\n');
	}
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
		// that will require that mines < size - 9.
		return EXIT_FAILURE;
	}
	init_game(width, height, num_mines);
	run_game(); // TODO: actual game loop (run_game = 1 iteration?)
	free(Game.board);
	return EXIT_SUCCESS;
}
