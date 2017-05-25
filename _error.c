#include "_error.h"


void print_my_error(int err)
{
    switch(err)
    {
        case E_OK:
            fprintf(stderr, "No error");
            break;
        case E_ENVIRONMET_INIT:
            fprintf(stderr, "ENVIRONMET INIT error");
            break;
        case E_SETPGID:
            fprintf(stderr, "SETPGID error");
            break;
        default:
            fprintf(stderr, "Unknown error");
            break;
    }
}