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
} Mine;

void printmine(Mine mine)
{
	char *escape;
	if (mine.is_mine)
	{
		escape = "\e[1m";
	} else {
		escape = "\e[0m";
	}
	printf("%s%d",escape,mine.adjacent);
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
	Mine *board = calloc(sizeof(Mine), size);
	for (int i = 0; i < size; ++i)
	{
		Mine *mine = board+i;
		mine->is_mine = 0;
		mine->revealed = 0;
		mine->flagged = 0;
		mine->adjacent = 0;
	}
	int mines_left = mines;
	while (mines_left > 0)
	{
		int index = random() % size;
		Mine *mine = board + index;
		if (mine->is_mine) continue;
		mine->is_mine = true;
		int x = index % width;
		int y = index / width;
		// printf("Mine index %3d, %2d %2d\n",index,x,y);
		int start_i = -1;
		int start_j = -1;
		int end_i = 2;
		int end_j = 2;
		if (x == 0) {start_i = 0;}
		if (x == width - 1) {end_i = 1;}
		if (y == 0) {start_j = 0;}
		if (y == height - 1) {end_j = 1;}
		for (int i = start_i; i < end_i; ++i)
		{
			for (int j = start_j; j < end_j; ++j)
			{
				(mine+i+width*j)->adjacent += 1;
			}
		}
		--mines_left;
	}
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			printmine(board[i*width+j]);
		}
		putchar('\n');
	}
	free(board);
	return 0;
}
