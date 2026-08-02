/*! \file

Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
to determine which process may continue its execution next.

The file contains five strategies:
-even
-random
-round-robin
-inactive-aging
-run-to-completion
*/

#include "os_scheduling_strategies.h"

#include "defines.h"

#include <stdlib.h>

SchedulingInformation schedulingInfo;



ProcessQueue* MLFQ_getQueue(uint8_t index){
	return &schedulingInfo.pqueues[index];
}

void MLFQ_removeProcess(ProcessID pid){
	for (uint8_t qid = 0; qid<4; qid++){
		ProcessQueue* pq = MLFQ_getQueue(qid);
		pqueue_removePID(pq, pid);
	}
}

uint8_t MLFQ_successorQueue(uint8_t queue_id){
	if (queue_id == 3) {return 3;}
	return (queue_id + 1);
}

uint8_t MLFQ_decodePriority(Priority prio){
	return 3-(prio>>6);
}

uint8_t MLFQ_getDefaultTimeSlice(uint8_t queue_id){
	return (1<<queue_id);
}

void MLFQ_initProcessTimeSlice(ProcessID pid){
	uint8_t pq_level = MLFQ_decodePriority(os_getProcessSlot(pid)->priority);
	schedulingInfo.MLFQtimeSlices[pid] = MLFQ_getDefaultTimeSlice(pq_level);
}


void pqueue_init(ProcessQueue* pq){
	pq->size = MAX_NUMBER_OF_PROCESSES;
	pq->usize = 0;
	pq->head = 0;
	pq->tail = 0;
}

void pqueue_reset(ProcessQueue* pq){
	pqueue_init(pq);
}

bool pqueue_hasNext(const ProcessQueue* pq){
	return (pq->usize != 0);
}

ProcessID pqueue_getFirst(const ProcessQueue* pq){
	return pq->data[pq->tail];
}

void pqueue_dropFirst(ProcessQueue* pq){
	if (pq->usize == 0){return;}
	pq->usize = pq->usize - 1;
	pq->tail = (pq->tail + 1)%MAX_NUMBER_OF_PROCESSES;
}

bool pqueue_append(ProcessQueue* pq, ProcessID pid){
	if (pq->usize == MAX_NUMBER_OF_PROCESSES){return false;}
	pq->usize = pq->usize + 1;
	pq->data[pq->head] = pid;
	pq->head = (pq->head + 1)%MAX_NUMBER_OF_PROCESSES;
	return true;
}

void pqueue_removePID(ProcessQueue* pq, ProcessID pid){
	if (pq->usize == 0){return;}
	ProcessID new_data[MAX_NUMBER_OF_PROCESSES];
	uint8_t new_usize = 0;
	uint8_t j = 0;

	for (uint8_t i = 0; i<pq->usize; i++){
		uint8_t slot = (pq->tail+i)%MAX_NUMBER_OF_PROCESSES;
		if (pq->data[slot] != pid){
			new_usize += 1;
			new_data[j] = pq->data[slot];
			j++;
		}
	}

	for (uint8_t k = 0; k<new_usize; k++){
		pq->data[k] = new_data[k];
	}
	pq->tail = 0;
	pq->head = j;
	pq->usize = new_usize;
}

void os_initSchedulingInformation(){
	for (uint8_t i = 0; i<4; i++){
		pqueue_init(MLFQ_getQueue(i));
	}
}


/*!
*  Reset the schefduling information for a specific strategy
*  This is only relevant for RoundRobin and InactiveAging
*  and is done when the strategy is changed through os_setSchedulingStrategy
*
*  \param strategy  The strategy to reset information for
*/
void os_resetSchedulingInformation(SchedulingStrategy strategy)
{

	if (strategy == OS_SS_ROUND_ROBIN){
		ProcessID current = os_getCurrentProc();
		schedulingInfo.timeSlice = os_getProcessSlot(current)->priority;
	}
	if (strategy == OS_SS_INACTIVE_AGING){
		for (int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
		{
			os_resetProcessSchedulingInformation(i);
		}
	}
	if (strategy == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE){
		os_initSchedulingInformation();
		MLFQ_removeProcess(0);
		for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES; i++){
			MLFQ_removeProcess(i);
			if (os_getProcessSlot(i)->state != OS_PS_UNUSED){
				os_resetProcessSchedulingInformation(i);
				
			}
		}
	}
}

