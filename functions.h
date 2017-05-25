#ifndef FUNCTIONS
#define FUNCTIONS

#define _XOPEN_SOURCE 1000

#include <stdio.h>

#include "environment.h"
#include "structs.h"
#include "processes.h"

void my_pwd(int argc, char** argv);
void my_jobs(int argc, char** argv);
void my_fg(int argc, char** argv);
void my_bg(int argc, char** argv);
void my_exit(int argc, char** argv);
void my_cd(int argc, char** argv);


#endif