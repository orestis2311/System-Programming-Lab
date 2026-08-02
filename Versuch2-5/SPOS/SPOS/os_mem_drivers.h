#ifndef OS_MEM_DRIVERS_H
#define OS_MEM_DRIVERS_H

#include <stdint.h>
#include "atmega644constants.h"
#include "os_mem_drivers.h"
#include "os_core.h"
#include "defines.h"
#include "os_spi.h"

// Define MemAddr and MemValue types
typedef uint16_t MemAddr;
typedef uint8_t MemValue;

#define intSRAM (&intSRAM__)
#define extSRAM (&extSRAM__)

// Define the MemDriver structure
typedef struct MemDriver {
	void (*init)(void);                        // Initialization function
	MemValue (*read)(MemAddr addr);           // Read function
	void (*write)(MemAddr addr, MemValue val); // Write function

	const MemAddr start_addr;  // Start address of the memory
	const uint16_t size;       // Size of the memory
} MemDriver;

MemDriver intSRAM__;
MemDriver extSRAM__;

#endif // OS_MEM_DRIVERS_H
