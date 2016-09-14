/**
 * @file io.h
 * @brief Functions that handle communication with peripherals.
 */

#ifndef PRINTING_H
#define PRINTING_H

#include <stdio.h>
#include <stdlib.h>

#include "main.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

void printnum (int16_t num, FILE *stream);
void printmsg_noNL (const uint8_t *msg, FILE *stream);
void printmsg (const uint8_t *msg, FILE *stream);
void printline (FILE *stream);
void newline (FILE *stream);
uint8_t print_string (void);

// =============================================================================
// CONSTANTS AND CUSTOM DATA TYPES
// =============================================================================



// ============================================================================
// GLOBAL VARIABLES
// =============================================================================



// ============================================================================
// ASSEMBLER MACROS
// =============================================================================


#endif