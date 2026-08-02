// TODO Add signatures
#ifndef OS_MEMORY_H
#define OS_MEMORY_H
#include "os_memheap_drivers.h"
#include "defines.h"
#include "os_process.h"

void os_adjust_alloc_bounds(ProcessID pid, MemAddr index);
void os_mem_move(Heap* heap, MemAddr old_index, MemAddr new_index, uint16_t size);

MemAddr os_malloc(Heap* heap, uint16_t size);
MemAddr os_realloc(Heap* heap, MemAddr addr, uint16_t size);

void os_free(Heap* heap, MemAddr addr);
void os_free_force(Heap* heap, MemAddr addr);


MemAddr os_sh_malloc (Heap *heap, size_t size);
void os_sh_free (Heap *heap, MemAddr *ptr);

void os_sh_write (const Heap *heap, const MemAddr *ptr, uint16_t offset, const MemValue *dataSrc, uint16_t length);
void os_sh_read (const Heap *heap, const MemAddr *ptr, uint16_t offset, MemValue *dataDest, uint16_t length);
MemAddr os_sh_readOpen (const Heap *heap, const MemAddr *ptr);
MemAddr os_sh_writeOpen (const Heap *heap, const MemAddr *ptr);
void os_sh_close (const Heap *heap, MemAddr addr);

size_t os_getMapSize(Heap const* heap);
size_t os_getUseSize(Heap const* heap);
MemAddr os_getMapStart(Heap const* heap);
MemAddr os_getUseStart(Heap const* heap);

uint16_t os_getChunkSize(Heap const* heap, MemAddr addr);

void os_freeProcessMemory(Heap* heap, ProcessID id);

AllocStrategy os_getAllocationStrategy(Heap const* heap);
void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat);
#endif
