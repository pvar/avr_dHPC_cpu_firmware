// ----------------------------------------------------------------------------
// Implementation of audio related commands
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

#include "cmd_audio.h"

uint8_t play (void)
{
	send_to_apu (snd_play);
	return POST_CMD_NEXT_STATEMENT;
}

uint8_t stop (void)
{
	send_to_apu (snd_stop);
	return POST_CMD_NEXT_STATEMENT;
}

uint8_t tempo (void)
{
		uint16_t specified_tempo;
		ignorespace();
		specified_tempo = parse_expr_s1();
		if (error_code)
            return POST_CMD_WARM_RESET;
		switch (specified_tempo) {
            case 60:
                send_to_apu (snd_tempo);
                send_to_apu (0);
                break;
            case 120:
                send_to_apu (snd_tempo);
                send_to_apu (8);
                break;
            case 150:
                send_to_apu (snd_tempo);
                send_to_apu (16);
                break;
            case 180:
                send_to_apu (snd_tempo);
                send_to_apu (24);
                break;
		}
	return POST_CMD_NEXT_STATEMENT;
}

uint8_t music (void)
{
		uint8_t delim;
		ignorespace();
		error_code = 0;
		delim = *txtpos;
		// check for opening delimiter
		if (delim != '"' && delim != '\'') {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
		txtpos++;
		// loop until closing delimiter
		while (*txtpos != delim) {
			switch (*txtpos) {
                case 'Y':	// enable channel
                case 'y':
                case 'A':
                case 'a':
                    send_to_apu (snd_ena);
                    parse_channel();
                    break;
                case 'N':	// disable channel
                case 'n':
                case 'D':
                case 'd':
                    send_to_apu (snd_dis);
                    parse_channel();
                    break;
                case 'X':	// clear channel
                case 'x':
                case 'C':
                case 'c':
                    send_to_apu (snd_clr);
                    parse_channel();
                    break;
                case 'M':	// insert melody
                case 'm':
                case 'E':
                case 'e':
                    send_to_apu (snd_notes);
                    parse_channel();
                    if (error_code != 0)
                        break;
                    parse_notes();
                    break;
                default:
                    error_code = 0x4;
                    break;
			}
			if (error_code != 0) {
				send_to_apu (snd_abort);
                return POST_CMD_WARM_RESET;
			}
			// process next character
			txtpos++;
		}
		// skip closing delimiter
		txtpos++;
	return POST_CMD_NEXT_STATEMENT;
}
