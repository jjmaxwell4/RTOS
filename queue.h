#ifndef QUEUE_H
#define QUEUE_H

#include "globals.h"

PCB* pcb_dq(pcb_q *target);

PCB* rpq_dq();

int pcb_eq(pcb_q *target, PCB *new_pcb);

void rpq_eq(PCB *new_PCB);

msg_env* msg_dq(msg_q *target);

void msg_eq(msg_q *target, msg_env *env);

msg_env* PCB_msg_dequeue(PCB *PCB);

void PCB_msg_enqueue(PCB *target, msg_env *env);

msg_q* create_msg_q();

#endif
