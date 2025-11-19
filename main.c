#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct {
	bool is_mine: 1;
	bool revealed: 1;
	bool flagged: 1;
	unsigned char adjacent: 4;
} Cell;

void printcell(Cell cell)
// TODO: Needs work. flagged/revealed/just implementation in general
{
	char *escape;
	if (cell.is_mine)
	{
		escape = "\e[1m";
	} else {
		escape = "\e[0m";
	}
	printf("%s%d",escape,cell.adjacent);
	// free(escape);
}

int main(int argc, char** argv)
{
	int seed = time(NULL);
	// printf("Seed: %d\n", seed);
	srandom(seed);
	if (argc < 4)
	{
		printf("Please supply <width> <height> <mines>\n");
		return 1;
	}
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int size = width * height;
	int mines = atoi(argv[3]);
	Cell *board = calloc(sizeof(Cell), size);
	// for (int i = 0; i < size; ++i)
	// // Initialize each cell (unnecessary because calloc?)
	// {
	// 	Cell *cell = board+i;
	// 	cell->is_mine = 0;
	// 	cell->revealed = 0;
	// 	cell->flagged = 0;
	// 	cell->adjacent = 0;
	// }
	int mines_left = mines;
	while (mines_left > 0)
	// Scatter mines
	{
		int index = random() % size;
		Cell *mine = board + index;
		if (mine->is_mine) continue;
		mine->is_mine = true;
		int x = index % width;
		int y = index / width;
		// printf("Cell index %3d, %2d %2d\n",index,x,y);
		int min_dx = -1;
		int min_dy = -1;
		int max_dx = 1;
		int max_dy = 1;
		if (x == 0) {min_dx = 0;}
		if (x == width - 1) {max_dx = 0;}
		if (y == 0) {min_dy = 0;}
		if (y == height - 1) {max_dy = 0;}
		// 8^ overflow prevention
		for (int dx = min_dx; dx <= max_dx; ++dx)
		{
			for (int dy = min_dy; dy <= max_dy; ++dy)
			{
				(mine+dx+width*dy)->adjacent += 1;
			}
		}
		--mines_left;
	}
	for (int x = 0; x < height; ++x)
	{
		for (int y = 0; y < width; ++y)
		{
			printcell(board[x*width+y]);
		}
		putchar('\n');
	}
	free(board);
	return 0;
}
