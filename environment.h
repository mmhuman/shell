#define _XOPEN_SOURCE 1000

#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <linux/limits.h>

#include "_error.h"

int environment_init(int argc, char** argv);
void environment_destructor();

int             SHELL_ARGC;
char**          SHELL_ARGV;
int             SHELL_LAST_STATUS;
char            SHELL_PWD[PATH_MAX + 1];
char            SHELL_PATH[PATH_MAX + 1];
pid_t           SHELL_PID;
pid_t           SHELL_PGID;
int             SHELL_FD;
int             SHELL_OUTPUT_FD;
int             SHELL_INTERECTIVE;
struct termios  SHELL_TERMINAL_PARAM;
int             SHELL_EXIT;