/*!
*  Reset the scheduling information for a specific process slot
*  This is necessary when a new process is started to clear out any
*  leftover data from a process that previously occupied that slot
*
*  \param id  The process slot to erase state for
*/
void os_resetProcessSchedulingInformation(ProcessID id)
{
	if (os_getSchedulingStrategy() == OS_SS_INACTIVE_AGING){
		schedulingInfo.age[id] = 0;
	}
	if (os_getSchedulingStrategy() == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE){
		MLFQ_removeProcess(id);
		MLFQ_initProcessTimeSlice(id);
		pqueue_append(MLFQ_getQueue(MLFQ_decodePriority(os_getProcessSlot(id)->priority)), id);
	}
}

void os_resetBlockedProcessState(volatile Process processes[]){
	for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES; i++){
		if (processes[i].state == OS_PS_BLOCKED){
			processes[i].state = OS_PS_READY;
		}
	}
}


/*!
*  This function implements the even strategy. Every process gets the same
*  amount of processing time and is rescheduled after each scheduler call
*  if there are other processes running other than the idle process.
*  The idle process is executed if no other process is ready for execution
*
*  \param processes An array holding the processes to choose the next process from.
*  \param current The id of the current process.
*  \return The next process to be executed determined on the basis of the even strategy.
*/
ProcessID os_Scheduler_Even(volatile Process processes[], ProcessID current)
{
	// Is the idle process forced?
	uint8_t rdy_count = 0;
	uint8_t blocked_count = 0;
	for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES; i++){
		if (processes[i].state == OS_PS_READY){rdy_count += 1;}
		if (processes[i].state == OS_PS_BLOCKED){blocked_count += 1;}
	}

	// Need to run idle?
	if (rdy_count + blocked_count == 0){
		os_resetBlockedProcessState(processes);
		return 0;
	}
	// Need to run only rdy?
	if (rdy_count == 1){
		for(uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES; i++){
			if (processes[i].state == OS_PS_READY){
				os_resetBlockedProcessState(processes);
				return i;
			}
		}
	}
	// Need to run a blocked process?
	// blocked_count >= 1
	if ((rdy_count == 0)){
		for(uint8_t i = current + 1;; i++){
			if (i == MAX_NUMBER_OF_PROCESSES){i=1;}
			if (processes[i].state == OS_PS_BLOCKED){
				os_resetBlockedProcessState(processes);
				return i;
			}
		}
	}

	// Multiple rdy
	for(uint8_t i = current + 1;; i++){
		if (i == MAX_NUMBER_OF_PROCESSES){i=1;}
		if (processes[i].state == OS_PS_READY){
			os_resetBlockedProcessState(processes);
			return i;
		}
	}
	os_resetBlockedProcessState(processes);
	return 0;
}

