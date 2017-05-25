#include "environment.h"



char* shell_arg(int num)
{
    if (num >= SHELL_ARGC)
        return NULL;
    else
        return SHELL_ARGV[num];
}


int environment_init(int argc, char** argv)
{
    char *ptr, *ptr1;
    SHELL_ARGC = argc;
    SHELL_ARGV = argv;
    SHELL_LAST_STATUS = 0;
    ptr1 = realpath("./", SHELL_PWD); 
    ptr = realpath(argv[0], SHELL_PATH); 
    SHELL_PID = getpid();
    SHELL_FD = STDIN_FILENO;
    SHELL_OUTPUT_FD = STDOUT_FILENO;
    SHELL_INTERECTIVE = isatty(SHELL_FD);
    SHELL_EXIT = 0;

    if (SHELL_PWD == NULL || ptr == NULL || ptr1 == NULL)
    {
        perror("Environment initialization failed");
        return E_ENVIRONMET_INIT;
    }

    if (SHELL_INTERECTIVE)
    {
        while (tcgetpgrp(SHELL_FD) != (SHELL_PGID = getpgrp()))
        {
            kill(-SHELL_PGID, SIGTTIN);
        }
        signal(SIGINT,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        
        if (setpgid(SHELL_PID, SHELL_PID) < 0)
        {
            perror("Error in setpgid");
            return E_SETPGID;
        }
        SHELL_PGID = SHELL_PID;

        tcsetpgrp(SHELL_FD, SHELL_PGID);
        tcgetattr(SHELL_FD, &SHELL_TERMINAL_PARAM);

    }

    return E_OK;
}


void environment_destructor()
{
}
