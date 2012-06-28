#include "kernel.h"

/*KERNEL PRIMITIVES*/

PCB* PIDtoPCB(int PID){ // Converts a PID to a pointer to that PCB
        PCB *tPCB;
        if (PID > NUM_PROCESS || PID < 0){
		    tPCB = NULL;
			return tPCB;
        }
        tPCB = IT[PID].PCB;
        return tPCB;
}

void K_context_switch(jmp_buf prev, jmp_buf next) { // Switches Context
  if (setjmp(prev) == 0) {
    longjmp(next, 1);
  } else {
    return;
  }
}

void K_process_switch() { // Prepares PCB for Process Swtich
  PCB* next = rpq_dq();
  PCB* tmp = current_process;
  next->status = EXECUTING;
  current_process = next; 
  K_context_switch(tmp->context, next->context);
}


void K_release_processor(){ //Put current process on RPQ, and then Releases the Processor
	PCB *current = current_process;
	current->status = READY;
	rpq_eq(current);
	K_process_switch();
	return;
}

void K_dispatch() { //Dispatches the first process to Run
	jmp_buf temp;
	PCB *first_process = rpq_dq();
	current_process = first_process;
	K_context_switch(temp, current_process->context);
}

//Sends Message
int K_message_send(int target_PID, msg_env* env){
	if(env != NULL) {
		env->sender_ID = current_process->pid;
		env->recipient_ID = target_PID;
		PCB *recipient_PCB = PIDtoPCB(target_PID);
		PCB_msg_enqueue(recipient_PCB, env);

		if(recipient_PCB->status == BLOCKED_ON_RECEIVE){ // Unblock the PCB
			recipient_PCB->status = READY;
			rpq_eq(recipient_PCB);
		}
		//Add data to Trace Buffer
		tracebuffer[tracebuf_count].sender_ID = env->sender_ID;
		tracebuffer[tracebuf_count].recipient_ID = env->recipient_ID;
		tracebuffer[tracebuf_count].message_type = env->type;
		tracebuffer[tracebuf_count].timestamp = ticks; // We use ticks in the trace buffer
		//TraceBuffer Hack. Creates a Circular Array
		if(tracebuf_count == 16) 
			tracebuf_count = 0;
		else 
			tracebuf_count++;
		return 1;
	}
}

//Receive Message
msg_env* K_message_receive(){
	msg_env *env;
	env = NULL;
	while(current_process->msg_q.head == NULL) { // No Message Waiting. Block
		if(current_process->status == IPROCESS){ // Don't Block an IPROCESS
			return env;
		}
		current_process->status = BLOCKED_ON_RECEIVE;
		K_process_switch();		
	}
	env = (msg_env*) PCB_msg_dequeue(current_process); // Get the Waiting Message
	env->next = NULL;
	env->prev = NULL;
	/*Add Some Stuff to Trace Buffer*/
	tracebuffer[tracebuf_count].sender_ID = env->sender_ID;
	tracebuffer[tracebuf_count].recipient_ID = env->recipient_ID;
	tracebuffer[tracebuf_count].message_type = env->type;
	tracebuffer[tracebuf_count].timestamp = ticks;
	//TraceBuffer Hack. Creates a Circular Array
	if(tracebuf_count == 16) 
		tracebuf_count = 0;
	else {
		tracebuf_count++;
	}
	//End TraceBuffer Hack
	return env;
}

//Get Console Chars
int K_get_console_chars(msg_env* env){
	int i = -1;
	env->type = CONSOLE_INPUT;
	//sender PID will be written to the env by the message_send primative
	i = K_message_send(PID_I_KB,env);
	return i;
}


//Send Console Char
int K_send_console_chars(msg_env* env){
	int i = -1;
	if(env->type != CLOCK) // Never change the env->type of the Clock Envelope
		env->type = CONSOLE_OUTPUT; //Console Output
	//sender PID will be written to the env by the message_send primative
	i = K_message_send(PID_I_CRT,env);
	return i;
}

//Request Envelope (Envelope Allocation)
msg_env* K_request_msg_env(){
	msg_env* env = NULL;
	while(free_env_q.head == NULL){  // If there are no envelopes, BLOCK
		if(current_process->status == IPROCESS) // DOn't Block an IPROCESS
			return env;
		pcb_eq(&blocked_on_resource_q, current_process);
		current_process->status = BLOCKED_ON_RESOURCE;
		K_process_switch();
	}
	env = msg_dq(&free_env_q);
	return env;
}

//Release Envelope (Deallocate)
int K_release_msg_env(msg_env* env){
	if(env != NULL) {
		strcpy(env->data, "\n"); // Clear the data
		env->next = NULL;
		env->prev = NULL;
		if (in_mem_p->flag == MEM_FULL && current_process->pid != PID_I_TIMER ) { // If the Memory is Full, 
														//the kb_i_process probably needs envelopes
			env->type = DEFAULT;
			K_message_send(PID_I_KB, env);
			
			/*WE HAVE TO MANUALLY CALL kb_i_process() THIS HACK ALLOWS US TO ENSURE THAT
				kb_i_process WILL ALWAYS HAVE MESSAGE ENVELOPES***/
			interrupted_process = current_process;
			current_process = (PCB*) PIDtoPCB(PID_I_KB);
			kb_i_process();
			current_process = interrupted_process;
			interrupted_process = NULL;
			
			return;
		}		
		env->type = DEFAULT;
		msg_eq(&free_env_q, env);
		if(blocked_on_resource_q.head != NULL){ // Put the first one back on the Ready Queue
			PCB* temp;
			temp = pcb_dq(&blocked_on_resource_q);
			temp->status = READY;
			rpq_eq(temp); 
		}
	return 1;
	}
}

