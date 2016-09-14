/*
 * Fundamental components of nstBASIC.
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
 * @file main.c
 * @brief Functions that perform data preprocessing and simple sanity checks,
 * insert busy-delays and print messages.
 */

#include "main.h"

/** ---------------------------------------------------------------------------
 * @brief Main function
 * ---------------------------------------------------------------------------- */
int main (void)
{
    init_io();
    text_color (TXT_COL_DEFAULT);
    paper_color (0);
    printmsg (msg_welcome, stdout);
    do_beep();

    // setup timer2 for seed generation
    TCCR2A = 0;
    TCCR2B = _BV (CS22) | _BV (CS21) | _BV (CS20);
    TIMSK2 = 0;

    //while (1)
    basic_init();
    interpreter();
	return 0;
}

/** ---------------------------------------------------------------------------
 * @brief Get a new line from EEPROM or STDIO
 * ---------------------------------------------------------------------------- */
void get_line (void)
{
	uint8_t *maxpos; 

	txtpos = program_end + sizeof (uint16_t);
    maxpos = txtpos;
	uint8_t incoming_char;
	uint8_t temp1, temp2;
    // READ FROM EEPROM
	if (main_config & cfg_from_eeprom) {
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
    // READ FROM STANDARD INPUT
    } else {
		while (1) {
			incoming_char = fgetc (stdin); // use &stream_pseudo to read from serial
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

/** ---------------------------------------------------------------------------
 * @brief Push a byte in the STACK
 * ---------------------------------------------------------------------------- */
void push_byte (uint8_t byte_to_push)
{
	stack_ptr--;
	*stack_ptr = byte_to_push;
}

/** ---------------------------------------------------------------------------
 * @brief Pop a byte from the STACK
 * ---------------------------------------------------------------------------- */
uint8_t pop_byte (void)
{
	uint8_t byte_to_pop;
	byte_to_pop = *stack_ptr;
	stack_ptr++;
	return byte_to_pop;
}

/* ----------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------- */
uint8_t *find_line (void)
{
	uint8_t *line = program_start;
	while (1) {
		if (line == program_end)
            return line;
		if ( ((uint16_t *)line)[0] >= linenum)
            return line;
		// add line length to current address :: proceed to next line
		line += line[ sizeof (uint16_t) ];
	}
}

/** ---------------------------------------------------------------------------
 * @brief Normalize user input
 * Ignore spaces and transform code (not strings) to upper case
 * ---------------------------------------------------------------------------- */
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

/** ---------------------------------------------------------------------------
 * @brief Check is suitable for name
 * Check if specified character can be used for a filename
 * ---------------------------------------------------------------------------- */
uint16_t valid_filename_char (uint8_t c)
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

/* ----------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------- */
uint8_t * valid_filename (void)
{
	// SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
	uint8_t * ret = txtpos;
	error_code = 0;
	// make sure there are no quotes or spaces, search for valid characters
	// while (*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE) txtpos++;
	while (!valid_filename_char (*txtpos))
		txtpos++;
	ret = txtpos;
	if (*ret == '\0') {
		error_code = 1;
		return ret;
	}
	// find the next invalid char
	txtpos++;
	while (valid_filename_char (*txtpos))
		txtpos++;
	if (txtpos != ret)
		*txtpos = '\0';
	// set error code if no string
	if (*ret == '\0')
		error_code = 1;
	return ret;
}

/** ---------------------------------------------------------------------------
 * @brief Busy-delay functions
 * ---------------------------------------------------------------------------- */
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

/** ---------------------------------------------------------------------------
 * @brief Check if BREAK button was pressed
 * ---------------------------------------------------------------------------- */
uint8_t break_test (void)
{
	if (break_flow || UDR0 == ETX) {
		break_flow = 0;
		return 1;
	}
	return 0;
}

/** ---------------------------------------------------------------------------
 * @brief Transform string representation to numeric
 * ---------------------------------------------------------------------------- */
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
