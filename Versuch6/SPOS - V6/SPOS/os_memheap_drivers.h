
#ifndef _OS_MEMHEAP_DRIVERS_H
#define _OS_MEMHEAP_DRIVERS_H
#include "defines.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "os_mem_drivers.h"

#define intHeap (&intHeap__)
// #define extHeap (&extHeap__)

typedef enum AllocStrategy {
	OS_MEM_FIRST,
	OS_MEM_NEXT,
	OS_MEM_BEST,
	OS_MEM_WORST
} AllocStrategy;

typedef struct Heap
{
	MemDriver* driver;

	MemAddr map_start;
	MemAddr map_size;
	MemAddr use_start;
	MemAddr use_size;
	
	AllocStrategy strategy;

	// Apparently a char name instead
	char* const name;
} Heap;

MemValue os_readNibble(const Heap *heap, MemAddr index);

void os_writeNibble(const Heap *heap, MemAddr index, uint8_t nib);

void os_initHeaps();

uint8_t os_getHeapListLength();

Heap* os_lookupHeap(uint8_t index);

Heap intHeap__;
// Heap extHeap__;

#endif