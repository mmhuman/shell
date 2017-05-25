#define _XOPEN_SOURCE 1000

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "processes.h"
#include "_error.h"
#include "environment.h"
#include "read.h"

#define DEFAULT_ARRAY_SIZE     32
