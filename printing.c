/*
 * String and message printing functions for nstBASIC.
 *
 * Copyright 2016, Panagiotis Varelas <varelaspanos@gmail.com>
 *
 * nstBASIC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * nstBASIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

/**
 * @file printing.c
 * @brief Functions that handle communication with peripherals.
*/

#include "printing.h"


/** ---------------------------------------------------------------------------
 * @brief Print specified string
 *
 * ---------------------------------------------------------------------------- */
void printstr (char *str, FILE *stream)
{
    uint8_t i = 0;
    while (str[i] != 0) {
        fputc (str[i], stream);
        i++;
    }
}

/** ---------------------------------------------------------------------------
 * @brief Print specified integer number
 *
 * ---------------------------------------------------------------------------- */
void printnum (int16_t num, FILE *stream)
{
	int digits = 0;
	if (num < 0) {
		num = -num;
		fputc ('-', stream);
	}
	do {
		push_byte (num % 10 + '0');
		num = num / 10;
		digits++;
	} while (num > 0);

	while (digits > 0) {
		fputc (pop_byte(), stream);
		digits--;
	}
}

/** ---------------------------------------------------------------------------
 * @brief Print selected message from FLASH (without new-line)
 *
 * ---------------------------------------------------------------------------- */
void printmsg_noNL (const uint8_t *msg, FILE *stream)
{
	while (pgm_read_byte (msg) != 0)
        fputc (pgm_read_byte (msg++), stream);
}

/** ---------------------------------------------------------------------------
 * @brief Print selected message from FLASH (with new-line)
 *
 * ---------------------------------------------------------------------------- */
void printmsg (const uint8_t *msg, FILE *stream)
{
	printmsg_noNL (msg, stream);
	newline (stream);
}

/** ---------------------------------------------------------------------------
 * @brief Print a program line
 *
 * ---------------------------------------------------------------------------- */
void printline (FILE *stream)
{
	LINE_NUMBER line_num;
	line_num = * ((LINE_NUMBER *) (list_line));
	list_line += sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);
	// print line number
	printnum (line_num, stream);
	fputc (' ', stream);
	// print line content
	while (*list_line != LF) {
		fputc (*list_line, stream);
		list_line++;
	}
	list_line++;
	newline (stream);
}

/** ---------------------------------------------------------------------------
 * @brief Print a NEW LINE character
 *
 * ---------------------------------------------------------------------------- */
void newline (FILE *stream)
{
	fputc (LF, stream);
	fputc (CR, stream);
}

/** ---------------------------------------------------------------------------
 * @brief Print a user string (enclosed in quotes)
 *
 * ---------------------------------------------------------------------------- */
uint8_t print_string (void)
{
	uint16_t i = 0;
	uint8_t delim = *txtpos;
	// check for opening delimiter
	if (delim != '"' && delim != '\'')
        return 0;
	txtpos++;
	// check for closing delimiter
	while (txtpos[i] != delim) {
		if (txtpos[i] == LF)
            return 0;
		i++;
	}
	// print characters
	while (*txtpos != delim) {
		fputc (*txtpos, stdout);
		txtpos++;
	}
	txtpos++; // skip closing
	return 1;
}

