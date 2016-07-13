// ----------------------------------------------------------------------------
// Implementation of eeprom related commands
// ----------------------------------------------------------------------------
//
// nstBASIC (not-so-tiny BASIC) is limited implementation
// of the venerable BASIC language. The code is heavily tied
// to the hardware of a homebrew computer.
//
// Created by Panos Varelas <varelaspanos@gmail.com>
//
// Based on "TinyBasic Plus" by:
//   - Mike Field <hamster@snap.net.nz>,
//   - Scott Lawrence <yorgle@gmail.com> and
//   - Jurg Wullschleger <wullschleger@gmail.com>
//     (fixed whitespace and unary operations)
//
// ----------------------------------------------------------------------------

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
		program_end = program_start;
		main_config |= cfg_from_eeprom;
	} else
		error_code = 0x9;
    return POST_CMD_WARM_RESET;
}

uint8_t esave (void)
{
	eeprom_ptr = 0;
	list_line = find_line();
	while (list_line != program_end)
        printline (&stream_eeprom);
	fputc (0, &stream_eeprom);
	return POST_CMD_NEXT_LINE;
}
