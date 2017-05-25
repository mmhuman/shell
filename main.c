#include "main.h"


void char_array_resize(char*** st, int old_size, int new_size)
{
    char** tmp = (char**)malloc(sizeof(char*) * new_size);
    if ((*st) != NULL)
    {
        int i;
        for (i = 0; i < old_size && i < new_size; ++i)
        {
            tmp[i] = (*st)[i];
        }
        free((*st));
    }
    (*st) = tmp;
}

command_t* command_handler(char* command)
{
    int size;
    int argc;
    char** argv;
    int _error;
    command_t* cmd;
    _error = 0;
    cmd = new_command();

    argc = 0;
    size = 0;
    argv = NULL;
    char_array_resize(&argv, size, DEFAULT_ARRAY_SIZE);
    size = DEFAULT_ARRAY_SIZE;

    while (*command != 0)
    {
        if (argc + 1 >= size)
        {
            char_array_resize(&argv, size, size * 2);
            size *= 2;
        }
        argv[argc] = next_arg(&command);
        if (argv[argc] == NULL)
            break;
        if (strcmp(argv[argc], "<") == 0 || strcmp(argv[argc], ">") == 0 || strcmp(argv[argc], ">>") == 0)
        {
            char* file;
            file = next_arg(&command);
            if (file == NULL)
            {
                fprintf(stderr, "Syntax error in stream redirection\n");
                fprintf(stderr, "Expected input file after '%s'\n", argv[argc]);
                _error = 1;
                free(argv[argc]);
                break;
            }
            if (strcmp(argv[argc], "<") == 0)
            {
                cmd->input_file = file;
            }
            else
            {
                cmd->output_file = file;
                if (strcmp(argv[argc], ">>") == 0)
                    cmd->output_type = 1;
            }
            free(argv[argc]);
            continue;
        }
        ++argc;
    }
    if (argc == 0)
        _error = 1;
    if (_error)
    {
        del_command(cmd);
        return NULL;
    }
    else
    {
        cmd->argc = argc;
        cmd->argv = argv;
        /*printf("argv[0] = %s\n", argv[0]);*/
        return cmd;
    }
}


void job_handler(char* job_st)
{
    job_t* job;
    command_t* last_cmd;
    int _error;
    _error = 1;
    job = new_job();
    last_cmd = NULL;


    while (*job_st != 0)
    {
        char* command = next_command(&job_st);
        command_t* cmd;
        if (command == NULL)
            break;
        if (strcmp(command, "&") == 0)
        {
            free(command);
            job->background = 1;
            break;
        }
        cmd = command_handler(command);
        if (cmd == NULL)
        {
            fprintf(stderr, "Syntax error in command: %s\n", command);
            fprintf(stderr, "Job ignored\n");
            _error = 1;
            free(command);
            break;
        }
        free(command);
        if (job->cmdq == NULL)
        {
            _error = 0;
            job->cmdq = cmd;
            last_cmd = cmd;   
        }
        else
        {
            last_cmd->next = cmd;
            last_cmd = cmd;
        }
    }
    /*
    print_job_info(job);
    */
    if (_error)
    {
        del_job(job);
    }
    else
    {
        job->next = JOBQ;
        JOBQ = job;
        if (strcmp(job->cmdq->argv[0], "fg") == 0 ||
            strcmp(job->cmdq->argv[0], "bg") == 0 ||
            strcmp(job->cmdq->argv[0], "exit") == 0 ||
            strcmp(job->cmdq->argv[0], "cd") == 0)
        {
            if (job->cmdq->next != NULL)
            {
                fprintf(stderr, "You can't use %s in pipe line\n", job->cmdq->argv[0]);
                del_job(job);
                return;
            }
        }
        else
        {
            job_execute(job); 
            return;
        }
        if (strcmp(job->cmdq->argv[0], "fg") == 0)
        {
            my_fg(job->cmdq->argc, job->cmdq->argv);
        }
        else
        if (strcmp(job->cmdq->argv[0], "bg") == 0)
        {
            my_bg(job->cmdq->argc, job->cmdq->argv);
        }
        else
        if (strcmp(job->cmdq->argv[0], "exit") == 0)
        {
            my_exit(job->cmdq->argc, job->cmdq->argv);
        }
        else
        if (strcmp(job->cmdq->argv[0], "cd") == 0)
        {
            my_cd(job->cmdq->argc, job->cmdq->argv);
        }
        del_job(job);
    }
}

void listener()
{
    while (!is_EOF() && !SHELL_EXIT)
    {
        char* job;
        if (IS_INPUT_NEW)
        {
            upd_jobs_status();
            job_notificaton();
            if (SHELL_INTERECTIVE)
                printf("maxshell$ ");
        }
        job = next_job();
        if (job == NULL)
        {
            continue;
        }
        job_handler(job);
        free(job);
    }
}


int init(int argc, char** argv)
{
    int err;
    if ((err = environment_init(argc, argv)) != E_OK)
    {
        return err;
    }
    read_init();
    JOBQ = NULL;
    return E_OK;
}

void destructor()
{
    environment_destructor();
}

int main(int argc, char** argv)
{
    int err;
    if ((err = init(argc, argv)) != E_OK)
    {
        return err;
    }
    listener();
    destructor();

    return E_OK;
}