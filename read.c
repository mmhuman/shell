#include "read.h"

int already_EOF;

void read_init()
{
    already_EOF = 0;
    IS_INPUT_NEW = 1;
}

int is_EOF()
{
    return already_EOF;
}

void string_resize(char** st, int old_size, int new_size)
{
    char* tmp = (char*)malloc(sizeof(char) * new_size);
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


int is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

void skip_spaces(char** ptr_st)
{
    char* st = *ptr_st;
    while (is_space(*st))
    {
        ++st;
    }
    (*ptr_st) = st;
}


int next_char()
{
    int c;
    c = getc(stdin);
    if (c == EOF)
        already_EOF = 1;
    return c;
}

void safestrcat(char** st_ptr, int* len_ptr, int* size_ptr, char* tmp)
{
    char* st;
    int len, size;
    st = *st_ptr;
    len = *len_ptr;
    size = *size_ptr;
    st[len] = 0;
    while (size < len + strlen(tmp))
    {
        string_resize(&st, size, 2 * size);
        size *= 2;
    }
    len += strlen(tmp);
    strcat(st, tmp);
    (*st_ptr) = st;
    (*len_ptr) = len;
    (*size_ptr) = size;
}

int comp_end(char* src, int len, char* tmp)
{
    int tmp_len = strlen(tmp);
    int i;
    if (tmp_len > len)
        return 0;
    for (i = 0; i < tmp_len; ++i)
    {
        if (src[len - tmp_len + i] != tmp[i])
            return 0;
    }
    return 1;
}

char* st_from_long(long a)
{
    char* res = (char*)malloc(sizeof(char) * 25);
    int len;
    long ten_pow;
    len = 0;
    if (a < 0)
    {
        res[len] = '-';
        ++len;
        a = -a;
    }
    ten_pow = 1;
    while ((long)10 * ten_pow > 0 &&  (long)10 * ten_pow <= a)
    {
        ten_pow *= (long)10;
    }
    while (ten_pow > 0)
    {
        res[len] = (char)('0' + a / ten_pow);
        ++len;
        a %= ten_pow;
        ten_pow /= 10;
    }
    res[len] = 0;
    return res;
}

void service_built_in_vars(char** st_ptr, int* len_ptr, int* size_ptr)
{
    char* st;
    int len, size;
    st = *st_ptr;
    len = *len_ptr;
    size = *size_ptr;
    st[len] = 0;
    if (len > 2 && st[len - 2] == '$' && st[len - 1] >= '0' && st[len - 1] <= '9')
    {
        if (SHELL_ARGC > st[len - 1] - '0')
        {
            len -= 2;
            safestrcat(&st, &len, &size, SHELL_ARGV[st[len + 1] - '0']);
        }
    }
    else
    if (comp_end(st, len, "$#"))
    {
        char* tmp = st_from_long(SHELL_ARGC);
        len -= 2;
        safestrcat(&st, &len, &size, tmp);
        free(tmp);
    }
    else
    if (comp_end(st, len, "$?"))
    {
        char* tmp = st_from_long(SHELL_LAST_STATUS);
        len -= 2;
        safestrcat(&st, &len, &size, tmp);
        free(tmp);
    }
    else
    if (comp_end(st, len, "${SHELL}"))
    {
        len -= strlen("${SHELL}");
        safestrcat(&st, &len, &size, SHELL_PATH);
    }
    else
    if (comp_end(st, len, "${PWD}"))
    {
        len -= strlen("${PWD}");
        safestrcat(&st, &len, &size, SHELL_PWD);
    }
    else
    if (comp_end(st, len, "${PID}"))
    {
        char* tmp = st_from_long(SHELL_PID);
        len -= strlen("${PID}");
        safestrcat(&st, &len, &size, tmp);
        free(tmp);
    }
    (*st_ptr) = st;
    (*len_ptr) = len;
    (*size_ptr) = size;
}


/**
*   Функция считывает следующую "работу".
*   Корнер кейсы:
*   кавычки начинаються с " (') , значение в кавычках считываеться до следующей " ('),
*   перед которой нету \ , (даже если встретился \n)
*/
char* next_job()
{
    int len, size;
    char* job;
    IS_INPUT_NEW = 0;
    job = NULL;
    len = 0;
    size = 0;
    string_resize(&job, size, DEFAULT_STRING_SIZE);
    size = DEFAULT_STRING_SIZE;
    while (1)
    {
        int c_int;
        c_int = next_char();
        if (c_int == EOF || c_int == ';')
        {
            break;
        }
        else
        if (c_int == ' ' && len > 0 && job[len - 1] == '\\')
        {
            --len;
            continue;
        }
        else
        if (c_int == '\n')
        {
            if (len > 0 && job[len - 1] == '\\') {
                --len;
                continue;
            }
            IS_INPUT_NEW = 1;
            break;
        }
        else
        if (c_int == '#')
        {
            if (len > 0 && job[len - 1] == '\\')
            {
                --len;
            }
            else 
            {
                IS_INPUT_NEW = 1;
                while (c_int != '\n' && c_int != EOF)
                {
                    c_int = next_char();
                }
                break;
            }
        }
        else
        if (c_int == '"' || c_int == '\'')
        {
            int quote = c_int;
            job[len] = c_int;
            ++len;
            while (1)
            {
                c_int = next_char();
                if (c_int == EOF)
                {
                    fprintf(stderr, "Unexpected EOF\n");
                    free(job);
                    return NULL;
                }
                if (c_int == quote)
                {
                    if (job[len - 1] != '\\')
                    {
                        break;
                    }
                }
                if (len + 1 >= size)
                {
                    string_resize(&job, size, 2 * size);
                    size *= 2;
                }
                job[len] = (char)c_int;
                ++len;
                if (quote == '"')
                {
                    service_built_in_vars(&job, &len, &size);
                }
            }
        }
        if (len + 1 >= size)
        {
            string_resize(&job, size, 2 * size);
            size *= 2;
        }

        job[len] = (char)c_int;
        ++len;
        if (c_int == '&')
        {
            break;
        }
    }
    job[len] = 0;
    {
        int blank = 1;
        int i;
        for (i = 0; i < len; ++i)
        {
            if (job[i] != ' ' && job[i] != '\t')
            {
                blank = 0;
                break;
            }
        }
        if (blank)
        {
            free(job);
            return NULL;
        }
    }
    return job;
}


char* next_command(char** ptr_st)
{
    char* st = *ptr_st;
    int len, size;
    char* command;
    command = NULL;
    len = 0;
    size = 0;
    string_resize(&command, size, DEFAULT_STRING_SIZE);
    size = DEFAULT_STRING_SIZE;

    skip_spaces(&st);
    while (1)
    {
        int c_int;
        c_int = (*st);
        ++st;
        if (c_int == '|')
        {
            break;
        }
        else
        if (c_int == 0)
        {
            --st;
            break;
        }
        else
        if (c_int == '&')
        {
            if (len == 0)
            {
                command[0] = '&';
                len = 1;
            }
            else
            {
                --st;
            }
            break;
        }
        else
        if (c_int == '"' || c_int == '\'')
        {
            int quote = c_int;
            command[len] = c_int;
            ++len;
            while (1)
            {
                c_int = (*st);
                ++st;
                if (c_int == quote)
                {
                    if (command[len - 1] != '\\')
                    {
                        break;
                    }
                }
                if (len + 1 >= size)
                {
                    string_resize(&command, size, 2 * size);
                    size *= 2;
                }
                command[len] = (char)c_int;
                ++len;
            }
        }

        if (len + 1 >= size)
        {
            string_resize(&command, size, 2 * size);
            size *= 2;
        }
        command[len] = (char)c_int;
        ++len;
    }
    command[len] = 0;
    (*ptr_st) = st;
    return command;
}


char* next_arg(char** ptr_st)
{
    char* st = *ptr_st;
    int len, size;
    char* arg;

    arg = NULL;
    len = 0;
    size = 0;

    skip_spaces(&st);
    if (*st == 0) {
        (*ptr_st) = st;
        return NULL;
    }


    string_resize(&arg, size, DEFAULT_STRING_SIZE);
    size = DEFAULT_STRING_SIZE;
    if (*st == '<') {
        arg[0] = '<';
        len = 1;
        ++st;
    }
    else
    if (*st == '>')
    {
        ++st;
        arg[0] = '>';
        len = 1;
        if (*st == '>')
        {
            arg[1] = '>';
            len = 2;
            ++st;
        }
    }
    else
    if (*st == '"' || *st == '\'')
    {
        int quote = *st;
        ++st;
        while (1)
        {
            int c_int;
            c_int = (*st);
            ++st;
            if (c_int == quote)
            {
                if (len == 0 || arg[len - 1] != '\\')
                {
                    break;
                }
                --len;
            }
            if (len + 1 >= size)
            {
                string_resize(&arg, size, 2 * size);
                size *= 2;
            }
            arg[len] = (char)c_int;
            ++len;
        }
    }
    else
    {
        while (1)
        {
            int c_int;
            c_int = (*st);
            ++st;
            if (is_space(c_int))
            {
                break;
            }
            else
            if (c_int == 0)
            {
                --st;
                break;
            }
            else
            if (c_int == '\'' || c_int == '"' || c_int == '<' || c_int == '>')
            {
                --st;
                break;
            }
            else
            if (c_int == '\\' && len > 0 && arg[len - 1] == '\\')
            {
                --len;
            }
            if (len + 1 >= size)
            {
                string_resize(&arg, size, 2 * size);
                size *= 2;
            }
            arg[len] = c_int;
            ++len;
        }
    }
    arg[len] = 0;
    (*ptr_st) = st;
    return arg;
}