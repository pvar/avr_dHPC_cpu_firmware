// ----------------------------------------------------------------------------
// Interpreter loop for nstBASIC
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
#include "cmd_audio.h"
#include "cmd_screen.h"
#include "cmd_flow.h"
#include "cmd_eeprom.h"
#include "cmd_pinctl.h"
#include "cmd_other.h"

#include "parser.c"
#include "cmd_audio.c"
#include "cmd_screen.c"
#include "cmd_flow.c"
#include "cmd_eeprom.c"
#include "cmd_pinctl.c"
#include "cmd_other.c"

uint16_t get_linenumber (void)
{
	uint16_t num = 0;
	ignorespace();
	while (*txtpos >= '0' && *txtpos <= '9') {
		// Trap overflows
		if (num >= 0xFFFF / 10) {
			num = 0xFFFF;
			break;
		}
		num = num * 10 + *txtpos - '0';
		txtpos++;
	}
	return	num;
}

void append_line (void)
{
	// make room for line
	while (linelen > 0) {
		uint8_t *from, *dest;
		uint16_t tomove;
		uint16_t room_to_make;
		room_to_make = txtpos - program_end;
		if (room_to_make > linelen)
            room_to_make = linelen;
		new_end = program_end + room_to_make;
		tomove = program_end - start;
		// source and destination
		from = program_end;
		dest = new_end;
		while (tomove > 0) {
			from--;
			dest--;
			*dest = *from;
			tomove--;
		}
		// copy line content
		for (tomove = 0; tomove < room_to_make; tomove++) {
			*start = *txtpos;
			txtpos++;
			start++;
			linelen--;
		}
		program_end = new_end;
	}
}

void remove_line (void)
{
    if (start != program_end && * ((uint16_t *)start) == linenum) {
		uint8_t *dest, *from;
		uint16_t tomove;
		from = start + start[sizeof (uint16_t)];
		dest = start;
		tomove = program_end - from;
		while (tomove > 0) {
			*dest = *from;
			from++;
			dest++;
			tomove--;
		}
		program_end = dest;
	}
}

void move_line (void)
{
	/* find end of new line */
	txtpos = program_end + sizeof (uint16_t);
	while (*txtpos != LF)
        txtpos++;

	/* move line to the end of program_memory */
	uint8_t *dest;
	dest = variables_begin - 1;
	while (1) {
		*dest = *txtpos;
		if (txtpos == program_end + sizeof (uint16_t))
            break;
		dest--;
		txtpos--;
	}
	txtpos = dest;
}

void prep_line (void)
{
    /* find length of line */
    linelen = 0;
    while (txtpos[linelen] != LF)
        linelen++;
    /* increase to account for LF */
    linelen++;
    /* increase even more for line-header */
    linelen += sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);
    /* move pointer to the beginning of line header */
    txtpos -= sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);
    /* add line number and length*/
    * ((LINE_NUMBER *)txtpos) = linenum;

    txtpos[sizeof (LINE_NUMBER)] = linelen;
    //* ((LINE_LENGTH *)(txtpos + sizeof(LINE_NUMBER))) = linelen;
}



void error_message (void)
{
	text_color (TXT_COL_ERROR);
	paper_color (0);
	switch (error_code) {
        case 0x1:	// not yet implemented
            printmsg (err_msg01, stdout);
            break;
        case 0x2:	// syntax error
            printmsg_noNL (err_msg02, stdout);
            if (current_line != NULL) {
                printf (" -- ");
                uint8_t tmp = *txtpos;
                if (*txtpos != LF)
                    *txtpos = '^';
                list_line = current_line;
                printline (stdout);
                *txtpos = tmp;
            }
            newline (stdout);
            break;
        case 0x3:	// stack overflow
            printmsg (err_msg03, stdout);
            break;
        case 0x4:	// unexpected character
            printmsg (err_msg04, stdout);
            break;
        case 0x5:	// left parenthesis missing
            printmsg (err_msgxl, stdout);
            printmsg (err_msg05_06, stdout);
            break;
        case 0x6:	// right parenthesis missing
            printmsg (err_msgxr, stdout);
            printmsg (err_msg05_06, stdout);
            break;
        case 0x7:	// variable expected
            printmsg (err_msg07, stdout);
            break;
        case 0x8:	// jump point not found
            printmsg (err_msg08, stdout);
            break;
        case 0x9:	// invalid line number
            printmsg (err_msg09, stdout);
            break;
        case 0xA:	// operator expected
            printmsg (err_msg0A, stdout);
            break;
        case 0xB:	// division by zero
            printmsg (err_msg0B, stdout);
            break;
        case 0xC:	// invalid pin
            printmsg (err_msg0C, stdout);
            break;
        case 0xD:	// pin io error
            printmsg (err_msg0D, stdout);
            break;
        case 0xE:	// unknown function
            printmsg (err_msg0E, stdout);
            break;
        case 0xF:	// unknown command
            printmsg (err_msg0F, stdout);
            break;
        case 0x10:	// invalid coordinates
            printmsg (err_msg10, stdout);
            break;
        case 0x11:	// invalid variable name
            printmsg (err_msg11, stdout);
            break;
        case 0x12:	// expected byte
            printmsg (err_msg12, stdout);
            break;
        case 0x13:	// out of range
            printmsg (err_msg13, stdout);
            break;
        case 0x14:	// expected color value
            printmsg (err_msg14, stdout);
            break;
	}
	text_color (TXT_COL_DEFAULT);
	paper_color (0);
}

