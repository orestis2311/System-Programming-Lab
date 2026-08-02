#include "os_memory.h"
#include "os_memory_strategies.h"
#include "os_scheduler.h"
#include "os_core.h"

uint16_t walk_back(const Heap* heap, uint16_t index){
	while (1){
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}
	return index;
}

void os_adjust_alloc_bounds(ProcessID pid, MemAddr index){
	Process* p = os_getProcessSlot(pid);
	if (index < p->low_alloc){
		p->low_alloc = index;
	}
	if (index > p->high_alloc){
		p->high_alloc = index;
	}
}

MemAddr os_malloc(Heap* heap, uint16_t size){
	os_enterCriticalSection();
	AllocStrategy strat = os_getAllocationStrategy(heap);
	ProcessID pid = os_getCurrentProc();
	
	// The nib where a possible block could start
	uint16_t index = 0;

	if (strat == OS_MEM_FIRST){
		index = os_alloc_first_fit(heap, size);
		// No possible alloc
		if (index == 0){os_leaveCriticalSection(); return 0;}
		// Decode the 0=Error encoding
		index -= 1;
		} else if (strat == OS_MEM_NEXT){
		index = os_alloc_next_fit(heap, size);
		if (index == 0){os_leaveCriticalSection(); return 0;}
		// Decode the 0=Error encoding
		index -= 1;
		} else if (strat == OS_MEM_BEST){
		index = os_alloc_best_fit(heap, size);
		if (index == 0){os_leaveCriticalSection(); return 0;}
		// Decode the 0=Error encoding
		index -= 1;
		} else if (strat == OS_MEM_WORST){
		index = os_alloc_worst_fit(heap, size);
		if (index == 0){os_leaveCriticalSection(); return 0;}
		// Decode the 0=Error encoding
		index -= 1;
		} else {
		os_error("Unkown Alloc Strat");
	}

	// Assign corresponding nibbles to the current process
	os_writeNibble(heap, index, (uint8_t)pid);
	for (MemAddr i = 1; i<size; i++){
		os_writeNibble(heap, index+i, 0xF);
	}

	// Get the n-th use-byte for the n-th nibble
	MemAddr addr = heap->use_start + (index);

	os_adjust_alloc_bounds(pid, index);

	os_leaveCriticalSection();
	return addr; /* memory address (first use-byte) */;
}

MemAddr os_sh_malloc (Heap *heap, size_t size){
	os_enterCriticalSection();
	// Does an unnecessary adjust of alloc bounds but who cares
	MemAddr addr = os_malloc(heap, size);
	if (addr == 0){os_leaveCriticalSection(); return 0;}
	MemAddr index = (addr - heap->use_start);
	os_writeNibble(heap, index, 8);
	os_leaveCriticalSection();
	return addr;
}

void os_mem_move(Heap* heap, MemAddr old_index, MemAddr new_index, uint16_t size){
	uint16_t offset = 0;
	while (offset<size){
		MemValue byte = heap->driver->read(heap->use_start + old_index + offset);
		heap->driver->write(heap->use_start + new_index + offset, byte);
		offset += 1;
	}
}

MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size){
	os_enterCriticalSection();
	if (size == 0){
		os_free(heap, addr);
		os_leaveCriticalSection();
		return 0;
	}
	ProcessID pid = os_getCurrentProc();
	MemAddr index = (addr - heap->use_start);

	if (os_readNibble(heap, index) == 0){
		os_leaveCriticalSection();
		return 0;
	}

	// Walk back
	while (index != 0 && os_readNibble(heap, index) == 0xF){
		index -= 1;
	}

	// Start of current alloc
	MemAddr base_index = index;
	if (os_readNibble(heap, base_index) != pid){
		os_error("No permission to realloc");
		os_leaveCriticalSection();
		return 0;
	}
	// Find r.end r.bound,
	// If r.end contains size, reduce
	// r.bound contains size expand
	// Otherwise:
	// find l.bound
	// Is move to l.bound possible?
	// Malloc or move

	// Skip permission nibble
	index += 1;
	// Find r_end
	while(index < heap->use_size && os_readNibble(heap, index) == 0xF){
		index += 1;
	}
	// [base_index, r_end) is current alloc
	MemAddr r_end = index;
	
	// Reduce
	if (base_index + size <= r_end){
		index = base_index + size;
		while(index < r_end){
			os_writeNibble(heap, index, 0);
			index += 1;
		}
		os_leaveCriticalSection();
		return heap->use_start + base_index;
	}

	// Find r_bound
	while(index < heap->use_size && os_readNibble(heap, index) == 0){
		index +=1;
	}
	// [base_index, r_bound) is maximal right alloc
	MemAddr r_bound = index;
	
	// Right expand
	if (base_index + size <= r_bound){
		index = r_end;
		while(index < base_index + size){
			os_writeNibble(heap, index, 0xF);
			index += 1;
		}
		os_leaveCriticalSection();
		return heap->use_start + base_index;
	}
	
	// Find l_bound
	MemAddr l_bound = 0;
	if (base_index != 0){
		index = base_index;
		while(index != 0 && os_readNibble(heap, index-1) == 0){
			index -= 1;
		}
		// [l_bound, r_bound) is maximal combined alloc
		l_bound = index;
	}

	if (l_bound + size <= r_bound){
		// Move to l_bound
		index = l_bound+1;
		os_writeNibble(heap, l_bound, pid);
		while(index < l_bound + size){
			os_writeNibble(heap, index, 0xF);
			index += 1;
		}
		while(index < r_end){
			os_writeNibble(heap, index, 0);
			index += 1;
		}
		os_mem_move(heap, base_index, l_bound, r_end - base_index);
		os_adjust_alloc_bounds(pid, l_bound);
		os_leaveCriticalSection();
		return heap->use_start + l_bound;
		} else {
		// Malloc
		MemAddr addr = os_malloc(heap, size);
		os_mem_move(heap, base_index, addr - heap->use_start, r_end - base_index);
		os_free(heap, heap->use_start + base_index);
		os_leaveCriticalSection();
		return addr;
	}
}

