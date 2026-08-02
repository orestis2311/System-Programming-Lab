#include "os_memory_strategies.h"
#include "defines.h"

AllocStrategy alloc_strat = OS_MEM_FIRST;
uint16_t next_fit_search_point = 0;



// Returns the following nibble or 0 if no fit is found
MemAddr os_alloc_first_fit(Heap* heap, uint16_t size){

	// Number of bytes found
	uint16_t found_size = 0;

	uint16_t starting_nibble = 0;
	// 4 bits (= 1 nibble) of the map checked each time(map_size in bytes)
	for (uint16_t i = 0; (i/2)<(heap->map_size); i++){
		// Increase found size or reset
		if (os_readNibble(heap, i) == 0){
			found_size += 1;
			if (found_size >= size){
				// Return the starting nibble +1
				return (starting_nibble+1);
			}
			} else {
			found_size = 0;
			// This nibble is occupied, see if the next nibble is a valid start
			starting_nibble = i+1;
		}
	}
	// 0 reserved for error
	return 0;
}

// Returns the following nibble or 0 if no fit is found
MemAddr os_alloc_next_fit(Heap* heap, uint16_t size){
	
	
	// Number of bytes found
	uint16_t found_size = 0;

	uint16_t starting_nibble =  next_fit_search_point;
	// 4 bits (= 1 nibble) of the map checked each time(map_size in bytes)
	for (uint16_t i = next_fit_search_point; (i/2)<(heap->map_size); i++){
		// Increase found size or reset
		if (os_readNibble(heap, i) == 0){
			found_size += 1;
			if (found_size >= size){
				// Return the starting nibble +1
				next_fit_search_point = starting_nibble + found_size;
				return (starting_nibble+1);
			}
			} else {
			found_size = 0;
			// This nibble is occupied, see if the next nibble is a valid start
			starting_nibble = i+1;
		}
	}
	found_size = 0;
	starting_nibble = 0;
	
	for(uint16_t i=0;(i/2)<(heap->map_size); i++){
		
		// Increase found size or reset
		if (os_readNibble(heap, i) == 0){
			found_size += 1;
			if (found_size >= size){
				// Return the starting nibble +1
				next_fit_search_point = starting_nibble + found_size;
				return (starting_nibble+1);
			}
			} else {
			found_size = 0;
			// This nibble is occupied, see if the next nibble is a valid start
			starting_nibble = i+1;
		}
		
	}

	// 0 reserved for error
	return 0;
}


MemAddr os_alloc_best_fit(Heap* heap, uint16_t size){

	// Number of bytes found
	uint16_t found_size = 0;
	uint16_t best_fit_nibble = 0;
	uint16_t best_size = UINT16_MAX;
	uint16_t starting_nibble = 0;
	
	// 4 bits (= 1 nibble) of the map checked each time(map_size in bytes)
	for (uint16_t i = 0; (i/2)<(heap->map_size); i++){
		// Increase found size or reset
		if (os_readNibble(heap, i) == 0){
			found_size += 1;
			
			
			// TODO: Logic incorrect, best_size updated too early (free chunk can still grow and end up even larger)
			if (found_size > size && found_size < best_size){
				
				best_size = found_size;
				
				best_fit_nibble = starting_nibble;
				
				} else if(found_size == size){
				if(os_readNibble(heap,i+1) != 0)
				return starting_nibble+1;
			}
			
			} else {
			found_size = 0;
			// This nibble is occupied, see if the next nibble is a valid start
			starting_nibble = i+1;
		}
	}
	
	if(best_size == UINT16_MAX)
	return 0;
	else
	return best_fit_nibble+1;
	
	
	
	
}

MemAddr os_alloc_worst_fit(Heap* heap, uint16_t size){

	// Number of bytes found
	uint16_t found_size = 0;
	uint16_t worst_fit_nibble = 0;
	uint16_t worst_size = 0;
	uint16_t starting_nibble = 0;
	
	// 4 bits (= 1 nibble) of the map checked each time(map_size in bytes)
	for (uint16_t i = 0; (i/2)<(heap->map_size); i++){
		// Increase found size or reset
		if (os_readNibble(heap, i) == 0){
			found_size += 1;
			
			// TODO: Dont use heap size, -> use_size
			if(found_size >=(HEAP_SIZE/2) && found_size >= size){
				return starting_nibble + 1;
				
				}else if(found_size > size && found_size > worst_size){
				
				worst_size = found_size;
				
				worst_fit_nibble = starting_nibble;
				
			}
			
			} else {
			found_size = 0;
			// This nibble is occupied, see if the next nibble is a valid start
			starting_nibble = i+1;
		}
	}
	
	if(worst_size == 0)
	return 0;
	else
	return worst_fit_nibble+1;
	
	
	
	
}