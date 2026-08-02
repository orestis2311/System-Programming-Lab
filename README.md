# Embedded Operating System for ATmega644

A small embedded operating system developed in C for the ATmega644 microcontroller as part of the System Programming Lab at RWTH Aachen University.

The project implements core operating-system concepts such as process management, interrupt-driven scheduling, context switching, critical sections, dynamic memory management, and low-level hardware communication.

## Features

### Process Management

- Process creation and termination
- Process states and priorities
- Interrupt-driven context switching
- Cooperative process yielding
- Critical-section handling

### Scheduling

The system supports multiple scheduling strategies, including:

- Even scheduling
- Round-robin scheduling
- Inactive aging
- Run-to-completion
- Multilevel feedback queue scheduling

### Memory Management

- Dynamic allocation and deallocation
- Internal and external SRAM support
- Memory reallocation
- Shared-memory operations
- Heap management
- Multiple allocation strategies:
  - First fit
  - Next fit
  - Best fit
  - Worst fit

### Hardware Integration

- SPI communication
- LCD output
- LED-panel control
- Joystick input
- Analogue-to-digital conversion
- Timer and interrupt handling

### Demonstration Application

The completed system was used to run an interactive Snake game on an LED panel, demonstrating process execution, input handling, display control, and hardware communication.

## Technologies

- C
- AVR
- ATmega644
- AVR-GCC
- Microchip Studio / Atmel Studio
- Make
- Git

## Project Structure

```text
PSP/
├── Versuch1/
│   ├── Versuch_1_ADC/
│   └── Versuch_1_ADDA/
│
└── Versuch2-5/
    └── SPOS/
        ├── Makefile
        └── SPOS/
            ├── main.c
            ├── os_core.c
            ├── os_scheduler.c
            ├── os_scheduling_strategies.c
            ├── os_memory.c
            ├── os_memory_strategies.c
            ├── os_mem_drivers.c
            ├── os_memheap_drivers.c
            ├── os_spi.c
            ├── os_input.c
            ├── lcd.c
            ├── progs.c
            └── ...
