/**
 * @file printing.h
 * @brief Prototypes for printing functions.
*/

#ifndef PRINTING_H
#define PRINTING_H

// ------------------------------------------------------------------------------
// INCLUDES
// ------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "main.h"

// ------------------------------------------------------------------------------
// PROTOTYPES
// ------------------------------------------------------------------------------

void printnum (int16_t num, FILE *stream);
void printmsg_noNL (const uint8_t *msg, FILE *stream);
void printmsg (const uint8_t *msg, FILE *stream);
void printline (uint8_t **line, FILE *stream);
void newline (FILE *stream);
uint8_t print_string (void);

// ------------------------------------------------------------------------------
// CONSTANTS
// ------------------------------------------------------------------------------

#define MAXCPL 32
#define TXT_COL_DEFAULT 76
#define TXT_COL_ERROR 3

#endif
