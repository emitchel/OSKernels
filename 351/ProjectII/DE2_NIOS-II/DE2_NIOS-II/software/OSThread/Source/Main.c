/*
 * Main.c
 *
 *  Created on: Oct 18, 2013
 *      Author: Zach
 */
#include <stdio.h>
#include <malloc.h>
#include "sys/alt_alarm.h"
#include "alt_types.h"
#include "../main.h"


int first = 1;
int y = 0;
void mythread(int thread_id){
	// The declaration of j as an integer was added on 10/24/2011
	int i, j, n;

	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++){
		printf("This is message %d of thread #%d.\n", i, thread_id);
		for (j = 0; j < MAX; j++);
	}
}

void newTCB(TCB* tcb, int i){
	int x = 0;
	asm("movia %0, mythread": "=r"(x));
	tcb->context = malloc(sizeof(int) * 6);
	tcb->stack = malloc(sizeof(int) * 4);
	tcb->context[0] = x;
	tcb->context[1] = 0;
	tcb->context[2] = 0;
	tcb->context[3] = i;
	tcb->context[4] = 0;
	tcb->context[5] = 0;
}

void mythread_join(int i){
	runnable_threads++;
	threads[i].status = READY;
}

void mythread_scheduler(param_list){
	currThread = &threads[0];
	int x;
	asm("movia %0, destroy": "=r"(x));
	if(runnable_threads && !first){
		asm("mov %0, r7": "=r"(currThread->context[0]));
		asm("mov %0, r2": "=r"(currThread->context[1]));
		asm("mov %0, r3": "=r"(currThread->context[2]));
		asm("mov %0, r4": "=r"(currThread->context[3]));
		asm("mov %0, r5": "=r"(currThread->context[4]));
		asm("mov %0, r6": "=r"(currThread->context[5]));
	}
	else if(runnable_threads && first){
		asm("mov %0, r17": "=r"(currThread->stack[0]));
		asm("mov %0, r2": "=r"(currThread->stack[1]));
		asm("mov %0, r3": "=r"(currThread->stack[2]));
		asm("mov %0, r4": "=r"(currThread->stack[3]));
		first =0;
	}

	int dc;
	if(runnable_threads){
		asm("mov r9, %0": "=r"(dc): "r"(currThread->context[0]));
		asm("mov r4, %0": "=r"(dc): "r"(currThread->context[1]));
		asm("mov r5, %0": "=r"(dc): "r"(currThread->context[2]));
		asm("mov r6, %0": "=r"(dc): "r"(currThread->context[3]));
		asm("mov r7, %0": "=r"(dc): "r"(currThread->context[4]));
		asm("mov r8, %0": "=r"(dc): "r"(currThread->context[5]));
		asm("mov r10, %0": "=r"(dc): "r"(x));
	}
	
	first = 0;
}

alt_u32 mythread_handler (void * param_list){
	if(runnable_threads)
		global_flag = 1;
	else
		global_flag = 0;
	printf("Exception\n");
	return ALARMTICKS(10); //Using the defined function above
}



void mythread_create(int i){
	TCB tcb;
	newTCB(&tcb, i);
	threads[i] = tcb;
}

void destroy(){
	printf("In destroy function\n");
	int dc,i;
	int stack[4];
	for(i=0;i<4;i++){
		stack[i] = currThread->stack[i];
	}
	currThread->status = DONE;
	asm("mov r2, %0": "=r"(dc): "r"(stack[1]));
	asm("mov r3, %0": "=r"(dc): "r"(stack[2]));
	asm("mov r4, %0": "=r"(dc): "r"(stack[3]));
	asm("mov r12, %0": "=r"(dc): "r"(stack[0]));
	asm("stw r12, 28(sp)");

	free(currThread->context);
	free(currThread->stack);
	free(currThread);
}

void prototype_os(param_list)
{
	int i;
	for(i=0; i<NUM_THREADS; i++){
		mythread_create(i);
	}

	for(i=0; i<NUM_THREADS; i++){
		mythread_join(i);
	}

	//Set the timer to be 1 second
	alt_alarm_start(&alarm, 10, mythread_handler, NULL);

	while (1)
	{
		printf ("This is the prototype os for my exciting CSE351 course projects!\n");
		int j=0;
		//MAX is the amount of wait time before the next printf above
		for (j = 0 ; j < MAX; j++);
	}
}

int main(){
	prototype_os();
	return 0;
}

