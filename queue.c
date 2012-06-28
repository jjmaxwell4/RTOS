#include "queue.h"

/**** Dequeue *PCB from a PCB Queue ****/
PCB* pcb_dq(pcb_q *target)
{
	PCB *pcb = target->head;
	if(target->head != NULL)
	{
		if(target->head->next == NULL) { // Only one in queue
			target->head = NULL;
			target->tail = NULL;
		}
		else if(target->head->next->next == NULL) { // Two in queue
			target->head = target->head->next; 
			target->tail = target->head;
		}
		else {
			target->head = target->head->next;
		}
		pcb->next = NULL; // Can't point to anything else
		return pcb;
	}
	else
		return NULL;
}

/**** Dequeue a *PCB from the Ready Process Queue ****/
PCB* rpq_dq() {
	PCB *dispatching;
	int f = 0;
	
	while(rpq[f].head == NULL && f < NUM_PROCESS) {
		f++;	
	}
	dispatching = pcb_dq(&rpq[f]);
	return dispatching;
}

/**** Enqueue to a *PCB Queue ****/
int pcb_eq(pcb_q *target, PCB *new_pcb)
{
	if(new_pcb == NULL) {
		printf("Trying to pcb_eq a NULL PCB");
		return 0;
	}
	if(target->head == NULL) // Empty Queue
	{
		target->head = new_pcb;
		target->head->next = NULL;
		target->tail = new_pcb;
		target->tail->next = NULL;
		return 1;
	}
	else {
		target->tail->next = new_pcb;
		target->tail = target->tail->next;
		target->tail->next = NULL;
		return 1;
	}
}

/**** Enqueue to the Ready Process Queue ****/

void rpq_eq(PCB *new_PCB) {
	if(new_PCB != NULL){
		//printf("Setting the RPQ Priority: <%i>\n", new_PCB->priority);
		//printf("Setting the RPQ Name: <%s>\n", new_PCB->name);
		pcb_q *temp;
		temp = &rpq[new_PCB->priority];
		pcb_eq(temp, new_PCB);
	}
}

/*** Dequeue from a message Queue  
Note: Head and Tail Will be equal if only 1 message **/
msg_env* msg_dq(msg_q *target) {
	msg_env *env = target->head;
	
	if(env != NULL) {
		if(target->head->next == NULL) { // If there are only 1 messages
			target->head = NULL;
			target->tail = NULL;
		}
		
		else if(target->head->next->next == NULL) { // There are  2 messages
			target->head = target->head->next; 
			target->tail = target->head;
		}
		
		else {
			target->head = target->head->next;
		}
		env->next = NULL; // Can't point to anything else
		env->prev = NULL;
		return env;			
	}
	else {
		return env; // This means there is nothing to return. It return NULL
	}
}

/*** Enqueue from a message Queue  
Note: Head and Tail Will be equal if only 1 message **/
void msg_eq(msg_q *target, msg_env *env)
{
	if(target->head == NULL) // No messages in queue
	{
		target->head = env;
		env->next = NULL; // Can't point to anything else
		env->prev = NULL;
		target->tail = env; // Head == Tail in this case
	}
	else //More then one message in Queue
	{
		target->tail->next = env;
		env->next = NULL;
		env->prev = target->tail;
		target->tail = env;
		target->tail->next = NULL;
	}
}

/**** Dequeue a Message From a Specified *PCB ****/
msg_env* PCB_msg_dequeue(PCB *PCB)
{
	msg_env *env = PCB->msg_q.head;
	if(env != NULL)
	{
		if(PCB->msg_q.head->next == NULL) { // If there are only 1
			//printf("<%s> is dequeueing from its message queue. Only 1 message...\n", current_process);
			PCB->msg_q.head = NULL;
			PCB->msg_q.tail = NULL;
			}
		else if(PCB->msg_q.head->next->next == NULL) { // THere are only 2
			//printf("<%s> is dequeueing from its message queue. 2 messages...\n", current_process);
			PCB->msg_q.head = PCB->msg_q.head->next; 
			PCB->msg_q.tail = PCB->msg_q.head;
		}
		else {
			PCB->msg_q.head = PCB->msg_q.head->next;
		}
		env->next = NULL; // Can't point to anything else
		env->prev = NULL;
		return env;			
	}
	else
	{
		return env; // This means there is nothing to return. It return NULL
	}
}

/**** Enqueue a Message to a Specified *PCB ****/
void PCB_msg_enqueue(PCB *target, msg_env *env)
{
	if(target->msg_q.head == NULL) // No messages in queue
	{
		//printf("<%s> is adding to an empty PCB Queue...\n", current_process);
		target->msg_q.head = env;
		target->msg_q.head->next = NULL;
		target->msg_q.head->prev = NULL;
		target->msg_q.tail = env; // Head == Tail in this case
	}
	else //More then one message in Queue
	{
		//printf("<%s> is adding to an non-empty PCB Queue...\n", current_process);
		target->msg_q.tail->next = env;
		target->msg_q.tail = target->msg_q.tail->next;
		target->msg_q.tail->next = NULL;
	}
}

/**Creates an empty message Queue**/
msg_q* create_msg_q() {
	msg_q *new_q;
	new_q = (msg_q*) malloc(sizeof(msg_q));
	new_q->head = NULL;
	new_q->tail = NULL;
}
