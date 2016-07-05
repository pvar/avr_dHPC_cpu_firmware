/*
 * Firmware for the CPU of a homebrew computer.
 * Main software component is nstBASIC -- a parser for BASIC.
 *
 * Copyright 2016 Panos Varelas <pvar@defiant.gr>
 *
 * Written for deltaHacker magazine <https://deltahacker.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#define F_CPU UINT32_C(20000000)
#define BAUD 57600

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#include "main.h"
#include "io.c"
#include "parser.c"

// ----------------------------------------------------------------------------
// main program
// ----------------------------------------------------------------------------
int main (void)
{
	init_io();
	do_beep();
	text_color (TXT_COL_DEFAULT);
	paper_color (0);
	printmsg (msg_welcome, stdout);
	// setup timer2 for seed generation
	TCCR2A = 0;
	TCCR2B = _BV (CS22) | _BV (CS21) | _BV (CS20);
	TIMSK2 = 0;
	while (1)
		basic_init();
	return 0;
}

// ----------------------------------------------------------------------------
// message and data printing
// ----------------------------------------------------------------------------
void do_beep (void)
{
	// disable keyboard interrupt
	EIMSK &= ~KEYBOARD_INT;
	uint8_t cnt = 100;
	while (cnt > 0) {
		aux_ctl_bus_out &= ~buzzer_led;
		fx_delay_us (750);
		aux_ctl_bus_out |= buzzer_led;
		fx_delay_us (500);
		cnt--;
	}
	// enable keyboard interrupt
	EIMSK |= KEYBOARD_INT;
}

void printstr (char *str, FILE *stream)
{
    uint8_t i = 0;
	while (str[i] != 0) {
        fputc (str[i], stream);
        i++;
    }
}

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

void printmsg_noNL (const uint8_t *msg, FILE *stream)
{
	while (pgm_read_byte (msg) != 0)
        fputc (pgm_read_byte (msg++), stream);
}

void printmsg (const uint8_t *msg, FILE *stream)
{
	printmsg_noNL (msg, stream);
	newline (stream);
}

void printline (FILE *stream)
{
	uint16_t line_num;
	line_num = * ((uint16_t *) (list_line));
	list_line += sizeof (uint16_t) + sizeof (uint8_t);
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

void newline (FILE *stream)
{
	fputc (LF, stream);
	fputc (CR, stream);
}

// ----------------------------------------------------------------------------
// internal data handling
// ----------------------------------------------------------------------------
void get_line (uint8_t prompt)
{
	txtpos = program_end + sizeof (uint16_t);
	maxpos = txtpos;
	uint8_t incoming_char;
	uint8_t temp1, temp2;
	if (main_config & cfg_from_eeprom) {	// get new line from eeprom
		while (1) {
			incoming_char = fgetc (&stream_eeprom);
			switch (incoming_char) {
			case 0:
				main_config &= ~cfg_from_eeprom;
				return;
			case LF:
			case CR:
				txtpos[0] = LF;
				return;
			default:
				txtpos[0] = incoming_char;
				txtpos++;
			}
		}
	} else {				// get new line from standard input
		while (1) {
			incoming_char = fgetc (stdin);
			switch (incoming_char) {
			case BELL:		// make a short sound
				do_beep();
				break;
			case FF:		// FORM-FEED or NEW-PAGE (CTRL+L)
				break;
			case HOME:		// HOME
				if (txtpos > program_end + 2) {
					putchar (vid_tosol);
					// how many lines the cursor has to go up
					temp1 = txtpos - program_end - 2;
					putchar (temp1 / MAXCPL + 1);
					txtpos = program_end + 2;
				}
				break;
			case END:		// END
				if (txtpos < maxpos) {
					putchar (vid_toeol);
					// how many lines the cursor has to go down
					temp1 = maxpos - program_end - 2;	// characters to last position
					temp2 = txtpos - program_end - 2;	// characters to current position
					temp2 = temp1 / MAXCPL - temp2 / MAXCPL;
					putchar (temp2 + 1);
					// ending column
					temp2 = temp1 - (temp1 / MAXCPL) * MAXCPL;
					putchar (temp2);
					txtpos = maxpos;
				}
				break;
			case ARLT:		// ARROW LEFT
				if (txtpos <= program_end + 2) do_beep();
				else {
					putchar (vid_tolft);
					txtpos--;
				}
				break;
			case ARRT:		// ARROW RIGHT
				if (txtpos < maxpos) {
					txtpos++;
					putchar (vid_torgt);
				} else do_beep();
				break;
			///////////////////////////////////////////////////////
			case ARUP:		// ARROW UP
			case ARDN:		// ARROW DOWN
				break;
			///////////////////////////////////////////////////////
			case LF:
			case CR:
				// lines are always terminated with LF
				txtpos = maxpos;
				txtpos[0] = LF;
				newline (stdout);
				return;
			case BS:
				if (txtpos <= program_end + 2) do_beep();
				else {
					putchar (BS);
					txtpos--;
				}
				break;
			default:
				// need at least one space to allow shuffling the lines
				if (txtpos == variables_begin - 2) do_beep();
				else {
					putchar (incoming_char);
					txtpos[0] = incoming_char;
					txtpos++;
					if (txtpos > maxpos) maxpos = txtpos;
				}
			}
		}
	}
}

void push_byte (uint8_t b)
{
	sp--;
	*sp = b;
}

unsigned char pop_byte (void)
{
	uint8_t b;
	b = *sp;
	sp++;
	return b;
}

unsigned char *find_line (void)
{
	uint8_t *line = program_start;
	while (1) {
		if (line == program_end) return line;
		if (((uint16_t *) line)[0] >= linenum) return line;
		// Add the line length onto the current address, to get to the next line;
		line += line[ sizeof (uint16_t) ];
	}
}

// ----------------------------------------------------------------------------
// normalization (kind of)
// ----------------------------------------------------------------------------
void ignorespace (void)
{
	while (*txtpos == SPACE || *txtpos == TAB)
		txtpos++;
}

void uppercase (void)
{
	uint8_t *c = program_end + sizeof (uint16_t);
	uint8_t quote = 0;
	while (*c != LF) {
		// are we in a quoted string?
		if (*c == quote) quote = 0;
		else if (*c == DQUOTE || *c == SQUOTE) quote = *c;
		else if (quote == 0 && *c >= 'a' && *c <= 'z') *c = *c + 'A' - 'a';
		c++;
	}
}

// ----------------------------------------------------------------------------
// file-access related
// ----------------------------------------------------------------------------

// return 1 if the character is acceptable for a file name
uint16_t isValidFnChar (uint8_t c)
{
	if ((c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c == '_')
		|| (c == '+')
		|| (c == '.')
		|| (c == '~'))
		return 1;
	else
		return 0;
}

uint8_t * filenameWord (void)
{
	// SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
	uint8_t * ret = txtpos;
	error_code = 0;
	// make sure there are no quotes or spaces, search for valid characters
	// while (*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE) txtpos++;
	while (!isValidFnChar (*txtpos))
		txtpos++;
	ret = txtpos;
	if (*ret == '\0') {
		error_code = 1;
		return ret;
	}
	// now, find the next nonfnchar
	txtpos++;
	while (isValidFnChar (*txtpos))
		txtpos++;
	if (txtpos != ret)
		*txtpos = '\0';
	// set the error code if we've got no string
	if (*ret == '\0')
		error_code = 1;
	return ret;
}

// ----------------------------------------------------------------------------
// various
// ----------------------------------------------------------------------------
void fx_delay_ms (uint16_t ms)
{
	while (ms > 0) {
		_delay_ms (1);
		ms--;
	}
}

void fx_delay_us (uint16_t us)
{
	while (us > 0) {
		_delay_us (1);
		us--;
	}
}

// check if this a line-number
uint16_t linenum_test (void)
{
	uint16_t num = 0;
	ignorespace();
	while (*txtpos >= '0' && *txtpos <= '9') {
		// Trap overflows
		if (num >= 0xFFFF / 10) {
			num = 0xFFFF;
			break;
		}
		num = num * 10 + *txtpos - '0';
		txtpos++;
	}
	return	num;
}

// check for attempted break
uint8_t break_test (void)
{
	if (break_flow || UDR0 == ETX) {
		break_flow = 0;
		return 1;
	}
	return 0;
}

// transform string representation to numeric
int16_t str_to_num (uint8_t *strptr)
{
	uint8_t negative = 0;
	int16_t value = 0;
	// check for minus sign
	if (*strptr == '-') {
		strptr++;
		negative = 1;
	}
	// calculate value of given number
	while (*strptr >= '0' && *strptr <= '9') {
		value = 10 * value + (*strptr - '0');
		strptr++;
	}
	if (negative)
		return -value;
	else
		return value;
}