/*!
*  This function implements the random strategy. The next process is chosen based on
*  the result of a pseudo random number generator.
*
*  \param processes An array holding the processes to choose the next process from.
*  \param current The id of the current process.
*  \return The next process to be executed determined on the basis of the random strategy.
*/
ProcessID os_Scheduler_Random(volatile Process processes[], ProcessID current)
{
	uint8_t rdy_count = 0;
	for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
	{
		if (processes[i].state == OS_PS_READY){
			rdy_count++;
		}
	}
	if (rdy_count == 0){
		for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++){
			if (processes[i].state == OS_PS_BLOCKED){
				os_resetBlockedProcessState(processes);
				return i;
			}
		}
		os_resetBlockedProcessState(processes);
		return 0;
	}
	
	uint16_t selected = rand() % rdy_count;
	uint8_t i = 1;

	while (1)
	{

		while (1)
		{

			if (i >= MAX_NUMBER_OF_PROCESSES)
			{
				os_resetBlockedProcessState(processes);
				return 0;
			}

			if (processes[i].state == OS_PS_READY)
			{
				break;
				} else {
				i++;
			}
		}

		if (selected == 0)
		{
			os_resetBlockedProcessState(processes);
			return i;
			
			} else {
			selected -= 1;
			i++;
		}
	}
}

/*!
*  This function implements the round-robin strategy. In this strategy, process priorities
*  are considered when choosing the next process. A process stays active as long its time slice
*  does not reach zero. This time slice is initialized with the priority of each specific process
*  and decremented each time this function is called. If the time slice reaches zero, the even
*  strategy is used to determine the next process to run.
*
*  \param processes An array holding the processes to choose the next process from.
*  \param current The id of the current process.
*  \return The next process to be executed determined on the basis of the round robin strategy.
*/
ProcessID os_Scheduler_RoundRobin(volatile Process processes[], ProcessID current) {
	
	
	if ((schedulingInfo.timeSlice > 1) && processes[current].state == OS_PS_READY){
		schedulingInfo.timeSlice -= 1;
		os_resetBlockedProcessState(processes);
		return current;
	}
	ProcessID new = os_Scheduler_Even(processes, current);
	
	schedulingInfo.timeSlice = processes[new].priority;
	os_resetBlockedProcessState(processes);
	return new;
}

/*!
*  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
*  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
*  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
*  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
*  is to be returned is reset to 0.
*
*  \param processes An array holding the processes to choose the next process from.
*  \param current The id of the current process.
*  \return The next process to be executed, determined based on the inactive-aging strategy.
*/
ProcessID os_Scheduler_InactiveAging(volatile Process processes[], ProcessID current)
{
	Age max_age = 0;
	
	for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
	{
		if (processes[i].state != OS_PS_READY)
		{
			continue;
		}
		schedulingInfo.age[i] += processes[i].priority;
		if (schedulingInfo.age[i] > max_age)
		{
			max_age = schedulingInfo.age[i];
		}
	}
	
	uint8_t count = 0;
	
	for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++) {
		if (processes[i].state != OS_PS_READY){
			continue;
		}
		if (schedulingInfo.age[i] == max_age) {
			count++; // Increment count for the max_age
		}
	}
	
	// None rdy
	if (count == 0){
		for (uint8_t i = 1; i<MAX_NUMBER_OF_PROCESSES; i++){
			if (processes[i].state == OS_PS_BLOCKED){
				os_resetBlockedProcessState(processes);
				os_resetProcessSchedulingInformation(i);
				return i;
			}
		}
		
		os_resetBlockedProcessState(processes);
		return 0;
	}

	if (count == 1)
	{
		// Return oldest
		for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
		{
			if (processes[i].state != OS_PS_READY)
			{
				continue;
			}
			if (schedulingInfo.age[i] == max_age)
			{
				os_resetProcessSchedulingInformation(i);
				os_resetBlockedProcessState(processes);
				return i;
			}
		}
		} else {

		Priority highest_prio = 0;
		// Figure out highest prio among max age
		for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
		{
			if (processes[i].state != OS_PS_READY)
			{
				continue;
			}
			if (schedulingInfo.age[i] == max_age)
			{
				if (processes[i].priority > highest_prio)
				{
					highest_prio = processes[i].priority;
				}
			}
		}

		// Return highest prio among max age, automatically select the first (lowest pid)
		for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
		{
			if (processes[i].state != OS_PS_READY)
			{
				continue;
			}
			if (schedulingInfo.age[i] == max_age)
			{
				if (processes[i].priority == highest_prio)
				{
					os_resetProcessSchedulingInformation(i);
					os_resetBlockedProcessState(processes);
					return i;
				}
			}
		}
	}
	
	os_resetBlockedProcessState(processes);
	return 0;
}

