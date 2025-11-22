#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

void enter_screen(void);
void exit_screen(void);

void enter_rawmode(void);
void exit_rawmode(void);
