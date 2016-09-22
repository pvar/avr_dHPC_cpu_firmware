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

#include "cmd_serial.h"

uint8_t sload (void)
{
        // get lines from SERIAL
        sys_config |= cfg_from_serial;
        // reset program space pointer
        prog_end_ptr = program_space;

        return POST_CMD_WARM_RESET;
}

uint8_t ssave (void)
{
        uint8_t *line = find_line();
        LINE_LENGTH length = 0;
        while (line != prog_end_ptr) {
                printline (line, &stream_serial);
                length = (LINE_LENGTH) (*(line + sizeof (LINE_NUMBER)));
                line += length;
        }
        fputc (0, &stream_serial);
        return POST_CMD_NEXT_LINE;
}
