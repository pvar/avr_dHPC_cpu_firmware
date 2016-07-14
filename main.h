#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#include "io.h"
#include "interpreter.h"

/**
 ** prototypes of non-static functions
 **
 **/

int main (void);

// message and data printing
void do_beep (void);
void printnum (int16_t num, FILE *stream);
void printmsg_noNL (const uint8_t *msg, FILE *stream);
void printmsg (const uint8_t *msg, FILE *stream);
void printline (FILE *stream);
void newline (FILE *stream);
uint8_t print_string (void);

// internal data handling
void uppercase (void);
void ignorespace (void);
void get_line (void);
void push_byte (uint8_t b);
uint8_t pop_byte (void);
uint8_t *find_line (void);
int16_t str_to_num (uint8_t *strptr);

uint8_t break_test (void);

// file-access related
uint16_t valid_filename_char (uint8_t c);
uint8_t * valid_filename (void);

// delay functions
void fx_delay_ms (uint16_t ms);
void fx_delay_us (uint16_t us);

/**
 ** definition of constants and custom data types
 **
 **/

#define MAXCPL 32
#define TXT_COL_DEFAULT 76
#define TXT_COL_ERROR 3
#define true 1
#define false 0

#define cfg_auto_run 		1   // 1st bit
#define cfg_run_after_load	2   // 2nd bit
#define cfg_from_sdcard		4   // 3rd bit
#define cfg_from_eeprom 	8   // 4th bit

/**
 ** declarations of global variables
 **
 **/

uint8_t main_config;

#endif