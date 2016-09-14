/*
 * Implementation of graphics related commands of nstBASIC.
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

#include "cmd_screen.h"

uint8_t reset_display (void)
{
	putchar (vid_reset);
	uart_ansi_rst_clr();
	printmsg (msg_welcome, stdout);
	program_end = program_start;
	return POST_CMD_PROMPT;
}

uint8_t clear_screen (void)
{
	putchar (vid_clear);
	uart_ansi_rst_clr();
	return POST_CMD_NEXT_STATEMENT;
}

uint8_t pen (void)
{
		uint16_t col;
		// get color value
		col = parse_expr_s1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		if (col < 0 || col > 127) {
			error_code = 0x14;
            return POST_CMD_WARM_RESET;
		}
		text_color ((uint8_t)col);
	return POST_CMD_NEXT_LINE;
}

uint8_t paper (void)
{
    uint16_t col;
    // get color value
    col = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (col < 0 || col > 127) {
        error_code = 0x14;
        return POST_CMD_WARM_RESET;
    }
    paper_color ((uint8_t)col);
	return POST_CMD_NEXT_LINE;
}

uint8_t locate (void)
{
    uint16_t line, column;
    // get target line
    line = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (line < 0 || line > 23) {
        error_code = 0x10;
        return POST_CMD_WARM_RESET;
    }
    // check for comma
    if (*txtpos != ',') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    txtpos++;
    // get target line
    column = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (column < 0 || column > 31) {
        error_code = 0x10;
        return POST_CMD_WARM_RESET;
    }
    locate_cursor (line, column);
	return POST_CMD_NEXT_LINE;
}

uint8_t print (void)
{
	// If we have an empty list then just put out a LF
	if (*txtpos == ':') {
		newline (stdout);
		txtpos++;
        return POST_CMD_NEXT_STATEMENT;
	}
	if (*txtpos == LF)
        return POST_CMD_NEXT_LINE;
	while (1) {
		ignorespace();
		if (print_string())
			;
		else if (*txtpos == '"' || *txtpos == '\'') {
			error_code = 0x4;
            return POST_CMD_WARM_RESET;
		} else {
			uint16_t e;
			error_code = 0;
			e = parse_expr_s1();
			if (error_code) {
                return POST_CMD_WARM_RESET;
            }
			printnum (e, stdout);
		}
		// at this point we have three options, a comma or a new line
		if (*txtpos == ',')
            txtpos++;   // skip the comma and move on
		else if (txtpos[0] == ';' && (txtpos[1] == LF || txtpos[1] == ':')) {
			txtpos++; // end of print without newline
			break;
		} else if (*txtpos == LF || *txtpos == ':') {
			newline (stdout);
			break;
		} else {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
    }
    return POST_CMD_NEXT_STATEMENT;
}

uint8_t pset (void)
{
    uint16_t x, y, col;
    // get x-coordinate
    x = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (x < 0 || x > 255) {
        error_code = 0x10;
        return POST_CMD_WARM_RESET;
    }
    // check for comma
    if (*txtpos != ',') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    txtpos++;
    // get y-coordinate
    y = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (y < 0 || y > 239) {
        error_code = 0x10;
        return POST_CMD_WARM_RESET;
    }
    // check for comma
    if (*txtpos != ',') {
        error_code = 0x2;
        return POST_CMD_WARM_RESET;
    }
    txtpos++;
    // get color
    col = parse_expr_s1();
    if (error_code) {
        return POST_CMD_WARM_RESET;
    }
    if (col < 0 || col > 127) {
        error_code = 0x14;
        return POST_CMD_WARM_RESET;
    }
    put_pixel ((uint8_t)x, (uint8_t)y, (uint8_t)col);
	return POST_CMD_NEXT_LINE;
}
