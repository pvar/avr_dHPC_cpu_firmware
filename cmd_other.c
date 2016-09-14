/*
 * Implementation of various (uncategorized) commands of nstBASIC.
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
 * @file cmd_other.c
 * @brief Functions for program execution and other common tasks,
 * like assinging values to variables, pritining program list and so on.
 */

#include "cmd_other.h"

int8_t input (void)
{
    uint8_t chr = 0;
    uint8_t cnt = 0;
    int16_t *var;
    uint8_t *input_buffer_ptr;

    // variable to store user value
    ignorespace();
    if (*txtpos < 'A' || *txtpos > 'Z') {
        error_code = 0x7;
        return POST_CMD_WARM_RESET;
    }
    var = (int16_t *)variables_begin + *txtpos - 'A';
    // check for proper statement termination
    txtpos++;
    ignorespace();
    if (*txtpos != LF && *txtpos != ':') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    // get user value (accept only digits)
    input_buffer_ptr = input_buffer;
    *input_buffer_ptr = 0;
    cnt = 0;
    EIMSK |= BREAK_INT; //enable emergency break key (INT2)
    while (chr != LF && chr != CR) {
        chr = fgetc (stdin);
        if (break_flow == 1)
            return POST_CMD_NEXT_STATEMENT;
        switch (chr) {
            case '-':
                // only accept minus sign as first character
                if (cnt == 0) {
                    putchar (chr);
                    *input_buffer_ptr = chr;
                    cnt++;
                    input_buffer_ptr++;
                    *input_buffer_ptr = 0;
                }
                break;
            case LF:
            case CR:
                *input_buffer_ptr = LF;
                break;
            case BS:
                if (input_buffer_ptr <= input_buffer)
                    do_beep();
                else {
                    putchar (BS);
                    input_buffer_ptr--;
                    cnt--;
                }
                break;
            default:
                if (chr >= '0' && chr <= '9') {
                    if (cnt < INPUT_BUFFER_SIZE) {
                        putchar (chr);
                        *input_buffer_ptr = chr;
                        cnt++;
                        input_buffer_ptr++;
                        // mark last character with ASCII code 0
                        *input_buffer_ptr = 0;
                    } else
                        do_beep();
                }
        }
    }
    newline (stdout);
    // parse and store user value
    *var = str_to_num (input_buffer);
	return POST_CMD_NEXT_STATEMENT;
}

int8_t assignment (void)
{
    int16_t value, *var;
    // check if invalid character (non-letter)
    if (*txtpos < 'A' || *txtpos > 'Z') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    var = (int16_t *)variables_begin + *txtpos - 'A';
    txtpos++;

    // check for invalid variable name (more than one letters)
    if (*txtpos >= 'A' && *txtpos <= 'Z') {
        error_code = 0xFF;
        txtpos++;
        /* quick fix -- does it work? */
        return POST_CMD_WARM_RESET;
    }

    // check for missing assignment operator
    ignorespace();
    if (*txtpos != '=') {
        error_code = 0xF;
        return POST_CMD_WARM_RESET;
    } else {
        // check if variable name is rendered invalid
        if (error_code == 0xFF) {
            error_code = 0x11;
            return POST_CMD_WARM_RESET;
        }
    }
    txtpos++;
    ignorespace();
    value = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    // Check that we are at the end of the statement
    if (*txtpos != LF && *txtpos != ':') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    *var = value;
	return POST_CMD_NEXT_STATEMENT;
}

int8_t poke (void)
{
    int16_t value, address;
    // get the address
    address = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (address > MEMORY_SIZE) {
        error_code = 0x13;
        return POST_CMD_WARM_RESET;
    }
    // check for comma
    ignorespace();
    if (*txtpos != ',') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    txtpos++;
    // get the value to assign
    ignorespace();
    value = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (value < 0 || value > 255) {
        error_code = 0x12;
        return POST_CMD_WARM_RESET;
    }
    // check for proper statement termination
    if (*txtpos != LF && *txtpos != ':') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    // assign value to specified location in memory
    program[address] = value;
	return POST_CMD_NEXT_STATEMENT;
}

int8_t list (void)
{
	linenum = get_linenumber(); // returns 0 if no line found
	// should be EOL
	if (txtpos[0] != LF) {
		error_code = 0x4;
        return POST_CMD_WARM_RESET;
	}
	// find the line
	list_line = find_line();
	while (list_line != program_end)
        printline (stdout);
    return POST_CMD_WARM_RESET;
}

int8_t mem (void)
{
    // SRAM size
    printnum (variables_begin - program_end, stdout);
    printmsg (msg_ram_bytes, stdout);
    // EEPROM size
    printnum (E2END + 1, stdout);
    printmsg (msg_rom_bytes, stdout);
    // EEPROM usage
    //uint8_t val = 127;
    //uint16_t i;
    //for( i = 0; ( i < ( E2END + 1 ) ) && ( val != '\0' ); i++ )
    //    val = eeprom_read_byte( (uint8_t *)i );
    //printnum( ( E2END + 1 ) - ( i - 1 ), stdout );
    //printmsg( msg_available, stdout );
	return POST_CMD_NEXT_STATEMENT;
}

int8_t randomize (void)
{
	srand (TCNT2);
	return POST_CMD_NEXT_STATEMENT;
}

int8_t rndseed (void)
{
    uint16_t param;
    error_code = 0;
    // get seed for PRNG
    param = (uint16_t)parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    srand (param);
	return POST_CMD_NEXT_STATEMENT;
}

int8_t prog_run (void)
{
    //enable emergency break key (INT2)
    EIMSK |= BREAK_INT;
    // disable cursor
    putchar (vid_cursor_off);
    // disable auto scroll
    putchar (vid_scroll_off);
    current_line = program_start;
    return POST_CMD_EXEC_LINE;
}

int8_t prog_end (void)
{
    // should be at end of line
    if (txtpos[0] != LF) {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    // set current line at the end of program
    current_line = program_end;
    return POST_CMD_EXEC_LINE;
}

int8_t prog_new (void)
{
    if (txtpos[0] != LF) {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    program_end = program_start;
    return POST_CMD_PROMPT;
}