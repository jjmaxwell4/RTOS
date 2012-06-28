#ifndef KERNEL_H
#define KERNEL_H

#include <sys/mman.h>
#include <sys/wait.h>
#include "queue.h"
#include "iprocesses.h"
#include "User_API.h"
#include "globals.h"

PCB* PIDtoPCB(int PID);

void K_context_switch(jmp_buf prev, jmp_buf next);

void K_process_switch();

void K_release_processor();

void K_dispatch();

int K_message_send(int target_PID, msg_env *env);

msg_env* K_message_receive();

int K_send_console_chars(msg_env *env);

int K_get_console_chars(msg_env *env);

msg_env* K_request_msg_env();

int K_release_msg_env(msg_env *env);

int K_get_trace_buffers(msg_env* env);

int K_request_process_status(msg_env* env);

int K_terminate();

int K_change_priority(int new_priority, int PID);

int K_request_delay(int delay, int WAKE_CODE, msg_env* env);

void K_change_time(msg_env *env);

#endif
