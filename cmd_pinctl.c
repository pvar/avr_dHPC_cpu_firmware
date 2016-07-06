// ----------------------------------------------------------------------------
// Implementation of IO pin-control commands
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

#include "interpreter.h"
#include "cmd_pinctl.h"

uint8_t pindir (void)
{
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
		    return POST_CMD_WARM_RESET;
        }
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
		txtpos++;
		// get direction [0/1]
		b = parse_step1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		// create mask for altering direction
		a = 1 << a;
		switch (b) {
            case 0:
                sec_data_bus_dir &= ~a;
                break;
            case 1:
                sec_data_bus_dir |= a;
                break;
            default:
                // direction can only be 1 or 0
                error_code = 0x2;
                return POST_CMD_WARM_RESET;
		}
	return POST_CMD_NEXT_LINE;
}

uint8_t pindwrite (void)
{
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
            return POST_CMD_WARM_RESET;
		}
		// check for comma -- two parameters expected
		if (*txtpos != ',') {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
		txtpos++;
		// get value [0/1]
		b = parse_step1();
        if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		// create mask for altering direction
		a = 1 << a;
		switch (b) {
            case 0:
                sec_data_bus_out &= ~a;
                break;
            case 1:
                sec_data_bus_out |= a;
                break;
            default:
                // specified value can only be 1 or 0
                error_code = 0x2;
                return POST_CMD_WARM_RESET;
		}
	return POST_CMD_NEXT_LINE;
}