void warm_reset (void)
{
    // turn-on cursor
    putchar (vid_cursor_on);
    // turn-on scroll
    putchar (vid_scroll_on);
    // reset program-memory pointer
    current_line = 0;
    sp = program + MEMORY_SIZE;
    printmsg (msg_ok, stdout);
}

// ----------------------------------------------------------------------------
// Language environment init
// ----------------------------------------------------------------------------

void basic_init (void)
{
	program_start = program;
	program_end = program_start;
	sp = program + MEMORY_SIZE; // needed for printnum
	stack_limit = program + MEMORY_SIZE - STACK_SIZE;
	variables_begin = stack_limit - 27 * VAR_SIZE;

	// print (available) SRAM size
	printnum (variables_begin - program_end, stdout);
	printmsg (msg_ram_bytes, stdout);

	// print EEPROM size
	printnum (E2END + 1, stdout);
	printmsg (msg_rom_bytes , stdout);
	newline (stdout);
}

// ----------------------------------------------------------------------------
//   interpreter loop
// ----------------------------------------------------------------------------

void interpreter (void)
{
    uint8_t exec_status;

    /* start interpreter with a warm-reset */
    exec_status = POST_CMD_WARM_RESET;

    while(1) {
        /* if have to --> perform warm-reset */
        if (exec_status == POST_CMD_WARM_RESET) {
            exec_status = POST_CMD_NOTHING;
            warm_reset();
        }

        while(1) {
            error_code = 0;
            /* check if autorun is enabled */
            if ((main_config & cfg_auto_run) || (main_config & cfg_run_after_load)) {
                main_config &= ~cfg_auto_run;
                main_config &= ~cfg_run_after_load;
                current_line = program_start;
                txtpos = current_line + sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);
                break;
            }
            get_line();
            uppercase();
            move_line();
            /* attempt to read line number */
            linenum = get_linenumber();
            ignorespace();
            /* line number is present --> manipulate line */
            if (linenum != 0) {
                /* invalid line number --> ignore line :: warm reset */
                if (linenum == 0xFFFF) {
                    error_code = 0x9;
                    break;
                }
                /* valid line number --> merge with program */
                else {
                    /* embed line number and line length */
                    prep_line();
                    /* remove line with same line number */
                    start = find_line();
                    remove_line();
                    /* new line is empty --> get another one */
                    if (txtpos[sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH)] == LF)
                        continue;
                    /* append new line to program */
                    append_line();
                }
            }
            /* no line number --> execute it immediately */
            else {
                break_flow = 0;
                txtpos = program_end + sizeof (uint16_t);
                if (*txtpos == LF)
                    continue;
                else
                    break;
            }
        }
        /* if no error --> start execution */
        if (error_code == 0)
            exec_status = execution();

        // do not combine these two checks!
        // should check for error again, after execution...

        /* if error --> print message */
        if (error_code)
            error_message();
    }
}

