#include <stdio.h>
#include <stdlib.h>

#define E_OK                    0
#define E_ENVIRONMET_INIT       1
#define E_SETPGID               2
#define E_PIPE                  3
#define E_FORK                  4
#define E_EXECVP                5
#define E_FOPEN                 6
#define E_FILENO                7


void print_my_error(int err);

