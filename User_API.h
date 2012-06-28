#ifndef USER_API_H
#define USER_API_H

#include "globals.h"
#include "kernel.h"
#include <stdlib.h>

void dispatch();

int message_send(int target_PID,msg_env* env);

msg_env* message_receive();

int send_console_chars(msg_env* env);

int get_console_chars(msg_env* env);

msg_env* request_msg_env();

int release_msg_env(msg_env* env);

int get_trace_buffers(msg_env* env);

void release_processor();

int request_process_status(msg_env* env);

int terminate();

int change_priority(int new_priority, int PID);

int request_delay(int delay, int WAKE_CODE, msg_env* env);

void atomic(int on);

void change_time(msg_env *env);

#endif
