#ifndef IPROCESSES_H
#define IPROCESSES_H

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "kernel.h"
#include "User_API.h"
#include <signal.h>

void kb_i_process();

void crt_i_process();

void timeout_enqueue(msg_env *env);

void timing_i_process();

void update_clock();

void k_interrupt_handler(int signal);

#endif
