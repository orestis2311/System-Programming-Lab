/*! \file
*  \brief Scheduling module for the OS.
*
* Contains everything needed to realise the scheduling between multiple processes.
* Also contains functions to start the execution of programs.
*
*  \author Lehrstuhl Informatik 11 - RWTH Aachen
*/

#include "os_scheduler.h"

#include "lcd.h"
#include "os_core.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "util.h"
#include "os_memheap_drivers.h"
#include "os_memory.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals

//----------------------------------------------------------------------------
//! Array of states for every possible process
Process os_processes[MAX_NUMBER_OF_PROCESSES];
//! Index of process that is currently executed (default: idle)
ProcessID currentProc = 0;
//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
SchedulingStrategy sched_strat = OS_SS_EVEN;

//! Count of currently nested critical sections
uint8_t criticalSectionCount = 0;

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect)
__attribute__((naked));

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
*  Timer interrupt that implements our scheduler. Execution of the running
*  process is suspended and the context saved to the stack. Then the periphery
*  is scanned for any input events. If everything is in order, the next process
*  for execution is derived with an exchangeable strategy. Finally the
*  scheduler restores the next process for execution and releases control over
*  the processor to that process.
*/
ISR(TIMER2_COMPA_vect)
{

	saveContext();
	// Save the stack pointer of the currently executing process
	os_processes[currentProc].sp.as_int = SP;

	// Ensure the stack pointer is within valid bounds
	// if (SP > (uint16_t)PROCESS_STACK_BOTTOM(idx) || SP < (uint16_t)(PROCESS_STACK_BOTTOM(currentProc) - STACK_SIZE_PROC)) {
	// os_error("Invalid stack pointer in ISR");
	//}

	SP = BOTTOM_OF_ISR_STACK;
	os_enterCriticalSection();

	// Do checksum
	os_processes[currentProc].checksum = os_getStackChecksum(currentProc);

	if (os_processes[currentProc].state == OS_PS_RUNNING){
		os_processes[currentProc].state = OS_PS_READY;
	}

	// New current proc
	if (os_getSchedulingStrategy() == OS_SS_EVEN)
	{
		currentProc = os_Scheduler_Even(os_processes, currentProc);
	}
	else if (os_getSchedulingStrategy() == OS_SS_RANDOM)
	{
		currentProc = os_Scheduler_Random(os_processes, currentProc);
	}
	else if (os_getSchedulingStrategy() == OS_SS_INACTIVE_AGING)
	{
		currentProc = os_Scheduler_InactiveAging(os_processes, currentProc);
	}
	else if (os_getSchedulingStrategy() == OS_SS_ROUND_ROBIN)
	{
		currentProc = os_Scheduler_RoundRobin(os_processes, currentProc);
	}
	else if (os_getSchedulingStrategy() == OS_SS_RUN_TO_COMPLETION)
	{
		currentProc = os_Scheduler_RunToCompletion(os_processes, currentProc);
	}
	else if (os_getSchedulingStrategy() == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE)
	{
		currentProc = os_Scheduler_MLFQ(os_processes, currentProc);

	} else {
		os_error("Exhausted Strats");
	}
	
	
	ProcessState s = os_processes[currentProc].state;
	
	if (s != OS_PS_READY){
		os_error("Bad scheduling");
		if (s == OS_PS_RUNNING){
			os_error("Run");
		}
		if (s == OS_PS_UNUSED){
			os_error("Unused");
		}
		if (s == OS_PS_BLOCKED){
			os_error("Blocked");
		}
		if ((uint16_t)s >= 5){
			os_error("Too big");
		}
		_delay_ms(1000);
	}
	// Verify the stack pointer for the new current process
	if (os_processes[currentProc].sp.as_int < (uint16_t)(PROCESS_STACK_BOTTOM(currentProc) - STACK_SIZE_PROC) ||
	os_processes[currentProc].sp.as_int > (uint16_t)PROCESS_STACK_BOTTOM(currentProc))
	{
		os_error("Invalid stack pointer in ISR for next process");
	}

	if (os_processes[currentProc].checksum != os_getStackChecksum(currentProc))
	{
		os_error("Stack checksum mismatch");
	}

	os_processes[currentProc].state = OS_PS_RUNNING;
	os_leaveCriticalSection();

	SP = os_processes[currentProc].sp.as_int;

	restoreContext();
}