/*!
*  This function realizes the run-to-completion strategy.
*  As long as the process that has run before is still ready, it is returned again.
*  If  it is not ready, the even strategy is used to determine the process to be returned
*
*  \param processes An array holding the processes to choose the next process from.
*  \param current The id of the current process.
*  \return The next process to be executed, determined based on the run-to-completion strategy.
*/
ProcessID os_Scheduler_RunToCompletion(volatile Process processes[], ProcessID current) {
	if ((processes[current].state == OS_PS_READY) && (current != 0)){
		os_resetBlockedProcessState(processes);
		return current;
	}
	return os_Scheduler_Even(processes, current);
}

ProcessID os_Scheduler_MLFQ(volatile Process processes[], ProcessID current){
	// Remove unused
	for (uint8_t pid = 1; pid<MAX_NUMBER_OF_PROCESSES; pid++){
		if (processes[pid].state == OS_PS_UNUSED){
			MLFQ_removeProcess(pid);
		}
	}

	// Push-back yielding procs
	for (uint8_t qid = 0; qid<4; qid++){
		ProcessQueue* queue = MLFQ_getQueue(qid);
		if (pqueue_hasNext(queue)){
			ProcessID pid = pqueue_getFirst(queue);
			if (processes[pid].state == OS_PS_BLOCKED){
				pqueue_dropFirst(queue);
				pqueue_append(queue, pid);
			}
		}
	}

	// Check first of each queue
	for (uint8_t qid = 0; qid<4; qid++){
		ProcessQueue* queue = MLFQ_getQueue(qid);
		if (pqueue_hasNext(queue)){
			ProcessID pid = pqueue_getFirst(queue);
			if (processes[pid].state == OS_PS_READY){

				// Time remaining
				if (schedulingInfo.MLFQtimeSlices[pid]>1){
					schedulingInfo.MLFQtimeSlices[pid] -= 1;
					os_resetBlockedProcessState(processes);
					return pid;
					// Time drops to 0
					
					} else {
					ProcessQueue* suc_queue = MLFQ_getQueue(MLFQ_successorQueue(qid));
					pqueue_removePID(queue, pid);
					pqueue_append(suc_queue, pid);
					schedulingInfo.MLFQtimeSlices[pid] = MLFQ_getDefaultTimeSlice(MLFQ_successorQueue(qid));
					os_resetBlockedProcessState(processes);
					return pid;
				}

			}
		}
	}

	// Try forcing a blocked process
	// Check first of each queue
	for (uint8_t qid = 0; qid<4; qid++){
		ProcessQueue* queue = MLFQ_getQueue(qid);
		if (pqueue_hasNext(queue)){
			ProcessID pid = pqueue_getFirst(queue);
			// This should always be true, as none are rdy unused or running and there is one process in this queue
			if (processes[pid].state == OS_PS_BLOCKED){

				// Time remaining
				if (schedulingInfo.MLFQtimeSlices[pid]>1){
					schedulingInfo.MLFQtimeSlices[pid] -= 1;
					os_resetBlockedProcessState(processes);
					return pid;
					// Time drops to 0
					} else {
					ProcessQueue* suc_queue = MLFQ_getQueue(MLFQ_successorQueue(qid));
					pqueue_removePID(queue, pid);
					pqueue_append(suc_queue, pid);
					schedulingInfo.MLFQtimeSlices[pid] = MLFQ_getDefaultTimeSlice(MLFQ_successorQueue(qid));
					os_resetBlockedProcessState(processes);
					return pid;
				}

			}
		}
	}

	os_resetBlockedProcessState(processes);
	return 0;
}