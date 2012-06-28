#include "user_process.h"

void cci_process(void) {
	while(1) {
	msg_env *input_env;
	input_env = message_receive(); // Blocks until message received
	if (input_env->type == DISPLAY_ACK) {
		release_msg_env(input_env);
	}
	else if(input_env->type == TERMINATE) 
	{
		terminate();
	
	}
	else if(input_env->type != CONSOLE_INPUT) {
		terminate();
	}
	else 
	{
		char a, b, c;
		int priority, PID, r;
		char time[2];
		a = input_env->data[0];	
		switch(a) 
		{
			// Send Message to Proces_A
			case 's':
				b = input_env->data[1];
				if(b == '\n') {
					input_env->type = IPC;
					message_send(PID_PROCESS_A, input_env);
					break;
				}
				else {
					strcpy(input_env->data, "ERROR: Improper input was not identified by CCI \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}	
				
			// Display Status of All Processes
			case 'p':
				b = input_env->data[1];
				c = input_env->data[2];
				if(b != 's' || c != '\n') {
					strcpy(input_env->data, "ERROR: Improper input was not identified by CCI \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				request_process_status(input_env); // This will get all the process info
				input_env->type = CONSOLE_OUTPUT;
				send_console_chars(input_env);
				break;
				
			// Display the Trace Buffers
			case 'b':
				b = input_env->data[1];
				if(b != '\n') {
					strcpy(input_env->data, "ERROR: Improper input was not identified by CCI \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				get_trace_buffers(input_env);
				input_env->type = CONSOLE_OUTPUT;
				send_console_chars(input_env);
				break;
				
			// Change the Priority of Process
			case 'n': 
				b = input_env->data[5];
				if(b != '\n') {
					strcpy(input_env->data, "ERROR: Improper input was not identified by CCI \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				b = '\n'; // Clear b
				b = input_env->data[2];
				sscanf(&b, "%1d", &priority); // Convert to a number
				
				c = '\n';  // Clear c
				c = input_env->data[4];
				sscanf(&c, "%1d", &PID); // Convert to a number
				
				if(priority < 0 || priority >3) {
					strcpy(input_env->data, "ERROR: INVALID NEW PRIORITY\n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				else if(PID > 8 || PID <0) {
					strcpy(input_env->data, "ERROR: INVALID PID \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				
				r = change_priority(priority, PID);
				if(r == 0) {
					strcpy(input_env->data, "ERROR: CANNOT CHANGE PRIORITY OF THIS PROCESS \n");
					input_env->type = CONSOLE_OUTPUT;
					send_console_chars(input_env);
					break;
				}
				break;
				
			// Time Cases
			case 'c':
				b = input_env->data[1];
				// Turn Clock on
				if(b == 'd') {
					clock_state = 1;
					break;
				}
				//Turn Clock Off
				else if(b == 't') {
					clock_state = 0;
					break;
				}
				c = input_env->data[4];
				//Chang the Time
				if(c == ':')
				{
					//printf("Case 'c': SETTING TIME....\n");
					change_time(input_env); 
					break;
				}
				
			default:
				strcpy(input_env->data, "ERROR: Improper input was not identified by CCI \n");
				input_env->type = CONSOLE_OUTPUT;
				send_console_chars(input_env);
				break;
		}
		release_processor();		
	}
	}
}

void null_process(void) { // NULL_PROCESS just releases the processor
	while(1) {
		release_processor();
	}
}

void process_A(void) {
	msg_env *env = NULL;
	env = message_receive(); // blocks until message is received
	release_msg_env(env);
	int num = 0;
	do {
		env = request_msg_env();
		env->type = IPC;
		sprintf(env->data, "%d", num);
		message_send(PID_PROCESS_B, env);
		num++;
		release_processor();
	} while(1);
}

void process_B(void) {
	msg_env *env = NULL;
	do {
		env = message_receive();
		message_send(PID_PROCESS_C, env);
	} while(1);
}

void process_C(void) {
	msg_q *local_q;
	local_q = create_msg_q();
	
	msg_env *env = NULL;
	do {
		if(local_q->head == NULL) {
			env = message_receive(); 
		}
		else {
			env = msg_dq(local_q);
		}
		if(env->type == IPC) {
			if((atoi(env->data)%20) == 0 && atoi(env->data) != 0) {
				strncpy(env->data,"PROCESS_C", MAX_CHAR);
				env->type = CONSOLE_OUTPUT;
				send_console_chars(env);		
				
				env = message_receive();
				
				while(env->type != DISPLAY_ACK) {
					if((atoi(env->data)%20) ==0)
						msg_eq(local_q, env);
					else 
						release_msg_env(env);
					env = message_receive();
				}
				
				request_delay(100, REQUEST_DELAY, env);
				
				env = message_receive();
				
				while (env->type != WAKEUP) {
					if((atoi(env->data)%20) == 0 || env->type != IPC)
						msg_eq(local_q, env);
					else 
						release_msg_env(env);
					env = message_receive();
				}
			}
			
		}
		release_msg_env(env);
		release_processor();		
	} while(1);
}
