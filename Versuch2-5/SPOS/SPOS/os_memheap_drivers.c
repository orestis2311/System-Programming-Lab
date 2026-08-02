#include "os_memheap_drivers.h"
#include "defines.h"


Heap intHeap__ = {
	.driver = intSRAM,
	.map_start = BOTTOM_OF_HEAP,
	.map_size = ((HEAP_SIZE)/3),
	.use_start = (BOTTOM_OF_HEAP+((HEAP_SIZE)/3)),
	.use_size = (((HEAP_SIZE)/3)*2),
	.strategy = OS_MEM_FIRST,
	.name = "intHeap_name"
};

Heap extHeap__ = {
	.driver = extSRAM,
	.map_start = 0,
	.map_size = (UINT16_MAX/3),
	.use_start = 0 + (UINT16_MAX/3),
	.use_size = (UINT16_MAX/3)*2,
	.strategy = OS_MEM_FIRST,
	.name = "extHeap_name"
};
Heap* heaplist[NUMBER_OF_HEAPS] = {intHeap, extHeap};



void os_initHeaps(){
	for (int i = 0; i<NUMBER_OF_HEAPS; i++){
		
		heaplist[i]->driver->init();
		// Reset nibbles
		for (int j = 0; j<heaplist[i]->map_size; j++){
			MemAddr addr = (heaplist[i]->map_start) + j;
			heaplist[i]->driver->write(addr, 0);
		}
	}
}

uint8_t os_getHeapListLength(){
	// Return number of heaps
	return NUMBER_OF_HEAPS;
}

Heap* os_lookupHeap(uint8_t index){
	return heaplist[index];
}


// Reads the nibble at the index;
MemValue os_readNibble(const Heap *heap, uint16_t index){

	// For every 2 nibbles go up a byte
	uint16_t addr = heap->map_start + index/2;
	// Get the byte
	MemValue byte = heap->driver->read(addr);
	// Get the correct nibble
	// Nibbles  -> bytes
	//(0,1)     -> 0,1,2,3 ...
	//(2,3)
	if ((index%2)==1){
		// Read low half
		return (byte&0x0F);
		} else {
		// Read high half
		return (byte>>4);
	}
}


// Writes the nibble at the index (0th nibble 1st nibble, ...)
void os_writeNibble(const Heap *heap, uint16_t index, uint8_t nib){
	// Low half only (redundant)
	nib &= 0x0F;
	// For every 2 nibbles go up a nibble-byte in the map
	uint16_t addr = heap->map_start + index/2;
	// Get the byte
	MemValue byte = heap->driver->read(addr);
	// Make the new nibble-byte
	// Nibbles  -> use-bytes
	//(0,1)     -> 0,1,2,3 ...
	//(2,3)
	if ((index%2)==1){
		// Edit the lower half
		byte &= (0xF0 | nib);
		byte |= (0x0F & nib);
		} else {
		// Edit the upper half
		nib = nib << 4;
		byte &= (0x0F | nib);
		byte |= (0xF0 & nib);
	}

	// Write back
	heap->driver->write(addr, byte);
}





