#include "functions.h"

void my_pwd(int argc, char** argv)
{
    printf("%s\n", SHELL_PWD);
}

int print_jobq(job_t* job)
{
    int num;
    if (job == NULL)
        return 0;
    num = print_jobq(job->next) + 1;
    printf("[%d] ", num);
    print_job(job);
    return num;
}

void my_jobs(int argc, char** argv)
{
    print_jobq(JOBQ->next);
}

job_t* job_for_bfg(int argc, char** argv)
{
    job_t* job;
    if (argc < 2)
    {
        for (job = JOBQ->next; job != NULL; job = job->next)
        {
            if (!is_completed(job))
            {
                break;
            }
        }
    }
    else
    {
        int job_num = atoi(argv[1]);
        job = job_by_num(job_num);
    }
    if (job == NULL)
    {
        fprintf(stderr, "Corresponding job not found\n");
        return NULL;
    }
    if (is_completed(job))
    {
        fprintf(stderr, "Job is already completed\n");
        return NULL;
    }
    if (!is_stopped(job))
    {
        kill(-job->pgid, SIGSTOP);
    }
    return job;
}


void my_fg(int argc, char** argv)
{
    job_t* job;
    job = job_for_bfg(argc, argv);
    if (job == NULL)
        return;
    job->background = 0;
    job_continue(job);
}


void my_bg(int argc, char** argv)
{
    job_t* job;
    job = job_for_bfg(argc, argv);
    if (job == NULL)
        return;
    job->background = 1;
    job_continue(job);
}

void my_exit(int argc, char** argv)
{
    SHELL_EXIT = 1;
}

void my_cd(int argc, char** argv)
{
    char* path;
    char* ptr;
    int _e;
    if (argc < 2)
    {
        fprintf(stderr, "Too few arguments\n");
        return;
    }
    path = argv[1];
    if (path[0] == '.')
    {
        char* tmp;
        tmp = (char*)malloc(sizeof(char) * (strlen(path) + PATH_MAX + 5));
        strcpy(tmp, SHELL_PWD);
        strcat(tmp, "/");
        strcat(tmp, path);
        free(path);
        path = tmp;
        argv[1] = path;
    }
    ptr = realpath(path, SHELL_PWD);
    if (ptr == NULL)
    {
        perror("Error in realpath");
        return;
    }
    _e = setenv("PWD", SHELL_PWD, 1);
    /*fprintf(stderr, "cd: path = %s\nSHELL_PWD = %s\n", path, SHELL_PWD);*/
    if (_e == -1)
    {
        perror("Error in setenv");
        return;
    }
}