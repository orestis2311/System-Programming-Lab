/*! \file
 *  \brief Simple definitions and assembler-macros, mostly 8/16-Bit values.
 *
 *  All constant values that can be simply parsed into the code at compile
 *  time are stored in here. These will reside in program memory at runtime.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _DEFINES_H
#define _DEFINES_H

#include "atmega644constants.h"

//----------------------------------------------------------------------------
// Programming reliefs
//----------------------------------------------------------------------------

/*!
 *  null-pointer
 *  The rationale for this is to be compliant with stddef::NULL without
 *  actually including it.
 */
#ifndef NULL
#define NULL ((void *)0)
#endif

//----------------------------------------------------------------------------
// Debug/Version settings
//----------------------------------------------------------------------------

//! The current id of the exercise (this must be changed every two weeks).
#define VERSUCH 6

//----------------------------------------------------------------------------
// System constants
//----------------------------------------------------------------------------

/*!
 *  Maximum number of processes that can be running at the same time
 *  (may be nothing > 8).
 *  This number includes the idle proc, although it is considered a system proc.
 *  The idle proc. has always id 0. The highest ID is MAX_NUMBER_OF_PROCESSES-1.
 */
#define MAX_NUMBER_OF_PROCESSES 3
#define NUMBER_OF_HEAPS 1

//! Standard priority for newly created processes
#define DEFAULT_PRIORITY 2

//! Default delay to read display values (in ms)
#ifndef DEFAULT_OUTPUT_DELAY
#define DEFAULT_OUTPUT_DELAY 100
#endif

//----------------------------------------------------------------------------
// Scheduler constants
//----------------------------------------------------------------------------

//! Number to specify an invalid process
#define INVALID_PROCESS 255

//----------------------------------------------------------------------------
// Stack constants
//----------------------------------------------------------------------------

//! The stack size available for initialization and globals
#define STACK_SIZE_MAIN 32

//! The scheduler's stack size
#define STACK_SIZE_ISR 192

#define STACK_FRACTION (2) // otherwise: (2)

#define POSITIVE_STACK_SIZE (((AVR_MEMORY_SRAM / STACK_FRACTION) > STACK_SIZE_MAIN + STACK_SIZE_ISR)

//! The stack size of a process
#define STACK_SIZE_PROC (((AVR_MEMORY_SRAM / STACK_FRACTION) - STACK_SIZE_MAIN - STACK_SIZE_ISR) / MAX_NUMBER_OF_PROCESSES)


//! The bottom of the main stack. That is the highest address.
#define BOTTOM_OF_MAIN_STACK (AVR_SRAM_LAST)

//! The bottom of the scheduler-stack. That is the highest address.
#define BOTTOM_OF_ISR_STACK (BOTTOM_OF_MAIN_STACK - STACK_SIZE_MAIN)

//! The bottom of the memory chunks for all process stacks. That is the highest address.
#define BOTTOM_OF_PROCS_STACK (BOTTOM_OF_ISR_STACK - STACK_SIZE_ISR)

//! The bottom of the memory chunk with number PID.
#define PROCESS_STACK_BOTTOM(PID) (BOTTOM_OF_PROCS_STACK - ((PID)*STACK_SIZE_PROC))

//----------------------------------------------------------------------------
// Heap constants
//----------------------------------------------------------------------------

// NOTE: __heap_start is a (useless) value; use &__heap_start to get the address of the heap start
extern uint8_t const __heap_start;

#define HEAPOFFSET 2000

// First address -- err whatever
#define BOTTOM_OF_HEAP ((AVR_SRAM_START) + HEAPOFFSET)

// First address outside of the heap
#define TOP_OF_HEAP ((AVR_MEMORY_SRAM - (AVR_MEMORY_SRAM / STACK_FRACTION)))

#define HEAP_SIZE (TOP_OF_HEAP-BOTTOM_OF_HEAP)

#endif

//----------------------------------------------------------------------------
// exS constants
//----------------------------------------------------------------------------

#define DUMMY (0xFF)

#define CSETMODE (0x01)
#define CWRITE (0x02)
#define CREAD (0x03)

// SPCR
#define SPE_V (1)
#define DORD_V (0)
#define MSTR_V (1)
#define CPOL_V (0)
#define CPHA_V (0)
#define SPR1_V (0)
#define SPR0_V (0)

// SPSR
#define SPI2X_V (1) 

