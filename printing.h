/**
 * @file printing.h
 * @brief Settings regardig printing and the necessary function prototypes.
 *
 * The three macros defined in this file determine the text colour for 'normal' and 'error'
 * messages, as well as the length of a text line on the screen.
*/

#ifndef PRINTING_H
#define PRINTING_H

// ------------------------------------------------------------------------------
// INCLUDES
// ------------------------------------------------------------------------------

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
void debug_print (uint8_t chr);

// ------------------------------------------------------------------------------
// MACROS
// ------------------------------------------------------------------------------

#define TXT_COL_DEFAULT 76
#define TXT_COL_ERROR 3
#define MAXCPL 32

#endif
