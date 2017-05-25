#ifndef PROCESSES
#define PROCESSES

#define _XOPEN_SOURCE 1000

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>

#include "functions.h"
#include "_error.h"


void wait_for_job(job_t* job);
int job_execute(job_t* job);
void job_notificaton();
void upd_jobs_status();
void job_continue(job_t *job);

#endif