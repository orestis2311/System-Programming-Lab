/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduling strategies.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H

#include "defines.h"
#include "os_scheduler.h"
#include "os_core.h"



//! Structure used to store specific scheduling informations such as a time slice
typedef struct{
	ProcessID data[MAX_NUMBER_OF_PROCESSES];
	uint8_t size;
	uint8_t usize;
	uint8_t head;
	uint8_t tail;
}ProcessQueue;

typedef struct {
	Age age[MAX_NUMBER_OF_PROCESSES];
	uint8_t timeSlice;
	uint8_t MLFQtimeSlices[MAX_NUMBER_OF_PROCESSES];
	ProcessQueue pqueues[4];
}SchedulingInformation;


ProcessQueue* MLFQ_getQueue(uint8_t index);
void MLFQ_removeProcess(ProcessID pid);
uint8_t MLFQ_successorQueue(uint8_t queue_id);
uint8_t MLFQ_decodePriority(Priority prio);
uint8_t MLFQ_getDefaultTimeSlice(uint8_t queue_id);
void MLFQ_initProcessTimeSlice(ProcessID pid);
void pqueue_init(ProcessQueue* pq);
void pqueue_reset(ProcessQueue* pq);
bool pqueue_hasNext(const ProcessQueue* pq);
ProcessID pqueue_getFirst(const ProcessQueue* pq);
void pqueue_dropFirst(ProcessQueue* pq);
bool pqueue_append(ProcessQueue* pq, ProcessID pid);
void pqueue_removePID(ProcessQueue* pq, ProcessID pid);

void os_resetBlockedProcessState(volatile Process processes[]);

void os_initSchedulingInformation();
//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(ProcessID id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(SchedulingStrategy strategy);

//! Even strategy
ProcessID os_Scheduler_Even(volatile Process processes[], ProcessID current);

//! Random strategy
ProcessID os_Scheduler_Random(volatile Process processes[], ProcessID current);

//! RoundRobin strategy
ProcessID os_Scheduler_RoundRobin(volatile Process processes[], ProcessID current);

//! InactiveAging strategy
ProcessID os_Scheduler_InactiveAging(volatile Process processes[], ProcessID current);

//! RunToCompletion strategy
ProcessID os_Scheduler_RunToCompletion(volatile Process processes[], ProcessID current);

ProcessID os_Scheduler_MLFQ(volatile Process processes[], ProcessID current);

#endif