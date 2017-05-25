#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"

#define DEFAULT_STRING_SIZE     256

int IS_INPUT_NEW;

void read_init();
int is_EOF();
char* next_job();
char* next_command(char** ptr_st);
char* next_arg(char** ptr_st);