#include "os_spi.h"
#include "os_scheduler.h"

uint8_t inited = 0;

void os_spi_lower_cs(void){
	// Cause a high-to-low-transition
	PORTB |= (1 << PB4);  // NotCS = 1
	PORTB &= ~(1 << PB4);  // NotCS = 0
}

void os_spi_raise_cs(void){
	PORTB |= (1 << PB4);
}

void os_spi_init(void){
	// BusIOPins, become Master, external SRAM(exR) set SPI-Byte-Mode
	DDRB |= (1 << PB4);   // Set Pin B4 (CS) as output
	DDRB |= (1 << PB5);	  // Set Pin B5 (MOSI) as output
	DDRB &= ~(1 << PB6);	  // Set Pin B6 (MISO) as input
	PORTB |= (1 << PB6);  // Enable, pullup
	DDRB |= (1 << PB7);	  // Set Pin B7 (CLK) as output
	uint8_t check = DDRB;
	inited = 1;
	
	//uint8_t spcr_conf = (SPE_V << SPE)|(DORD_V << DORD)|(1 << MSTR)|(CPOL_V << CPOL)|(CPHA_V << CPHA)|(SPR1_V << SPR1)|(SPR0_V << SPR0);
	//uint8_t check = spcr_conf;
	//os_error("here");
	
	//SPCR |= ((~(1 << SPIE)) & spcr_conf);
	//SPCR &= ((1 << SPIE) | spcr_conf);
	SPCR = 0x50;
	check = SPCR;
	
	SPSR |= (SPI2X_V << SPI2X);
	
	check = SPSR;
	if (check == 0){
		os_error("CHECK");
	}

	os_spi_start_transmission();

	// Set Byte mode
	os_spi_send(CSETMODE);
	os_spi_send(0);

	os_spi_end_transmission();
}

void os_spi_start_transmission(void){
	os_enterCriticalSection();
	os_spi_lower_cs();
}

void os_spi_end_transmission(void){
	os_spi_raise_cs();
	os_leaveCriticalSection();
}

void os_spi_send(uint8_t byte){
	if (inited == 0){
		os_error("Not inited");
	}
	SPDR = byte;
	while ((SPSR & (1 << SPIF)) == 0);
}

uint8_t os_spi_receive(){
	SPDR = DUMMY;
	while ((SPSR & (1 << SPIF)) == 0);

	volatile uint8_t res = SPDR;
	
	return res;
}