void os_free(Heap* heap, MemAddr addr){
	// use_start+7 -> 0th nib; use_start+8 -> first nib ...
	os_enterCriticalSection();
	uint16_t index = (addr - heap->use_start);
	// Walk back
	while (1)
	{
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}

	// Check perm
	MemValue expected = (MemValue) os_getCurrentProc();
	if (os_readNibble(heap, index) != expected){
		// Belong to other process or double free
		os_error("No permission to free");
	}

	// Reset nibbles
	uint16_t i = 0;

	while (index+i<heap->use_size)
	{
		// This guards use, but free needs more?
		//if (heap->map_size<=i){break;}
		os_writeNibble(heap, index+i, 0);
		i += 1;
		MemValue seen = os_readNibble(heap, index+i);
		if (seen != 0xF){
			break;
		}
	}
	os_leaveCriticalSection();
}

void os_free_force(Heap* heap, MemAddr addr){
	// use_start+7 -> 0th nib; use_start+8 -> first nib ...
	os_enterCriticalSection();
	uint16_t index = (addr - heap->use_start);
	// Walk back
	while (1)
	{
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}

	// Reset nibbles
	uint16_t i = 0;

	while (index+i<heap->use_size)
	{
		os_writeNibble(heap, index+i, 0);
		i += 1;
		MemValue seen = os_readNibble(heap, index+i);
		if (seen != 0xF){
			break;
		}
	}
	os_leaveCriticalSection();
}

void os_sh_free (Heap *heap, MemAddr *ptr){
	os_enterCriticalSection();
	MemAddr addr = os_sh_writeOpen(heap, ptr);
	os_free_force(heap, addr);
	os_leaveCriticalSection();
}

void os_freeProcessMemory(Heap* heap , ProcessID id){
	
	os_enterCriticalSection();

	Process* p = os_getProcessSlot(id);

	uint16_t index = p->low_alloc;
	
	
	while(index < heap->use_size && index <= p->high_alloc){
		
		if(os_readNibble(heap,index) == id){
			MemAddr addr = heap->use_start + (index);
			os_free_force(heap,addr);
			} else {
			index++;
		}
		
	}
	
	os_leaveCriticalSection();
	
}

// TODO: verify that ownership nibbles are for shared processes - verify function for private mem dont work on sh mem
// 0 free, 1-7 owned by pid; shmem: 8 avail, 9 write locked, 10, 11, 12, 13, 14 - read locked by 1, 2, 3, 4, 5 processes; 15 = 0xF continue

void os_sh_write (const Heap *heap, const MemAddr *ptr, uint16_t offset, const MemValue *dataSrc, uint16_t length){
	MemAddr addr = os_sh_writeOpen(heap, ptr);
	if (addr == 0){os_error("Failed to writeOpen"); return;}

	uint16_t max_len = os_getChunkSize(heap, addr);
	if (offset >= max_len){os_error("offset to large");}
	for(uint16_t i = 0; i<length; i++){
		if (offset+i < max_len){
			uint8_t data = intHeap->driver->read(((MemAddr)dataSrc)+i);
			heap->driver->write(addr+offset+i, data);
			} else {
			os_error("sh_write ooB");
			break;
		}
	}
	
	os_sh_close(heap, addr);
}

