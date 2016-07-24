/**
 * @file cmd_flow.h
 * @brief Functions that implement FOR loops, unconditional jumps and
 * sub-routine calls (in nstBASIC).
 */

#ifndef CMD_FLOW_H
#define CMD_FLOW_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint8_t gotoline (void);
uint8_t check (void);
uint8_t loopfor (void);
uint8_t gosub (void);
uint8_t next (void);
uint8_t gosub_return (uint8_t cmd);

#endif