#include "Initialization.h"


void cleanup()
{ 
	int i, j;
	msg_env *temp;
	PCB *tPCB;
	
	/*Free's PCB's and any waiting messages */
	for(i=0; i<NUM_PROCESS; i++) {
		tPCB = PIDtoPCB(i);
		if (tPCB != NULL) {
			while(tPCB->msg_q.head != NULL) {
				temp = tPCB->msg_q.head;
				if(temp != NULL) {
					tPCB->msg_q.head = tPCB->msg_q.head->next;
					free(temp);
				}
			}
			free(tPCB);
		}
	}
	/*Free's other message queue;s*/
	

	temp = free_env_q.head;
	while(temp != NULL) {
		free_env_q.head = free_env_q.head->next;
		free(temp);
		temp = free_env_q.head;
	}
	

	temp  = crt_q.head;
	while(temp != NULL) {
		crt_q.head = crt_q.head->next;
		free(temp);
		temp  = crt_q.head;
	}
	
	temp  = timeout_q.head;
	while(temp != NULL) {
		timeout_q.head = timeout_q.head->next;
		free(temp);
		temp  = timeout_q.head;
	}

	/*Kills the KB and CRT Unix Process*/
	kill(in_pid, SIGINT);
	kill(out_pid, SIGINT);
	
	/*Cleans up the Shared Memory for both KB and CRT*/
	kstatus = munmap(kmmap_ptr, bufsize);
	if(kstatus == -1) {
		//printf("Bad kmunmap during cleanup\n");
	}
	
	kstatus = close(kfid);
	if(kstatus == -1) {
		//printf("Bad close of temporary kmmap file during cleanup\n");
	}
	
	kstatus = unlink(kbfilename);
	if(kstatus == -1) {
		//printf("Bad unlink during kb cleanup\n");
	}

	cstatus = munmap(cmmap_ptr, bufsize);
	if( cstatus == -1) {
		//printf("Bad munmap during crt cleanup\n");
	}
	
	cstatus = close(cfid);
	if(cstatus == -1) {
		//printf("Bad close of temporary cmmap file during cleanup\n");
	}
	
	cstatus = unlink(crtfilename);
	if(cstatus == -1) {
		//printf("Bad unlink during crt cleanup\n");
	}
	printf("\n\nCLOSING RTX...\n"); // This prints when the RTX is shutting down
}

void die(int signal)
{
	cleanup();
	exit(0);
}

void init_signals()  // Initializes all signals
{
	sigset(SIGQUIT, die);
	sigset(SIGBUS, die);
	sigset(SIGHUP, die);
	sigset(SIGILL, die);
	sigset(SIGABRT, die);
	sigset(SIGTERM, die);
	sigset(SIGSEGV, die);
	sigset(SIGINT, die);

	sigset(SIGUSR1, k_interrupt_handler);
	sigset(SIGUSR2, k_interrupt_handler);
	sigset(SIGALRM, k_interrupt_handler);
}

void create_queues() // Creates requires queue's/envelopes for RTX
{
	int i;
	/*****TERMINATE ENVELOPE ***/
	terminate_envelope = (msg_env*) malloc(sizeof(msg_env));
	terminate_envelope->type = TERMINATE;
	terminate_envelope->next = NULL;
	terminate_envelope->prev = NULL;
	
	/*****CLOCK ENVELOPE ***/
	clock_envelope = (msg_env*) malloc(sizeof(msg_env));
	clock_envelope->type = CLOCK;
	clock_envelope->next = NULL;
	clock_envelope->prev = NULL;
	
	/****MAKE EMPTY MESSAGE QUEUE*****/
	msg_env *first = (msg_env*) malloc(sizeof(msg_env));
	first->type = DEFAULT;
	msg_env *second = NULL;
	free_env_q.head = first;
	first->next = NULL;
	first->prev = NULL;
	
	for(i=0; i<MSG_ENV_NUM; i++){
		second = (msg_env*) malloc(sizeof(msg_env));
		second->type = DEFAULT;
		first->next = second;
		second->prev = first;
		first = first->next;
	}
	second->next = NULL;
	free_env_q.tail = second;
	
	/******MAKE TIMEOUT MESSAGE QUEUE ******/
	timeout_q.head = NULL;
	timeout_q.tail = NULL;
	
	/******MAKE CRT MESSAGE QUEUE ******/	
	crt_q.head  = NULL;
	crt_q.tail = NULL;
	
	/******MAKE RPQ PCB QUEUE ******/
	for(i = 0; i<PRIORITY_NUM; i++) {
		rpq[i].head = NULL;
		rpq[i].tail = NULL;
	}
	
	/******MAKE blocked_on_resource_q PCB QUEUE ******/
	blocked_on_resource_q.head = NULL;
	blocked_on_resource_q.tail = NULL;
}

