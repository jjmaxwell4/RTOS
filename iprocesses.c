#include "iprocesses.h"

void kb_i_process() {	
    msg_env *env;
	
	if(in_mem_p->data[0] == 't' && in_mem_p->data[1] == '\n') { // So there is always a terminate envelope
		printf("TERMINATING....\n");
		env = terminate_envelope;
		K_message_send(PID_CCI, env); 
		return;
	}
	env = K_request_msg_env(); 
	if(env == NULL){
		env = K_message_receive();
	}
	
	
	if(env != NULL) {
		if(in_mem_p->flag == MEM_FULL) {
			strncpy(env->data, in_mem_p->data, sizeof(env->data)); // Copy From Memory to the Envelope
			in_mem_p->flag = MEM_EMPTY; // Reset the Flag 
			env->type = CONSOLE_INPUT; 
			K_message_send(PID_CCI, env); // Send Envelope to the CCI
		}
	} 
}

void crt_i_process() {  
    msg_env *env;
	if(crt_q.head == NULL) {
		env = K_message_receive(); 
	}
	else { // Dequeue from the CRT_q if there are messages there
		env = msg_dq(&crt_q);
	} 

	while(env != NULL) {
		if(env->type == CONSOLE_OUTPUT || env->type == CLOCK) {
			if(out_mem_p->flag != MEM_EMPTY) { // If The data in Memory has not yet been outputed
				msg_eq(&crt_q, env); // Enqueue the data on the local queue
				return;
			}
			else {
				strcpy(out_mem_p->data, env->data); // Copy the envelope data to the Memory
				out_mem_p->flag = MEM_FULL; // Set the Flag to FULL
				if(env->type != CLOCK)  // Don't change env->type of the CLOCK Envelope
					env->type = DISPLAY_ACK;
				K_message_send(env->sender_ID, env); // Send the Envelope back to the sending process
			}
			
			if(crt_q.head == NULL) { // Check for another envelope
				env = K_message_receive(); 
			}
			else {
				env = msg_dq(&crt_q);
			} 
		}
		else {
			K_release_msg_env(env);
		}
	}
	
}
void timeout_enqueue(msg_env *env) { // Enqueue's a envelope onto the Timeout Queue back on timeout_ticks value
	msg_env *temp;
	if(timeout_q.head == NULL) {
		timeout_q.head = env;
		timeout_q.tail = env;
	}
	else {
		msg_env *temp_n = timeout_q.head;
		msg_env *temp_p = NULL;
		while(env->timeout_ticks > temp_n->timeout_ticks) {
			temp_p = temp_n;
			temp_n = temp_n->next;
		}
		temp_p->next = temp_n;
		temp_n->prev = temp_p;
		temp_n->next->prev = temp_n;
	}
}
		
	
void timing_i_process() { // Keeps Time
	msg_env *env;
	env = K_message_receive();
	while(env) {
		if(env->type == REQUEST_DELAY) {
			timeout_enqueue(env);
		}
		else if(env->type != CLOCK){
			K_release_msg_env(env);
		}
		env = K_message_receive();
	} 
	ticks++;
	if(timeout_q.head != NULL) {
		timeout_q.head->timeout_ticks--; 
		if(timeout_q.head->timeout_ticks <= 0) {
			env = msg_dq(&timeout_q);
			env->type = WAKEUP;
			K_message_send(env->sender_ID, env);
		}
	}
	
	if(ticks %10 == 0) { // If the time is an 'even' second, update the clock
		clock_sec++;
		update_clock();
	}
}

void update_clock() { // Updates the clock (not the ticks..)
	msg_env *env;
	if(clock_state)
		env = clock_envelope; 
	if(clock_sec >=60) 
		if(++clock_min >=60)
			++clock_hr;
	clock_sec %= 60;
	clock_min %= 60;
	clock_hr %= 24;
	if(clock_state) {
		sprintf(env->data, "%02d:%02d:%02d", clock_hr, clock_min, clock_sec);
		K_send_console_chars(env);
	}
}

void k_interrupt_handler(int signal) // Handles Unix Signals and passes to the appropriate i_process
{
  atomic(1);
  interrupted_process = current_process;
  
  switch(signal) {
    case SIGUSR1: current_process = (PCB*) PIDtoPCB(PID_I_KB);
                  kb_i_process();
                  break;
    case SIGUSR2: current_process = (PCB*) PIDtoPCB(PID_I_CRT);
                  crt_i_process();
                  break;
	case SIGALRM: current_process = (PCB*) PIDtoPCB(PID_I_TIMER);
				  timing_i_process();
				  break;
  }
  current_process = interrupted_process;
  interrupted_process = NULL;
  
  atomic(0);
}