void os_sh_read (const Heap *heap, const MemAddr *ptr, uint16_t offset, MemValue *dataDest, uint16_t length){
	MemAddr addr = os_sh_readOpen(heap, ptr);
	if (addr == 0){os_error("Failed to readOpen"); return;}

	uint16_t max_len = os_getChunkSize(heap, addr);
	if (offset >= max_len){os_error("offset to large");}

	for(uint16_t i = 0; i<length; i++){
		if(offset+i < max_len){
			uint8_t data = heap->driver->read(addr+offset+i);
			intHeap->driver->write(((MemAddr)dataDest)+i, data);
			} else {
			os_error("sh_read ooB");
			break;
		}
	}
	
	os_sh_close(heap, addr);
}

MemAddr os_sh_readOpen (const Heap *heap, const MemAddr *ptr){
	os_enterCriticalSection();
	MemAddr addr = *ptr;
	uint16_t index = (addr - heap->use_start);
	// Walk back
	index = walk_back(heap, index);

	if (os_readNibble(heap, index) <= 7){
		os_error("sh_readOpen to nonshare mem");
		os_leaveCriticalSection();
		return 0;
	}
	while(1){
		addr = *ptr;
		index = (addr - heap->use_start);
		index = walk_back(heap, index);
		if ((os_readNibble(heap, index) == 9)||(os_readNibble(heap, index) == 14)){
			os_yield();
			} else {
			break;
		}
	}
	
	if(os_readNibble(heap, index) == 8){
		os_writeNibble(heap, index, 10);
		} else {
		os_writeNibble(heap, index, os_readNibble(heap, index)+1);
	}
	addr = index + heap->use_start;
	os_leaveCriticalSection();
	return addr;
}

MemAddr os_sh_writeOpen (const Heap *heap, const MemAddr *ptr){
	os_enterCriticalSection();
	MemAddr addr = *ptr;
	uint16_t index = (addr - heap->use_start);
	
	// Walk back
	while (1){
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}

	if (os_readNibble(heap, index) <= 7){
		os_error("sh_writeOpen to nonshare mem");
		os_leaveCriticalSection();
		return 0;
	}
	
	while(1){
		addr = *ptr;
		index = (addr - heap->use_start);
		index = walk_back(heap, index);
		if (os_readNibble(heap, index) != 8){
			os_yield();
			} else {
			break;
		}
	}
	
	
	os_writeNibble(heap, index, 9);
	addr = index + heap->use_start;
	os_leaveCriticalSection();
	return addr;
}

void os_sh_close (const Heap *heap, MemAddr addr){
	os_enterCriticalSection();
	uint16_t index = (addr - heap->use_start);
	// Walk back
	while (1){
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}

	if (os_readNibble(heap, index) <= 7){
		os_error("sh_close to nonshare mem");
	}
	if((os_readNibble(heap, index) == 9)||(os_readNibble(heap, index) == 10)){
		os_writeNibble(heap, index, 8);
	}
	if(os_readNibble(heap, index) > 10){
		os_writeNibble(heap, index, os_readNibble(heap, index)-1);
	}
	os_leaveCriticalSection();
}

size_t os_getMapSize(Heap const* heap){
	return (size_t)heap->map_size;
}
size_t os_getUseSize(Heap const* heap){
	return (size_t)heap->use_size;
}
MemAddr os_getMapStart(Heap const* heap){
	return heap->map_start;
}
MemAddr os_getUseStart(Heap const* heap){
	return heap->use_start;
}
AllocStrategy os_getAllocationStrategy(Heap const* heap){
	return heap->strategy;
}
void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat){
	os_enterCriticalSection();
	heap->strategy = allocStrat;
	os_leaveCriticalSection();
}

uint16_t os_getChunkSize(Heap const* heap, MemAddr addr){
	os_enterCriticalSection();
	MemAddr index = (addr - heap->use_start);
	if (os_readNibble(heap, index) == 0){
		os_leaveCriticalSection();
		return 0;
	}

	// Walk back
	while (1)
	{
		if (os_readNibble(heap, index) == 0xF){
			index -= 1;
			} else {
			break;
		}
	}

	// Skip the ownership flag
	uint16_t chunk_size = 1;
	index += 1;
	
	// Walk over 0xF
	// At most use_size different indices into the map
	while (index<heap->use_size)
	{
		if (os_readNibble(heap, index) == 0xF){
			index += 1;
			chunk_size += 1;
			} else {
			break;
		}
	}
	os_leaveCriticalSection();
	return chunk_size;
}