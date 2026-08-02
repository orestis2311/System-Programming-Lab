//TODO: include in Microchip
#ifndef OS_MEMORY_STRATEGIES_H
#define OS_MEMORY_STRATEGIES_H

#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"



MemAddr os_alloc_first_fit(Heap* heap, uint16_t size);
MemAddr os_alloc_next_fit(Heap* heap, uint16_t size);
MemAddr os_alloc_best_fit(Heap* heap, uint16_t size);
MemAddr os_alloc_worst_fit(Heap* heap, uint16_t size);


#endif