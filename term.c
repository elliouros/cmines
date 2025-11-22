#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

static struct termios orig_termios;
static bool screen = false;
static bool rawmode = false;

// A lot of this code is either stolen from or inspired by antirez/kilo.
// There are differences present throughout but for the most part it's just
// patchwritten from kilo. Thanks antirez.

// Ignores escape sequences, returning ESC for any sequence.
// returns \0 if any errors occur with initial read (probably)
unsigned char getc_escsafe(void)
{
	char ch;
	int origin = fcntl(STDIN_FILENO, F_GETFL);
	ssize_t success = read(STDIN_FILENO, &ch, 1);
	if (success < 1 || ch < 0) return 0;
	if (ch == '\033')
	{
		fcntl(STDIN_FILENO, F_SETFL, origin | O_NONBLOCK);
		while (getchar() != EOF);
		fcntl(STDIN_FILENO, F_SETFL, origin);
	}
	return ch;
}

int enter_rawmode(void)
{
	if (rawmode) return 0;
	if (!isatty(STDIN_FILENO)) goto fatal;
	if (tcgetattr(STDOUT_FILENO, &orig_termios) < 0) goto fatal;
	struct termios raw = orig_termios;
	cfmakeraw(&raw);
	if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw) < 0) goto fatal;
	rawmode = true;
	return 0;
fatal:
	errno = ENOTTY;
	return -1;
}

void exit_rawmode(void)
{
	if (rawmode)
	{
		tcsetattr(STDOUT_FILENO, TCSAFLUSH, &orig_termios);
		rawmode = false;
	}
}

void enter_screen(void)
{
	if (!screen)
	{
		// \0337     save cursor pos
		// \033[?47h save screen
		// \033[2J
		fputs("\0337\033[?47h\033[2J", stdout);
		fflush(stdout);
		screen = true;
	}
}

void exit_screen(void)
{
	if (screen)
	{
		// \033[?47l load screen
		// \0338     load cursor pos
		fputs("\033[?47l\0338", stdout);
		fflush(stdout);
		screen = false;
	}
}
