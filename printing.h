/**
 * @file printing.h
 * @brief Prototypes, macros, enumerators and global variables...
 * The three macros defined in this file determine the text colour for 'normal' and 'error'
 * messages, as well as the length of a text line on the screen. When porting nstBASIC to
 * other platforms, they should probably have to be be updated.
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
void printline (uint8_t *line, FILE *stream);
void newline (FILE *stream);
uint8_t print_string (void);

// ------------------------------------------------------------------------------
// MACROS
// ------------------------------------------------------------------------------

#define MAXCPL 32
#define TXT_COL_DEFAULT 76
#define TXT_COL_ERROR 3

#endif
