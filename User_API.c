#include "User_API.h"

void dispatch() {
	atomic(1);
	K_dispatch();
	atomic(0);
}

int message_send(int target_PID,msg_env* env){
	int return_val;
	atomic(1);
	return_val = K_message_send(target_PID, env);
	atomic(0);
	return return_val;
}


msg_env* message_receive(){
	msg_env *env = NULL;
	atomic(1);
	//printf("<%s>: message_receive()...\n", current_process->name);
	env = K_message_receive();
	atomic(0);
	return env;
}


int get_console_chars(msg_env* env){
	if (env == NULL) {
		printf("Trying to send NULL envelope to k_get_console_chars");
		return -1;
	}
	else {
		int return_val;
		atomic(1);
		return_val = K_get_console_chars(env);
		atomic(0);
		return return_val;
	}
}


int send_console_chars(msg_env* env){
	if (env == NULL) {
		printf("Trying to send NULL envelope to k_get_console_chars");
		return -1;
	}
	else {
		int return_val;
		atomic(1);
		return_val = K_send_console_chars(env);
		atomic(0);
		return return_val;
	}
}

msg_env* request_msg_env(){
	msg_env* env = NULL;
	atomic(1);
	env = K_request_msg_env();
	atomic(0);
	return env;
}

int release_msg_env(msg_env* env){
	int return_val;
	atomic(1);
	return_val = K_release_msg_env(env);
	atomic(0);
	return return_val;
}

int get_trace_buffers(msg_env* env){
	int return_val;
	atomic(1);
	return_val = K_get_trace_buffers(env);
	atomic(0);
	return return_val;
}

void release_processor(){
	atomic(1);
	K_release_processor();
	atomic(0);
	return;
}

int request_process_status(msg_env* env){
	int return_val;
	atomic(1);
	return_val = K_request_process_status(env);
	atomic(0);
	return return_val;
}

int terminate(){
	int return_val;
	atomic(1);
	return_val = K_terminate();
	atomic(0);
	return return_val;
}

int change_priority(int new_priority, int PID){
	int return_val;
	atomic(1);
	return_val = K_change_priority(new_priority, PID);
	atomic(0);
	return return_val;
}

int request_delay(int delay, int WAKE_CODE, msg_env* env){
	int return_val;
	atomic(1);
	return_val = K_request_delay(delay,WAKE_CODE,env);
	atomic(0);
	return return_val;
}

void atomic(int on) {
	static sigset_t oldmask;
	sigset_t newmask;
	
	if (atomic_count < 0) {
		//printf("Error: Atomic_count is < 0.... Quiting RTX");
		die(0);
	}
	if (on == 1) {
		atomic_count++;
	}
	if(on == 0) {
		atomic_count--;
	}
	if (atomic_count == 1 && on == 1 ){ // Only do this when first turning atomic on
		//unblock the signals
		sigemptyset(&newmask);
		sigaddset(&newmask, SIGALRM); //the alarm signal
		sigaddset(&newmask, SIGINT); // the CNTRL-C
		sigaddset(&newmask, SIGUSR1); // the CRT signal
		sigaddset(&newmask, SIGUSR2); // the KB signal
		sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		//printf("Atomic ON: Atomic_count <%i>...\n", atomic_count);
	}
	if(atomic_count == 0 && on == 0) {
		sigprocmask(SIG_SETMASK, &oldmask, NULL);
		//printf("Atomic OFF: Atomic_count <%i>...\n", atomic_count);
	}
}

void change_time(msg_env *env) {
	atomic(1);
	K_change_time(env);
	atomic(0);
}
