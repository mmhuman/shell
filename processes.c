#include "processes.h"


int upd_command_status(pid_t pid, int status)
{
    job_t* job;
    for (job = JOBQ; job != NULL; job = job->next)
    {
        command_t* cmd;
        for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
        {
            if (cmd->pid == pid)
            {
                SHELL_LAST_STATUS = status;
                if (WIFSTOPPED(status))
                {
                    cmd->stopped = 1;
                }
                else
                {
                    cmd->completed = 1;
                    if (is_completed(job))
                        job->notified = 0;
                    if (WIFSIGNALED(status))
                    {
                        fprintf(stderr, "\n%d: Terminated by signal %d.\n", (int)pid, WTERMSIG(status));
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

void upd_jobs_status()
{
    int status;
    pid_t pid;
    while (1)
    {
        pid = waitpid(-1, &status, WUNTRACED | WNOHANG);
        /*fprintf(stderr, "upd_j_stat: pid = %d status = %d stopped = %d signaled = %d\n", pid, status, WIFSTOPPED(status), WIFSIGNALED(status));*/
        if (!upd_command_status(pid, status))
            break;
    }
}

void wait_for_job(job_t* job)
{
    int status;
    pid_t pid;
    int cnt_iter = 0;
    /*fprintf(stderr, "pid = %d\n", job->cmdq->pid);*/
    while (!is_stopped(job) && !is_completed(job))
    {
        ++cnt_iter;
        pid = waitpid(-1, &status, WUNTRACED);
        /*fprintf(stderr, "waitpid: pid = %d status = %d stopped = %d signaled = %d\n", pid, status, WIFSTOPPED(status), WIFSIGNALED(status));*/
        upd_command_status(pid, status);
        if (cnt_iter > 10000) {
            perror("cnt_iter of waitpid > 10000");
            exit(123);
        }
    }    
}

void put_job_in_background(job_t* job, int cont)
{
    printf("[%d] %d\n", job_num(job), job->pgid);
    if (cont)
    {
        if (kill(-job->pgid, SIGCONT) < 0)
        {
            perror ("Error in kill(-j->pgid, SIGCONT)");
        }
    }
}

void put_job_in_foreground(job_t* job, int cont)
{
    tcsetpgrp(SHELL_FD, job->pgid);
    if (cont)
    {
        tcsetattr(SHELL_FD, TCSADRAIN, &job->terminal_param);
        if (kill(-job->pgid, SIGCONT) < 0)
        {
            perror("Error in kill(-job->pgid, SIGCONT)");
        }
    }
    wait_for_job(job);

    tcsetpgrp(SHELL_FD, SHELL_PGID);
    tcgetattr(SHELL_FD, &job->terminal_param);
    tcsetattr(SHELL_FD, TCSADRAIN, &SHELL_TERMINAL_PARAM);

    if (is_completed(job))
    {
        del_job(job);
    }
}

int fd_from_filename(char* filename, char* mode)
{
    int fd;
    FILE* file = fopen(filename, mode);
    if (file == NULL)
    {
        perror("Error in fopen");
        exit(E_FOPEN);
    }
    fd = fileno(file);
    if (fd == -1)
    {
        perror("Error in fileno");
        exit(E_FILENO);
    }
    return fd;
}

void command_execute(command_t* cmd, pid_t pgid, int input_fd, int output_fd, int foreground)
{
    pid_t pid;
    /*fprintf(stderr, "%s\n", "command_execute started");*/

    if (SHELL_INTERECTIVE)
    {
        pid = getpid();
        if (pgid == 0)
            pgid = pid;
        setpgid(pid, pgid);
        if (foreground)
        {
            tcsetpgrp(SHELL_FD, pgid);
        }

        signal(SIGINT,  SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
    }
    if (cmd->input_file != NULL)
    {
        input_fd = fd_from_filename(cmd->input_file, "r");
    }
    if (cmd->output_file != NULL)
    {
        if (cmd->output_type == 0)
            output_fd = fd_from_filename(cmd->output_file, "w");
        else
            output_fd = fd_from_filename(cmd->output_file, "a");
    }

    if (input_fd != STDIN_FILENO)
    {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO)
    {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    /*fprintf(stderr, "argc = %d argv[0] = %s argv[1] = %s\n", cmd->argc, cmd->argv[0], cmd->argv[1]);*/
    cmd->argv[cmd->argc] = NULL;
    if (strcmp(cmd->argv[0], "pwd") == 0)
    {
        my_pwd(cmd->argc, cmd->argv);
        exit(E_OK);
    }
    else
    if (strcmp(cmd->argv[0], "jobs") == 0)
    {
        my_jobs(cmd->argc, cmd->argv);
        exit(E_OK);
    }
    else
    {
        if (cmd->argv[0][0] == '.')
        {
            cmd->name = (char*)malloc(sizeof(char) * (strlen(cmd->argv[0]) + PATH_MAX + 5));
            strcpy(cmd->name, SHELL_PWD);
            strcat(cmd->name, "/");
            strcat(cmd->name, cmd->argv[0]);
            /*fprintf(stderr, "cmd->name = %s\n", cmd->name);*/
            execvp(cmd->name, cmd->argv);
        }
        else
        {
            execvp(cmd->argv[0], cmd->argv);
        }
    }
    perror("Error in execvp");
    exit(E_EXECVP);
}

int job_execute(job_t* job)
{
    command_t* cmd;
    int mypipe[2], input_fd, output_fd;

    /*fprintf(stderr, "%s\n", "job_execute started");*/

    input_fd = SHELL_FD;

    for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
    {
        pid_t child_pid;
        if (cmd->next != NULL)
        {
            if (pipe(mypipe) < 0)
            {
                perror("Error in pipe installation");
                return E_PIPE;
            }
            output_fd = mypipe[1];
        }
        else
            output_fd = SHELL_OUTPUT_FD;

        child_pid = fork();
        if (child_pid == 0)
        {
            child_pid = getpid();
            /*в command_execute будет вызван exit*/
            command_execute(cmd, job->pgid, input_fd, output_fd, !job->background);
        }
        else
        if (child_pid < 0)
        {
            perror("Error in fork");
            return E_FORK;
        }
        else
        {
            cmd->pid = child_pid;
            if (SHELL_INTERECTIVE)
            {
                if (job->pgid == 0)
                    job->pgid = child_pid;
                setpgid(child_pid, job->pgid);
            }
        }

        if (input_fd != SHELL_FD)
            close(input_fd);
        if (output_fd != SHELL_OUTPUT_FD)
            close(output_fd);
        input_fd = mypipe[0];
    }


    if (!SHELL_INTERECTIVE)
        wait_for_job(job);
    else
    if (!job->background)
        put_job_in_foreground(job, 0);
    else
        put_job_in_background(job, 0);
    return E_OK;
}


void job_continue(job_t *job)
{
    command_t* cmd;
    for (cmd = job->cmdq; cmd != NULL; cmd = cmd->next)
    {
        cmd->stopped = 0;
    }
    if (!job->background)
        put_job_in_foreground(job, 1);
    else
        put_job_in_background(job, 1);
}

void job_notificaton()
{
    job_t *job, *nxt;
    for (job = JOBQ; job != NULL; job = nxt)
    {
        nxt = job->next;
        if (!job->notified)
        {
            job->notified = 1;
            if (is_completed(job))
            {
                print_job(job);
                del_job(job);
            }
            else
            if (is_stopped(job))
            {
                print_job(job);
            }
            else
            {
                printf("Something unexpected happened with job:\n");
                print_job(job);
            }
        }
    }
}