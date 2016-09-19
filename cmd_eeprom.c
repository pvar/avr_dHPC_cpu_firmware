/*
 * Implementation of eeprom related commands of nstBASIC.
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

#include "cmd_eeprom.h"

uint8_t elist (void)
{
    uint8_t value;
        eeprom_ptr = 0;
        for (uint16_t i = 0 ; i < (E2END + 1); i++) {
                value = fgetc (&stream_eeprom);
                if (value == '\0')
            return POST_CMD_NEXT_LINE;
                if (((value < ' ') || (value > '~')) && (value != LF) && (value != CR))
            putchar ('?');
                else
            putchar (value);
        }
        newline (stdout);
        return POST_CMD_NEXT_LINE;
}

uint8_t eformat (void)
{
        eeprom_ptr = 0;
        for (uint16_t i = 0 ; i < E2END ; i++) {
                fputc ('\0', &stream_eeprom);
                // print a dot every 64 bytes
                if ((i & 0x07f) == 0x40)
            fputc ('.', stdout);
        }
        newline (stdout);
        return POST_CMD_NEXT_LINE;
}

uint8_t eload (void)
{
    uint8_t value;
        // read the first byte of eeprom
        // if it is a number, assume there is a program we can load
        eeprom_ptr = 0;
        value = fgetc (&stream_eeprom);
        if (value >= '0' && value <= '9') {
                eeprom_ptr = 0;
                prog_end_ptr = program_space;
                sys_config |= cfg_from_eeprom;
        } else
                error_code = 0x9;
    return POST_CMD_WARM_RESET;
}

uint8_t esave (void)
{
        eeprom_ptr = 0;
        uint8_t *line = find_line();
        LINE_LENGTH length = 0;
        while (line != prog_end_ptr) {
                printline (line, &stream_eeprom);
                length = (LINE_LENGTH) (*(line + sizeof (LINE_NUMBER)));
                line += length;
        }
        fputc (0, &stream_eeprom);
        return POST_CMD_NEXT_LINE;
}
