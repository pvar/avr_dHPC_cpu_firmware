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
 * @brief Functions that would fit in no other file :)
 */

#include "main.h"

/** ***************************************************************************
 * @brief Program entry point -- the main function.
 *****************************************************************************/
int main (void)
{
        init_io();
        text_color (TXT_COL_DEFAULT);
        paper_color (0);
        printmsg (msg_welcome, stdout);
        do_beep();

        // configure timer2 (used for seed generation)
        TCCR2A = 0;
        TCCR2B = _BV (CS22) | _BV (CS21) | _BV (CS20);
        TIMSK2 = 0;

        basic_init();
        interpreter();

        return 0;
}

/** ***************************************************************************
 * @brief Read from SERIAL or EEPROM.
 *
 * Keeps reading until a NULL character is received.
 *****************************************************************************/

void read_serial_eeprom (FILE input_stream) {
        uint8_t in_chr;
        while (1) {
                in_chr = fgetc (&input_stream);
                switch (in_chr) {
                        // NULL -> stop reading
                        case 0:
                                sys_config &= ~cfg_from_serial;
                                sys_config &= ~cfg_from_eeprom;
                                return;
                        // LINE FEED or CARRIAGE RETURN -> LINE FEED
                        case LF:
                        case CR:
                                text_ptr[0] = LF;
                                return;
                        // OTHER CHARACTERS -> PUT IN BUFFER
                        default:
                                text_ptr[0] = in_chr;
                                text_ptr++;
                }
        }
}

/** ***************************************************************************
 * @brief Get line(s) from EEPROM, SERIAL or STDIO.
 *****************************************************************************/
void get_line (void)
{
        text_ptr = prog_end_ptr + sizeof (uint16_t);

        uint8_t *maxpos = text_ptr;
        uint8_t in_char, temp1, temp2;

        /* READ FROM EEPROM */
        if (sys_config & cfg_from_eeprom) {
                read_serial_eeprom (stream_eeprom);

        /* READ FROM SERIAL */
        } else if (sys_config & cfg_from_serial) {
                read_serial_eeprom (stream_pseudo);

        /* READ FROM STDIN */
        } else {
                while (1) {
                        in_char = fgetc (stdin);
                        switch (in_char) {
                                // BELL
                                case BELL:
                                        do_beep();
                                        break;
                                // FORM FEED or NEW PAGE (CTRL+L)
                                case FF:
                                        break;
                                // HOME
                                case HOME:
                                        if (text_ptr > prog_end_ptr + 2) {
                                                putchar (vid_tosol);
                                                // calculate lines to move the cursor up
                                                temp1 = text_ptr - prog_end_ptr - 2;
                                                putchar (temp1 / MAXCPL + 1);
                                                text_ptr = prog_end_ptr + 2;
                                        }
                                        break;
                                // END
                                case END:
                                        if (text_ptr < maxpos) {
                                                putchar (vid_toeol);
                                                // chars to end of line
                                                temp1 = maxpos - prog_end_ptr - 2;
                                                // chars from start of line
                                                temp2 = text_ptr - prog_end_ptr - 2;
                                                // calculate line-ending row
                                                temp2 = temp1 / MAXCPL - temp2 / MAXCPL;
                                                putchar (temp2 + 1);
                                                // calculate line-ending column
                                                temp2 = temp1 - (temp1 / MAXCPL) * MAXCPL;
                                                putchar (temp2);
                                                text_ptr = maxpos;
                                        }
                                        break;
                                // ARROW LEFT
                                case ARLT:
                                        if (text_ptr <= prog_end_ptr + 2)
                                                do_beep();
                                        else {
                                                putchar (vid_tolft);
                                                text_ptr--;
                                        }
                                        break;
                                // ARROW RIGHT
                                case ARRT:
                                        if (text_ptr < maxpos) {
                                                text_ptr++;
                                                putchar (vid_torgt);
                                        } else do_beep();
                                        break;
                                // ARROW UP
                                case ARUP:
                                        break;
                                // ARROW DOWN
                                case ARDN:
                                        break;
                                // LINE FEED or CARRIAGE RETURN
                                case LF:
                                case CR:
                                        // use LF for newline (like Unix)
                                        text_ptr = maxpos;
                                        text_ptr[0] = LF;
                                        newline (stdout);
                                        return;
                                // BACK SPACE
                                case BS:
                                        if (text_ptr <= prog_end_ptr + 2)
                                                do_beep();
                                        else {
                                                putchar (BS);
                                                text_ptr--;
                                        }
                                        break;
                                // OTHER CHARACTERS
                                default:
                                        if (text_ptr == variables_ptr - 2)
                                                do_beep();
                                        else {
                                                putchar (in_char);
                                                text_ptr[0] = in_char;
                                                text_ptr++;
                                                if (text_ptr > maxpos)
                                                        maxpos = text_ptr;
                                        }
                        }
                }
        }
}

/** ***************************************************************************
 * @brief Push a byte in the STACK.
 *****************************************************************************/
void push_byte (uint8_t byte_to_push)
{
        stack_ptr--;
        *stack_ptr = byte_to_push;
}

/** ***************************************************************************
 * @brief Pop a byte from the STACK.
 *****************************************************************************/
uint8_t pop_byte (void)
{
        uint8_t byte_to_pop;
        byte_to_pop = *stack_ptr;
        stack_ptr++;
        return byte_to_pop;
}

/** ***************************************************************************
 *
 *****************************************************************************/
uint8_t *find_line (void)
{
        uint8_t *line = program_space;
        while (1) {
                if (line == prog_end_ptr)
                        return line;
                if ( ((uint16_t *)line)[0] >= linenum)
                        return line;
                // add line's lenght (this value is stored exactly after line number0
                line += line[ sizeof (LINE_NUMBER) ];
        }
}

/** ***************************************************************************
 * @brief Normalize user input.
 * Ignore spaces and transform code (not strings) to upper case.
 *****************************************************************************/
void ignorespace (void)
{
        while (*text_ptr == SPACE || *text_ptr == TAB)
                text_ptr++;
}

void uppercase (void)
{
        uint8_t quote = 0;
        uint8_t *chr = prog_end_ptr + sizeof (uint16_t);

        while (*chr != LF) {
                // current character == stored quote
                if (*chr == quote)
                        quote = 0;

                // current character != stored quote
                // current character == some quote
                else if (*chr == DQUOTE || *chr == SQUOTE)
                        quote = *chr;

                // current character != stored quote
                // current character != some quote
                // stored quote == empty
                else if (quote == 0 && *chr >= 'a' && *chr <= 'z')
                        *chr = *chr + 'A' - 'a';
                chr++;
        }
}

/** ***************************************************************************
 * @brief Busy-delay functions.
 *****************************************************************************/
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

/** ***************************************************************************
 * @brief Check if BREAK button was pressed.
 *****************************************************************************/
uint8_t break_test (void)
{
        if (break_flow || UDR0 == ETX) {
                break_flow = 0;
                return 1;
        }
        return 0;
}

/** ***************************************************************************
 * @brief Transform string representation to numeric.
 *****************************************************************************/
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
