#ifndef OS_SPI_H
#define OS_SPI_H

#include <avr/io.h>
#include "defines.h"
#include "os_mem_drivers.h"


void os_spi_lower_cs(void);
void os_spi_raise_cs(void);

void os_spi_start_transmission(void);
void os_spi_end_transmission(void);

void os_spi_init(void);
void os_spi_send(uint8_t byte);
uint8_t os_spi_receive(void);


#endif