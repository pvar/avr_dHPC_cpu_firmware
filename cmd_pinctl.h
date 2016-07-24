/**
 * @file cmd_pinctl.h
 * @brief Functions that can set or get the state of a digital IO pin.
 */

#ifndef CMD_PINCTL_H
#define CMD_PINCTL_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint8_t pindwrite (void);
uint8_t pindir (void);

#endif