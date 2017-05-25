#define _XOPEN_SOURCE 1000

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>

typedef struct command
{
    int             argc;
    char**          argv;
    char*           input_file;
    char*           output_file;
    int             output_type;
    struct command* next;
    int             pid;
    int             stopped;
    int             completed;
    char*           name;
} command_t;

typedef struct job
{
    int             background;
    command_t*      cmdq;
    pid_t           pgid;
    struct termios  terminal_param; 
    int             notified;
    struct job*     next;
} job_t;


job_t* JOBQ;

command_t* new_command();
void del_command(command_t* cmd);
job_t* new_job();
void del_job(job_t* job);
int is_stopped(job_t* job);
int is_completed(job_t* job);
void print_job(job_t* job);
int job_num(job_t* job);
job_t* job_by_num(int num);