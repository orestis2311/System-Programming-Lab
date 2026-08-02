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

//#define extSRAM (&extSRAM__)

// Define the MemDriver structure
typedef struct MemDriver {
	void (*init)(void);                        // Initialization function
	MemValue (*read)(MemAddr addr);           // Read function
	void (*write)(MemAddr addr, MemValue val); // Write function

	const MemAddr start_addr;  // Start address of the memory
	const uint16_t size;       // Size of the memory
} MemDriver;



//---------- internal SRAM
static void sram_init(void) {
}

static MemValue sram_read(MemAddr addr) {
	if (!(addr >= AVR_SRAM_START && addr <= AVR_SRAM_LAST)) {
		os_error("sram_read ooB");
		return 0; // Invalid address
	}
	return *((MemValue*)addr);
}

static void sram_write(MemAddr addr, MemValue value) {
	if (!(addr >= AVR_SRAM_START && addr <= AVR_SRAM_LAST)) {
		os_error("sram_write ooB");
		return;
	}
	*((MemValue*)addr) = value;
}


MemDriver intSRAM__ = {
	.init = sram_init,
	.read = sram_read,
	.write = sram_write,
	.start_addr = AVR_SRAM_START,
	.size = AVR_MEMORY_SRAM
};

/*
//---------- external SRAM
static void extsram_init(void) {
	os_spi_init();
}

static MemValue extsram_read(MemAddr addr) {
	os_spi_start_transmission();
	os_spi_send(CREAD);
	os_spi_send(0);
	os_spi_send((uint8_t)(addr>>8));
	os_spi_send((uint8_t) ((addr)&0x00FF));
	MemValue ret = os_spi_receive();
	os_spi_end_transmission();
	return ret;

}

static void extsram_write(MemAddr addr, MemValue value) {
	os_spi_start_transmission();
	os_spi_send(CWRITE);
	os_spi_send(0);
	os_spi_send((uint8_t)(addr>>8));
	os_spi_send((uint8_t) ((addr)&0x00FF));
	os_spi_send(value);
	os_spi_end_transmission();
}


MemDriver extSRAM__ = {
	.init = extsram_init,
	.read = extsram_read,
	.write = extsram_write,
	.start_addr = 0,
	// Kinda, one more actually, but wth
	.size = UINT16_MAX
};
*/
#endif // OS_MEM_DRIVERS_H