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

/// @cond CONST_MESSAGES

// general messages
const uint8_t msg_welcome[25]   PROGMEM = "Welcome to nstBASIC v0.2\0";
const uint8_t msg_ram_bytes[11] PROGMEM = " bytes RAM\0";
const uint8_t msg_rom_bytes[11] PROGMEM = " bytes ROM\0";
const uint8_t msg_available[17] PROGMEM = " bytes available\0";
const uint8_t msg_break[7]      PROGMEM = "Break!\0";
const uint8_t msg_ok[3]         PROGMEM = "OK\0";

// error messages
const uint8_t err_msgxl[6]  PROGMEM = "Left \0";
const uint8_t err_msgxr[7]  PROGMEM = "Right \0";
const uint8_t err_msg01[20] PROGMEM = "Not yet implemented\0";
const uint8_t err_msg02[13] PROGMEM = "Syntax error\0";
const uint8_t err_msg03[15] PROGMEM = "Stack overflow\0";
const uint8_t err_msg04[21] PROGMEM = "Unexpected character\0";
const uint8_t err_msg05[20] PROGMEM = "parenthesis missing\0";
const uint8_t err_msg07[18] PROGMEM = "Variable expected\0";
const uint8_t err_msg08[21] PROGMEM = "Jump point not found\0";
const uint8_t err_msg09[20] PROGMEM = "Invalid line number\0";
const uint8_t err_msg0A[18] PROGMEM = "Operator expected\0";
const uint8_t err_msg0B[17] PROGMEM = "Division by zero\0";
const uint8_t err_msg0C[19] PROGMEM = "Invalid pin [0..7]\0";
const uint8_t err_msg0D[14] PROGMEM = "Pin I/O error\0";
const uint8_t err_msg0E[17] PROGMEM = "Unknown function\0";
const uint8_t err_msg0F[16] PROGMEM = "Unknown command\0";
const uint8_t err_msg10[20] PROGMEM = "Invalid coordinates\0";
const uint8_t err_msg11[22] PROGMEM = "Invalid variable name\0";
const uint8_t err_msg12[23] PROGMEM = "Expected byte [0..255]\0";
const uint8_t err_msg13[13] PROGMEM = "Out of range\0";
const uint8_t err_msg14[24] PROGMEM = "Expected color [0..127]\0";

// keyboard connectivity messages
const uint8_t kb_fail_msg[26] PROGMEM = "Keyboard self-test failed\0";
const uint8_t kb_success_msg[32] PROGMEM = "Keyboard connected successfully\0";

/// @endcond

/** ***************************************************************************
 * @brief Print specified string
 *****************************************************************************/
void printstr (char *string, FILE *stream)
{
    uint8_t i = 0;
    while (string[i] != 0) {
        fputc (string[i], stream);
        i++;
    }
}

/** ***************************************************************************
 * @brief Print specified integer number
 *****************************************************************************/
void printnum (int16_t number, FILE *stream)
{
        int digits = 0;
        if (number < 0) {
                number = -number;
                fputc ('-', stream);
        }
        do {
                push_byte (number % 10 + '0');
                number = number / 10;
                digits++;
        } while (number > 0);

        while (digits > 0) {
                fputc (pop_byte(), stream);
                digits--;
        }
}

/** ***************************************************************************
 * @brief Print selected message from FLASH (without new-line)
 *****************************************************************************/
void printmsg_noNL (const uint8_t *message, FILE *stream)
{
        while (pgm_read_byte (message) != 0)
        fputc (pgm_read_byte (message++), stream);
}

/** ***************************************************************************
 * @brief Print selected message from FLASH (with new-line)
 *****************************************************************************/
void printmsg (const uint8_t *message, FILE *stream)
{
        printmsg_noNL (message, stream);
        newline (stream);
}

/** ***************************************************************************
 * @brief Print a program line
 *****************************************************************************/
void printline (uint8_t **line, FILE *stream)
{
        LINE_NUMBER line_num = *((LINE_NUMBER *)(*line));

        *line += sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);

        // print line number and a space
        printnum (line_num, stream);
        fputc (' ', stream);

        // print line content
        while (**line != LF) {
                fputc (**line, stream);
                line++;
        }
        line++;
        newline (stream);
}

/** ***************************************************************************
 * @brief Print a NEW LINE character
 *****************************************************************************/
void newline (FILE *stream)
{
        fputc (LF, stream);
        fputc (CR, stream);
}

/** ***************************************************************************
 * @brief Print a user defined string (enclosed in quotes)
 * @note The opening delimiter of the string is pointed to by @c text_ptr.
 *****************************************************************************/
uint8_t print_string (void)
{
        uint16_t i = 0;
        uint8_t delim = *text_ptr;
        // check for opening delimiter
        if (delim != '"' && delim != '\'')
        return 0;
        text_ptr++;
        // check for closing delimiter
        while (text_ptr[i] != delim) {
                if (text_ptr[i] == LF)
            return 0;
                i++;
        }
        // print characters
        while (*text_ptr != delim) {
                fputc (*text_ptr, stdout);
                text_ptr++;
        }
        text_ptr++; // skip closing
        return 1;
}