uint8_t execution (void)
{
    while(1) {
        if (break_test()) {
            printmsg (msg_break, stdout);
            return POST_CMD_WARM_RESET;
        }

        cmd_status = POST_CMD_NOTHING;
        table_index = 0;
        error_code = 0;
        scantable (commands);

        switch (table_index) {
            case CMD_DELAY:
                val = parse_step1();
                fx_delay_ms (val);
                cmd_status = POST_CMD_NEXT_LINE;
                break;
            case CMD_NEW:
                cmd_status = prog_new();
                break;
            case CMD_BEEP:
                do_beep();
                cmd_status = POST_CMD_NEXT_LINE;
                break;
            case CMD_RUN:
                cmd_status = prog_run();
                break;
            case CMD_IF:
                cmd_status = check();
                break;
            case CMD_GOTO:
                cmd_status = gotoline();
                break;
            case CMD_MPLAY:
                cmd_status = play();
                break;
            case CMD_MSTOP:
                cmd_status = stop();
                break;
            case CMD_TEMPO:
                cmd_status = tempo();
                break;
            case CMD_MUSIC:
                cmd_status = music();
                break;
            case CMD_END:
            case CMD_STOP:
                cmd_status = prog_end();
                break;
            case CMD_LIST:
                cmd_status = list();
                break;
            case CMD_MEM:
                cmd_status = mem();
                break;
            case CMD_PEN:
                cmd_status = pen();
                break;
            case CMD_PAPER:
                cmd_status = paper();
                break;
            case CMD_NEXT:
                cmd_status = next();
                if (error_code)
                    break;
                cmd_status = gosub_return();
                break;
            case CMD_LET:
                cmd_status = assignment();
                break;
            case CMD_GOSUB:
                cmd_status = gosub();
                break;
            case CMD_RETURN:
                cmd_status = gosub_return();
                break;
            case CMD_RANDOMIZE:
                cmd_status = randomize();
                break;
            case CMD_RNDSEED:
                cmd_status = rndseed();
                break;
            case CMD_FOR:
                cmd_status = loopfor();
                break;
            case CMD_INPUT:
                cmd_status = input();
                break;
            case CMD_POKE:
                cmd_status = poke();
                break;
            case CMD_PSET:
                cmd_status = pset();
                break;
            case CMD_ELIST:
                cmd_status = elist();
                break;
            case CMD_EFORMAT:
                cmd_status = eformat();
                break;
            case CMD_ECHAIN:
                main_config |= cfg_run_after_load;
                cmd_status = eload();
                break;
            case CMD_ESAVE:
                cmd_status = esave();
                break;
            case CMD_ELOAD:
                cmd_status = eload();
                break;
            case CMD_RST:
                cmd_status = reset_display();
                break;
            case CMD_PRINT:
            case CMD_QMARK:
                cmd_status = print();
                break;
            case CMD_LOCATE:
                cmd_status = locate();
                break;
            case CMD_CLS:
                cmd_status = clear_screen();
                break;
            case CMD_REM:
            case CMD_HASH:
            case CMD_QUOTE:
                cmd_status = POST_CMD_NEXT_LINE;
                break;
            case CMD_PINDIR:
                cmd_status = pindir();
                break;
            case CMD_PINDWRITE:
                cmd_status = pindwrite();
                break;
            case CMD_DEFAULT:
                cmd_status = assignment();
                break;
            default:
                cmd_status = POST_CMD_NEXT_LINE;
                break;
        }

        // check if should warm reset
        if (cmd_status == POST_CMD_WARM_RESET)
            return POST_CMD_WARM_RESET;

        // check if should return to prompt
        if (cmd_status == POST_CMD_PROMPT)
            return POST_CMD_PROMPT;

        // check if should restart interpretation
        if (cmd_status == POST_CMD_LOOP)
            continue;

        if (cmd_status == POST_CMD_NEXT_STATEMENT) {
            ignorespace();
            if (*txtpos == ':') {
                while (*txtpos == ':')
                    txtpos++;
                ignorespace();
                continue;
            } else
                cmd_status = POST_CMD_NEXT_LINE;
        }

        // check if should proceed to next line
        if (cmd_status == POST_CMD_NEXT_LINE) {
            // check if in direct mode (no line number)
            if (current_line == NULL)
                return POST_CMD_WARM_RESET;
            // proceed to next line
            current_line += current_line[sizeof (uint16_t)];
        }

        // warm reset if reached end of program
        if (current_line == program_end)
            return POST_CMD_WARM_RESET;

        // if reached here, start execution of next line
        txtpos = current_line + sizeof (LINE_NUMBER) + sizeof (LINE_LENGTH);
    }
    //fputc ('>', &stream_pseudo);
    //printnum ((uint16_t)cmd_status, &stream_pseudo);
}