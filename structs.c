#include "structs.h"

command_t* new_command()
{
    command_t* cmd =    (command_t*)malloc(sizeof(command_t));
    cmd->name =         NULL;
    cmd->argc =         0;
    cmd->argv =         NULL;
    cmd->input_file =   NULL;
    cmd->output_file =  NULL;
    cmd->output_type =  0;
    cmd->next =         NULL;
    cmd->pid =          0;
    cmd->stopped =      0;
    cmd->completed =    0;
    return cmd;
}

void del_command(command_t* cmd)
{
    
    if (cmd->name != NULL)
    {
        free(cmd->name);
        /*printf("free cmd->name\n");*/
    }

    if (cmd->input_file != NULL)
        free(cmd->input_file);
    if (cmd->output_file != NULL)
        free(cmd->output_file);
    if (cmd->argv != NULL) {
        int i;
        for (i = 0; i < cmd->argc; ++i)
        {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    free(cmd);
}

job_t* new_job()
{
    job_t* job = (job_t*)malloc(sizeof(job_t));
    job->background = 0;
    job->pgid = 0;
    job->cmdq = NULL;
    job->notified = 1;
    job->next = NULL;
    return job;
}


void del_job(job_t* job)
{
    job_t* iter;
    job_t* prev = NULL;
    command_t* cmd;
    for (iter = JOBQ; iter != NULL; iter = iter->next)
    {
        if (iter == job)
        {
            if (prev != NULL)
                prev->next = iter->next;
            break;
        }
        prev = iter;
    }
    if (job == JOBQ) {
        JOBQ = job->next;
    }
    for (; job->cmdq != NULL;)
    {
        cmd = job->cmdq->next;
        del_command(job->cmdq);
        job->cmdq = cmd;
    }
    free(job);
}

int is_stopped(job_t* job)
{
    command_t* cmd;
    for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
    {
        if (cmd->stopped)
            return 1;
        if (cmd->completed)
            continue;
        return 0;
    }
    return 1;
}

int is_completed(job_t* job)
{
    command_t* cmd;
    for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
    {
        if (!cmd->completed)
            return 0;
    }
    return 1;
}


void print_job(job_t* job)
{
    command_t* cmd;
    printf("pgid=%d", job->pgid);
    if (job->background)
        printf("&");
    printf(" ");
    if (is_completed(job))
        printf("(completed)");
    else
    if (is_stopped(job))
        printf("(stopped)");
    else
        printf("         ");
    printf("\t\t");
    for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
    {
        int i;
        for (i = 0; i < cmd->argc; ++i)
        {
            printf("%s ", cmd->argv[i]);
        }
        if (cmd->next != NULL)
            printf("| ");
    }
    printf("\n");
}

int job_num(job_t* job)
{
    int job_cnt;
    int ans;
    job_t* iter;
    job_cnt = 0;
    ans = -1;
    for (iter = JOBQ; iter != NULL; iter = iter->next)
    {
        if (iter == job)
            ans = job_cnt;
        ++job_cnt;
    }
    return job_cnt - ans;
}

job_t* job_by_num(int num)
{
    int job_cnt;
    job_t* iter;
    job_cnt = 0;
    for (iter = JOBQ; iter != NULL; iter = iter->next)
    {
        ++job_cnt;
    }
    for (iter = JOBQ; iter != NULL; iter = iter->next)
    {
        if (job_cnt == num)
            return iter;
        --job_cnt;
    }
    return NULL;
}