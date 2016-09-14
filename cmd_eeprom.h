/**
 * @file cmd_eeprom.h
 * @brief Functions that erase (format) the whole EEPROM space,
 * save and load programs, or load-AND-run (chain) a program.
 */

#ifndef CMD_EEPROM_H
#define CMD_EEPROM_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint8_t elist (void);
uint8_t eformat (void);
uint8_t eload (void);
uint8_t esave (void);

#endif