/**
 * @file main.h
 * @brief All the library inclusions for the mother of all header files.
 *
 * This file includes all the needed libraries, as it is included by every
 * other source code file / unit. Other than that, there is nothing special here.
 */

#ifndef MAIN_H
#define MAIN_H

// ------------------------------------------------------------------------------
// INCLUDES
// ------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "io.h"
#include "interpreter.h"
#include "printing.h"

// ------------------------------------------------------------------------------
// PROTOTYPES
// ------------------------------------------------------------------------------

int main (void);

// internal data handling
void uppercase (void);
void ignorespace (void);
void push_byte (uint8_t b);
uint8_t pop_byte (void);
int16_t str_to_num (uint8_t *strptr);
uint8_t *find_line (void);
void get_line (void);

uint8_t break_test (void);

uint16_t valid_filename_char (uint8_t c);
uint8_t * valid_filename (void);

void fx_delay_ms (uint16_t ms);
void fx_delay_us (uint16_t us);

// ------------------------------------------------------------------------------
// CONSTANTS
// ------------------------------------------------------------------------------

#define cfg_auto_run        1  // 1st bit
#define cfg_run_after_load  2  // 2nd bit
#define cfg_from_serial     4  // 3rd bit
#define cfg_from_eeprom     8  // 4th bit

// ------------------------------------------------------------------------------
// GLOBALS
// ------------------------------------------------------------------------------

uint8_t sys_config;

#endif