/*!
*  This is the idle program. The idle process owns all the memory
*  and processor time no other process wants to have.
*/
void idle(void)
{
	lcd_clear();
	lcd_line1();

	while (1)
	{
		// Monitor stack usage
		if (SP < (uint16_t)PROCESS_STACK_BOTTOM(1))
		{
			os_error("Idle process stack overflow");
		}

		lcd_writeChar('.');
		_delay_ms(DEFAULT_OUTPUT_DELAY);
	}
}

void os_dispatcher(void){
	
	os_enterCriticalSection();
	ProcessID id = os_getCurrentProc();
	Program *p = os_processes[id].program;
	os_leaveCriticalSection();

	if (p == NULL){
		os_error("Dispatch null program");
	}

	p();

	os_kill(id);
	
	while (1);
}


bool os_kill(ProcessID pid){
	os_enterCriticalSection();
	if (pid == 0){
		os_leaveCriticalSection();
		return false;
	}

	os_processes[pid].state = OS_PS_UNUSED;
	for(uint8_t i=0; i<NUMBER_OF_HEAPS; i++){
		os_freeProcessMemory(os_lookupHeap(i),pid);
	}

	ProcessID id = os_getCurrentProc();
	if (pid == id){
		// Self-Term
		// while(criticalSectionCount >= 1){os_leaveCriticalSection();}
		os_yield();
	} else {
		// Other Term
		os_leaveCriticalSection();
	}
	return true;
}

void os_yield(){
	os_enterCriticalSection();
	ProcessID pid = os_getCurrentProc();
	if (os_processes[pid].state != OS_PS_UNUSED){
		os_processes[pid].state = OS_PS_BLOCKED;
	}

	uint8_t gieb = SREG >> 7;
	uint8_t cs_depth = criticalSectionCount;
	criticalSectionCount = 0;
	sei();
	TIMSK2 |= (1 << OCIE2A);

	TIMER2_COMPA_vect();
	
	TIMSK2 &= ~(1 << OCIE2A);
	criticalSectionCount = cs_depth;
	if (gieb){
		sei();
	} else {
		cli();
	}
	os_leaveCriticalSection();
}


/*!
*  This function is used to register the given program for execution.
*  A stack will be provided if the process limit has not yet been reached.
*  This function is multitasking safe. That means that programs can repost
*  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
*
*  \param program  The function of the program to start.
*  \param priority A priority ranging 0..255 for the new process:
*                   - 0 means least favourable
*                   - 255 means most favourable
*                  Note that the priority may be ignored by certain scheduling
*                  strategies.
*  \return The index of the new process or INVALID_PROCESS as specified in
*          defines.h on failure
*/
ProcessID os_exec(Program *program, Priority priority)
{
	/*
	1. Freien Platz im Array os_processes finden
	2. Den angegebenen Zeiger auf das Programm auf Validität überprüfen
	3. Programm, Prozesszustand und Prozesspriorität speichern
	4. Prozessstack vorbereiten
	*/
	os_enterCriticalSection();
	uint16_t i = 0;
	while (1)
	{
		if (i >= MAX_NUMBER_OF_PROCESSES)
		{
			os_leaveCriticalSection();
			os_error("Proc overflow");
			return INVALID_PROCESS;
		}
		if (os_processes[i].state == OS_PS_UNUSED)
		{
			break;
		}
		i++;
	}

	if (program == NULL)
	{
		os_leaveCriticalSection();
		return INVALID_PROCESS;
	}

	os_processes[i].program = program;
	os_processes[i].priority = priority;
	os_processes[i].state = OS_PS_READY;
	os_processes[i].low_alloc = 0;
	os_processes[i].high_alloc = 0;


	os_resetProcessSchedulingInformation(i);

	os_processes[i].sp.as_int = (uint16_t)PROCESS_STACK_BOTTOM(i);

	uint8_t left_byte = (uint8_t)(((uint16_t)(os_dispatcher) & 0xFF00) >> 8);
	uint8_t right_byte = (uint8_t)((uint16_t)(os_dispatcher) & 0x00FF);

	*(os_processes[i].sp.as_ptr) = right_byte;
	*(((uint8_t *)(os_processes[i].sp.as_ptr)) - 1) = left_byte;

	// Initialize with 33 zero bytes, SREG + 32regs
	for (int j = 1; j <= 33; j++)
	{
		*(((uint8_t *)(os_processes[i].sp.as_ptr)) - 1 - j) = 0;
	}

	// Set sp above all registers
	os_processes[i].sp.as_ptr = (((uint8_t *)os_processes[i].sp.as_ptr) - 35);
	os_processes[i].checksum = os_getStackChecksum(i);

	os_leaveCriticalSection();

	return i;
}

