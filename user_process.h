#ifndef USER_P
#define USER_P

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "kernel.h"
#include "User_API.h"
#include "iprocesses.h"

void cci_process(void);

void null_process(void);

void process_A(void);

void process_B(void);

void process_C(void);

#endif