/*** Get Trace Buffers 
We have an array of tracebuffer structs. We 
concatenate all values and return this value. ****/

int K_get_trace_buffers(msg_env* env){
	int i;
	int tb = (tracebuf_count - 1);
	char print_buf[MAX_CHAR];
	char tempbuf[MAX_CHAR];
	sprintf(print_buf, "Sender   Recipient    Type      Ticks\n"); // Format Headers
	for(i =tb; i >= 0; --i) { // Counts Down the circular array
		tempbuf[0] = '\n'; // Clears the string
		sprintf(tempbuf, "       %i           %i       %i        %i\n", 
			tracebuffer[i].sender_ID, 
			tracebuffer[i].recipient_ID, 
			tracebuffer[i].message_type,
			tracebuffer[i].timestamp);
		strcat(print_buf, tempbuf);
	}
	int j;
	for(j = 16; j > tracebuf_count; j--) { // Counts Down the circular array
		tempbuf[0] = '\n'; // Clears the string
		sprintf(tempbuf, "       %i           %i       %i        %i\n", 
			tracebuffer[j-1].sender_ID, 
			tracebuffer[j-1].recipient_ID, 
			tracebuffer[j-1].message_type,
			tracebuffer[j-1].timestamp);
		strcat(print_buf, tempbuf);
	}
	sprintf(env->data, print_buf);
	return 1;
}


//Request Process Status
int K_request_process_status(msg_env* env){
	char a[MAX_CHAR];
	char temp[MAX_CHAR];
	sprintf(a, "  PCB_ID     STATUS     PRIORITY\n"); // Format Headers
	int i;
	for (i = 0; i<NUM_PROCESS; i++){
		temp[0] = '\n'; // Clears the string
		PCB *tPCB = PIDtoPCB(i);
		sprintf(temp, "      %i         %i          %i \n", 
			tPCB->pid, tPCB->status, tPCB->priority);
		strcat(a, temp);
	}
	sprintf(env->data, a);
	return 1;
}

//TERMINATE
int K_terminate(){ 
	cleanup();
	exit(0);
}

//Change priority
int K_change_priority(int new_priority, int PID){
	PCB *target_PCB = PIDtoPCB(PID);
	PCB *temp, *temp2;
	temp2 = temp;
	int old_priority;
	old_priority = target_PCB->priority;
	if(old_priority == new_priority) {
		return BAD_INPUT_ARGS; // Error code
	}
	if(target_PCB->pid != PID_NULL) {
		switch(target_PCB->status) {
			case READY: 
					target_PCB->priority = new_priority;
					temp = rpq[old_priority].head; //Remove PCB from current ready queue
					if (temp->pid == target_PCB->pid){ //the first item in the queue
						temp = pcb_dq(&rpq[old_priority]); //Can use dequeue function		
					}
					else{ //Not first item in queue
						while(temp->pid != target_PCB->pid){
							temp2 = temp;
							temp = temp->next; //search for correct PCB
						}
						if (temp->next == NULL){ //last item in queue
							temp2->next = NULL;
							rpq[old_priority].tail = temp2;
						}
						else{ //Somewhere in the middle of the queue
							temp2->next = temp->next; //remove temp from queue
							temp->next = NULL;
						}
					}
					rpq_eq(temp);	//enqueue PCB onto new ready queue
					break;
			case IPROCESS: 
					return 0;//i-process priority remains unaltered
			case BLOCKED_ON_RESOURCE:
					target_PCB->priority = new_priority;
					break; 
			case EXECUTING:
					target_PCB->priority = new_priority;
					break; 
			case BLOCKED_ON_RECEIVE:
					target_PCB->priority = new_priority;
					break; 
		}
		return 1;
	}
	return 0;
}
// Requesting Delay Primitive
int K_request_delay(int delay, int WAKE_CODE, msg_env* env){
	int error_code;
	env->timeout_ticks = delay;
	env->type = WAKE_CODE;
	error_code = K_message_send(PID_I_TIMER, env); //get PID of timing i-process
	return error_code;
}

// Changing Time Primitive
void K_change_time(msg_env *env) {
	char time[3];
	int temp1, temp2, temp3, fail = 0;
	
	time[0] = env->data[2];
	time[1] = env->data[3];
	time[2] = '\n';
	temp1 = atoi(time);
	
	time[0] = env->data[5];
	time[1] = env->data[6];
	time[2] = '\n';
	temp2 = atoi(time);
	
	time[0] = env->data[8];
	time[1] = env->data[9];
	time[2] = '\n';
	temp3 = atoi(time);
	
	if(temp1 > 23 || temp1 < 0 || temp2 > 59 || temp2 < 0 || temp3 > 59 || temp3 < 0) {
		strcpy(env->data, "ERROR: INVALID TIME ENTERED \n");
		env->type = CONSOLE_OUTPUT;
		send_console_chars(env);
	}
	else {
		clock_hr = temp1;
		clock_min = temp2;
		clock_sec = temp3;
	}
	
}