/*!
*  If all processes have been registered for execution, the OS calls this
*  function to start the idle program and the concurrent execution of the
*  applications.
*/
void os_startScheduler(void)
{

	// Currently running process
	currentProc = 0;

	os_processes[currentProc].state = OS_PS_RUNNING;
	SP = os_processes[currentProc].sp.as_int;

	restoreContext();
}

/*!
*  In order for the Scheduler to work properly, it must have the chance to
*  initialize its internal data-structures and register.
*/
void os_initScheduler()
{
	// Step 1: Initialize all entries in os_processes to OS_PS_UNUSED
	for (int i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
	{
		os_processes[i].state = OS_PS_UNUSED;
	}

	// Step 2: Initialize the idle process in os_processes[0]
	os_exec(idle, DEFAULT_PRIORITY);

	// Step 3: Validate autostart list
	struct program_linked_list_node *current_node = autostart_head;
	if (current_node == NULL)
	{
		os_error("Autostart list is empty");
	}

	// Step 4: Traverse the autostart_head list
	while (current_node != NULL)
	{
		Program *program = current_node->program;
		if (program != NULL)
		{
			// TODO: If every program starts with default priority...
			// then the testtask are doing something weird - do they set it manually?
			os_exec(program, DEFAULT_PRIORITY);
		}
		current_node = current_node->next;
	}

	os_initSchedulingInformation();
}

/*!
*  A simple getter for the slot of a specific process.
*
*  \param pid The processID of the process to be handled
*  \return A pointer to the memory of the process at position pid in the os_processes array.
*/
Process *os_getProcessSlot(ProcessID pid)
{
	return &os_processes[pid];
}

/*!
*  A simple getter to retrieve the currently active process.
*
*  \return The process id of the currently active process.
*/
ProcessID os_getCurrentProc(void)
{
	return currentProc;
}

/*!
*  Sets the current scheduling strategy.
*
*  \param strategy The strategy that will be used after the function finishes.
*/
void os_setSchedulingStrategy(SchedulingStrategy strategy)
{
	sched_strat = strategy;
	os_resetSchedulingInformation(sched_strat);
}

/*!
*  This is a getter for retrieving the current scheduling strategy.
*
*  \return The current scheduling strategy.
*/
SchedulingStrategy os_getSchedulingStrategy(void)
{
	return sched_strat;
}

/*!
*  Enters a critical code section by disabling the scheduler if needed.
*  This function stores the nesting depth of critical sections of the current
*  process (e.g. if a function with a critical section is called from another
*  critical section) to ensure correct behaviour when leaving the section.
*  This function supports up to 255 nested critical sections.
*/
void os_enterCriticalSection(void)
{
	uint8_t interrupt_enable = SREG & 0b10000000;
	cli();
	if (criticalSectionCount == 255)
	{
		os_error("Critical section overflow");
	}
	else
	{
		criticalSectionCount += 1;
	}
	TIMSK2 &= ~(1 << OCIE2A); // Disable timer interrupt
	if (interrupt_enable != 0)
	{
		sei();
	}
}

/*!
*  Leaves a critical code section by enabling the scheduler if needed.
*  This function utilizes the nesting depth of critical sections
*  stored by os_enterCriticalSection to check if the scheduler
*  has to be reactivated.
*/
void os_leaveCriticalSection(void)
{
	uint8_t interrupt_enable = SREG & 0b10000000;
	cli();
	if (criticalSectionCount == 0)
	{
		os_error("Critical section underflow");
	}
	else
	{
		criticalSectionCount -= 1;
	}
	if (criticalSectionCount == 0)
	{
		TIMSK2 |= (1 << OCIE2A); // Re-enable timer interrupt
	}
	if (interrupt_enable != 0)
	{
		sei();
	}
}

/*!
*  Calculates the checksum of the stack for a certain process.
*
*  \param pid The ID of the process for which the stack's checksum has to be calculated.
*  \return The checksum of the pid'th stack.
*/
StackChecksum os_getStackChecksum(ProcessID pid)
{
	StackChecksum sum = 0;
	uint8_t *runner = (uint8_t *)PROCESS_STACK_BOTTOM(pid);

	while (1)
	{
		// Runner overtook sp
		if (os_processes[pid].sp.as_int >= (uint16_t)runner)
		{
			break;
		}

		sum ^= *runner;
		// Stack grows down
		runner -= 1;
	}
	return sum;
}