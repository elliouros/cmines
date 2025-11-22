#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

static struct termios orig_termios;
static bool screen = false;
static bool rawmode = false;

// basically stolen from antirez/kilo, although uses cfmakeraw instead of
// manually setting options.
int enter_rawmode(void)
{
	if (rawmode) return 0;
	if (!isatty(STDIN_FILENO)) goto fatal;
	if (tcgetattr(STDOUT_FILENO,&orig_termios) < 0) goto fatal;
	struct termios raw = orig_termios;
	cfmakeraw(&raw);
	if (tcsetattr(STDOUT_FILENO,TCSAFLUSH,&raw) < 0) goto fatal;
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
		tcsetattr(STDOUT_FILENO,TCSAFLUSH,&orig_termios);
		rawmode = false;
	}
}

void enter_screen(void)
{
	if (!screen)
	{
		// \0337     save cursor pos
		// \033[?47h save screen
		fputs("\0337\033[?47h",stdout);
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
		fputs("\033[?47l\0338",stdout);
		fflush(stdout);
		screen = false;
	}
}
