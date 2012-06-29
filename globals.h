#ifndef GLOBALS_H
#define GLOBALS_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <setjmp.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

//#include "iprocesses.h"
//#include "user_process.h"

/********************************************
	Constants
*********************************************/

/*Message Envelopes*/

#define MSG_ENV_SIZE 4096	
#define MSG_ENV_NUM 125		
#define MAX_CHAR 4096 	

/*Shared Memory*/

#define SHARED_MEM_SIZE 4096	

/*Processes*/

#define NUM_PROCESS 8 		
#define PRIORITY_NUM 4 		
#define STACK_SIZE 16384	
#define STK_OFFSET 4

/* PID */
#define PID_NULL 0 		
#define PID_I_CRT 1 			
#define PID_I_KB 2
#define PID_I_TIMER 3
#define PID_PROCESS_A 4
#define PID_PROCESS_B 5
#define PID_PROCESS_C 6
#define PID_CCI 7

/* Status */
#define EXECUTING 0				
#define READY 1					
#define BLOCKED_ON_RECEIVE 2		
#define BLOCKED_ON_RESOURCE 3	
#define IPROCESS 4	

/* Message Type Defines */
#define CONSOLE_OUTPUT 0	
#define CONSOLE_INPUT 1	
#define DISPLAY_ACK 2		// Used by CRT iprocess
#define REQUEST_DELAY 3
#define WAKEUP 4
#define DEFAULT 5
#define IPC 6
#define TERMINATE 7
#define CLOCK 8

/*Trace Buffers */
#define NUM_TRACE_BUFFER 16

/* I_Process */
#define CALLER_SIGNAL 0	
#define CALLER_PROCESS 1	

/* Shared Memory */
#define MEM_EMPTY 0
#define MEM_FULL 1

/* Error Codes */
#define BAD_INPUT_ARGS 2

/* Timer */
#define TIMER_INTERVAL 100000

/********************************************
	Structs 
*********************************************/
/*Message Envelope Struct*/
typedef struct msg_env {
	int type;
	char data[MSG_ENV_SIZE];
	int timeout_ticks;
	int sender_ID;
	int recipient_ID;
	struct msg_env *next;
	struct msg_env *prev;
} msg_env;

/*Message Queue Struct*/
typedef struct msg_q {
	struct msg_env *head;
	struct msg_env *tail;
} msg_q;

/*Process Control Block Struct*/
typedef struct PCB 	{
	char name[15]; 
	int pid;
	int status;
	int priority;
	struct msg_q msg_q; // PCB's Message Queue
	jmp_buf context;
	void (*process_code)(void); // Function Pointer to PCB's associated function/code
	char *stack;
	char *stack_head;
	struct PCB* next; // Used when connecting PCB's in a list
} PCB;

/*PCB Queue Struct*/
typedef struct pcb_q {
	struct PCB *head;
	struct PCB *tail;
} pcb_q;

/*Input Buffer Structure*/
typedef struct {
	int flag;
	char data[MAX_CHAR];
	int length;
} inputbuf;

/*Initialization Table Structure*/
typedef struct init_table {
	int pid;
	int priority;
	char name[MAX_CHAR];
	int status;
	struct PCB *PCB;
	void (*process_code)(void);
} init_table;

/*Trace Buffer Structure*/
typedef struct tbuffer {
	int sender_ID;
	int recipient_ID;
	int message_type;
	//char timestamp[8]; // Removed for Debugging Purposes
	int timestamp;
} tbuffer;

/********************************************
	Variables
*********************************************/

init_table IT[NUM_PROCESS]; // Initilization Table
msg_q free_env_q, timeout_q, crt_q; // Global Message Queue's Used by various processes
pcb_q rpq[PRIORITY_NUM], blocked_on_resource_q; // Global PCB Queue's Used by various processes
msg_env *terminate_envelope, *clock_envelope; // Two special envelopes

static char *kbfilename = "Keyboard Memory"; // KB Shared Memory File Name
static char *crtfilename = "CRT Memory"; // CRT Shared Memory File Name

static int bufsize = SHARED_MEM_SIZE; // Used in initializing the Shared Memory

inputbuf *in_mem_p, *out_mem_p; // Input/Output Memory Pointers
int in_pid, out_pid; // Linux pid of KB and CRT Process

caddr_t cmmap_ptr, kmmap_ptr; // Used in initializing the Shared Memory

int kfid, cfid, kstatus, cstatus; // Used in initializing the Shared Memory

int ticks, clock_state; // Used for timing
int clock_hr, clock_min, clock_sec; // Used for the Display Clock
int atomic_count; // Used for determining whether to turn atomic on/off

tbuffer tracebuffer[NUM_TRACE_BUFFER]; // Array of tbuffer elements for the TraceBuffer
int tracebuf_count; // Used in the TraceBuffer to make a circular array

PCB *current_process; // Pointer to Current Process
PCB *interrupted_process;// Pointer to Interupted Process when the handler is called

#endif