void read_init_table(){
	int pid, priority, i, status;
	/*Init Process Function Pointers */
	void (*n)(void); 
	n = &null_process; 
	void (*A)(void); 
	A = &process_A; 
	void (*B)(void); 
	B = &process_B; 
	void (*C)(void); 
	C = &process_C; 
	void (*CCI)(void); 
	CCI = &cci_process; 
	void (*NUL)(void) = NULL;
	void (*process_code[]) = {n, NUL ,NUL ,NUL , A, B, C, CCI};

	char name[15];
	FILE *file = fopen("init_table", "r");
	
	/*Read From Initilization Table*/
	for(i = 0; i<NUM_PROCESS; i++)
	{
		fscanf(file, "%d %d %d %s\n", &pid, &priority, &status, name);
		IT[i].pid = pid;
		IT[i].priority = priority;
		IT[i].status = status;
		strcpy(IT[i].name, name);
		IT[i].PCB = PIDtoPCB(i);
		IT[i].process_code = process_code[i];
	}
	fclose(file);
}

void init_processes()
{
	int i;
	jmp_buf kernel_buf;
	char *jmpsp = NULL;
	for(i = 0; i< NUM_PROCESS; i++) {
		PCB *apcb = (PCB*) malloc(sizeof(PCB));
		apcb->priority = IT[i].priority;
		apcb->pid = IT[i].pid;
		apcb->stack = (((char *) malloc(STACK_SIZE)) + STACK_SIZE - STK_OFFSET);
		apcb->stack_head = apcb->stack - STACK_SIZE + STK_OFFSET;
		apcb->status = IT[i].status;
		apcb->process_code = IT[i].process_code;
		if(apcb->pid == PID_I_KB) { // THE KB_I_PROCESS STARTS EXECUTION WITH AN ENVELOPE ON ITS MSG_QUEUE
			msg_env *KB = (msg_env*) malloc(sizeof(msg_env));
			KB->type = DEFAULT;
			KB->next = NULL;
			KB->prev = NULL;
			apcb->msg_q.head = KB;
			apcb->msg_q.tail = KB;
		}
		else { // ALL OTHER PROCESS DO NOT START WITH NO ENVELOPES
			apcb->msg_q.head = NULL;
			apcb->msg_q.tail = NULL;
		}
		strcpy(apcb->name, IT[i].name);
		IT[i].PCB = apcb;
		if(apcb->status != IPROCESS) {
			rpq_eq(apcb);
			PCB *new_PCB = apcb;
			if(setjmp(kernel_buf) == 0)
			{
				jmpsp = new_PCB->stack;
#ifdef i386
				__asm__ ("movl %0,%%esp" :"=m" (jmpsp)); // if Linux i386 target
#endif 

#ifdef __amd64
				__asm__ ("movl %0,%%esp" :"=m" (jmpsp)); // if Linux amd64 target
#endif 
			
#ifdef __sparc
				_set_sp( jmpsp ); // if Sparc target (eceunix)
#endif
				if (setjmp (new_PCB->context) == 0) {
					//printf("IF: <%s>...\n", new_PCB->name);
					longjmp (kernel_buf, 1);
				}
				else {
					void (*temp)(void); 
					temp = current_process->process_code;
					atomic(0); // Make sure that atomic_count is reset for next process
					temp(); // process starts for the first time here
				}
			}
		}
	}
}
/***Set Inital Values***/
void init_initial_values() {
	ticks = 0;
	clock_state = 0;
	clock_hr =0;
	clock_min=0;
	clock_sec=0;
	tracebuf_count=0;
}

/**
 ***************************************************************
 * 
 **************************************************************/
 
void k_init()
{
	atomic(1); // Turns off all signal immediatly
	init_initial_values(); // Initializes Initial Global Values. Not sure I need this
	init_signals(); // Initializes the signals for handling
	create_queues(); // Creates queue of messages
	
	read_init_table(); // Read In Initilization Table
	init_processes(); // Initialize PCB with init_table data
	atomic(0);
}
