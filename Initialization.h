#ifndef INIT_H
#define INIT_H

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "kernel.h"
#include "User_API.h"
#include "iprocesses.h"
#include "user_process.h"

void cleanup();

void die(int signal);

void init_signals();

void create_queues();

void read_init_table();

void init_processes();

void init_initial_values();

void k_init();

#endif
