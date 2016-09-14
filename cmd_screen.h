/**
 * @file cmd_screen.h
 * @brief Functions that change pen/paper color, clear the screen,
 * move cursor to an arbitrary position and put characters or pixels.
 */

#ifndef CMD_SCREEN_H
#define CMD_SCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint8_t reset_display (void);
uint8_t clear_screen (void);
uint8_t pen (void);
uint8_t paper (void);
uint8_t print (void);
uint8_t locate (void);
uint8_t pset (void);
 
#endif