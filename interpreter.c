// ----------------------------------------------------------------------------
// Interpreter mechanism for nstBASIC :: not-so-tinyBASIC
// ----------------------------------------------------------------------------
//
// By Panos Varelas <varelaspanos@gmail.com>
//
// Based on "TinyBasic Plus" by:
//   Mike Field <hamster@snap.net.nz>,
//   Scott Lawrence <yorgle@gmail.com> and
//   Jurg Wullschleger <wullschleger@gmail.com>
//   (who fixed whitespace and unary operations)
//
// ----------------------------------------------------------------------------

#include "interpreter.h"
#include "parser.c"


void error_message (void)
{
	text_color (TXT_COL_ERROR);
	paper_color (0);
	switch (error_code) {
	case 0x1:	//// not yet implemented
		printmsg (err_msg01, stdout);
		break;
	case 0x2:	//// syntax error
		printmsg_noNL (err_msg02, stdout);
		if (current_line != NULL) {
			printf (" -- ");
			uint8_t tmp = *txtpos;
			if (*txtpos != LF) *txtpos = '^';
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

// ----------------------------------------------------------------------------
// BASIC init and parser loop
// ----------------------------------------------------------------------------
void basic_init (void)
{
    // environment init
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

INTERPRETER_LOOP://=================================================================================

warm_reset:
	// turn-on cursor
	putchar (vid_cursor_on);
	// turn-on scroll
	putchar (vid_scroll_on);
	// reset program-memory pointer
	current_line = 0;
	sp = program + MEMORY_SIZE;
	printmsg (msg_ok, stdout);

prompt:
	if ((main_config & cfg_auto_run) || (main_config & cfg_run_after_load)) {
		main_config &= ~cfg_auto_run;
		main_config &= ~cfg_run_after_load;
		current_line = program_start;
		goto execline;
	}

	/* get a new line */
	get_line();
	uppercase();
	/* find end of new line */
	txtpos = program_end + sizeof (uint16_t);
	while (*txtpos != LF) txtpos++;
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

	/* check if the line starts with a (line) number */
	linenum = linenum_test();
	ignorespace();

	/* if no number is present --> execute line immediately */
	if (linenum == 0)
        goto direct;

	/* if invalid number is present --> print error message */
	if (linenum == 0xFFFF) {
		error_code = 0x9;
		error_message();
        goto warm_reset;
	}

	/* if valid number is present --> merge line with rest of program */
    /* find the length of the string after the line number
	   (including the yet-to-be-populated line header) */
    linelen = 0;
	while (txtpos[linelen] != LF) linelen++;
	/* increase length to include LF character */
	linelen++;
	/* increase even more to make room for header (line number and line length) */
	linelen += sizeof (uint16_t) + sizeof (uint8_t);
	/* go to the beginning of line header */
	txtpos -= 3;
	/* add line number and length*/
	* ((uint16_t *)txtpos) = linenum;
	txtpos[sizeof (uint16_t)] = linelen;
	/* merge line with program */
	start = find_line();
	/* remove any line with the same line number */
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
	/* if new line has no text, it was just a delete */
	if (txtpos[sizeof (uint16_t) + sizeof (uint8_t)] == LF)
		goto prompt;
	/* make room for the new line */
	while (linelen > 0) {
		uint8_t *from, *dest;
		uint16_t tomove;
		uint16_t room_to_make;
		room_to_make = txtpos - program_end;
		if (room_to_make > linelen) room_to_make = linelen;
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
		// copy over the bytes into the new space
		for (tomove = 0; tomove < room_to_make; tomove++) {
			*start = *txtpos;
			txtpos++;
			start++;
			linelen--;
		}
		program_end = new_end;
	}
	goto prompt;

/////////////////////////////////////////////////////////////////////////////// execute next statement
run_next_statement:
	while (*txtpos == ':')
        txtpos++;
	ignorespace();
	if (*txtpos == LF)
        goto execnextline;
	goto start_interpretation;

direct:
	break_flow = 0;
	txtpos = program_end + sizeof (uint16_t);
	if (*txtpos == LF)
        goto prompt;

start_interpretation:
	if (break_test()) {
		printmsg (msg_break, stdout);
		goto warm_reset;
	}

/////////////////////////////////////////////////////////////////////////////// scan for valid commands
	scantable (commands);
	error_code = 0;

	switch (table_index) {
	case CMD_DELAY:
		val = parse_step1();
		fx_delay_ms (val);
		goto execnextline;
	case CMD_NEW:
		if (txtpos[0] != LF) {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		program_end = program_start;
		goto prompt;
	case CMD_BEEP:
		do_beep();
		goto execnextline;
	case CMD_RUN:
		//enable emergency break key (INT2)
		EIMSK |= BREAK_INT;
		// turn-off cursor
		putchar (vid_cursor_off);
		// turn-off scroll
		putchar (vid_scroll_off);
		current_line = program_start;
		goto execline;
	case CMD_IF:
		val = parse_step1();
		if (error_code || *txtpos == LF) {
			error_code = 0x4;
			error_message();
            goto warm_reset;

		}
		if (val != 0) goto start_interpretation;
		goto execnextline;
	case CMD_GOTO:
		linenum = parse_step1();
		if (error_code || *txtpos != LF) {
			error_code = 0x4;
			error_message();
            goto warm_reset;
		}
		current_line = find_line();
		goto execline;
	case CMD_MPLAY:
		goto play;
	case CMD_MSTOP:
		goto stop;
	case CMD_TEMPO:
		goto tempo;
	case CMD_MUSIC:
		goto music;
	case CMD_END:
	case CMD_STOP:
		// set current line at the end of program
		if (txtpos[0] != LF) {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		current_line = program_end;
		goto execline;
	case CMD_LIST:
		goto list;
	case CMD_MEM:
		goto mem;
	case CMD_PEN:
		goto pen;
	case CMD_PAPER:
		goto paper;
	case CMD_NEXT:
		goto next;
	case CMD_LET:
		goto assignment;
	case CMD_GOSUB:
		goto gosub;
	case CMD_RETURN:
		goto gosub_return;
	case CMD_RANDOMIZE:
		goto randomize;
	case CMD_RNDSEED:
		goto rndseed;
	case CMD_FOR:
		goto forloop;
	case CMD_INPUT:
		goto input;
	case CMD_POKE:
		goto poke;
	case CMD_PSET:
		goto pset;
	case CMD_ELIST:
		goto elist;
	case CMD_EFORMAT:
		goto eformat;
	case CMD_ECHAIN:
		goto echain;
	case CMD_ESAVE:
		goto esave;
	case CMD_ELOAD:
		goto eload;
	case CMD_RST:
		goto reset_display;
	case CMD_PRINT:
	case CMD_QMARK:
		goto print;
	case CMD_LOCATE:
		goto locate;
	case CMD_CLS:
		goto clear_screen;
	case CMD_REM:
	case CMD_HASH:
	case CMD_QUOTE:
		goto execnextline;
	case CMD_PINDIR:
		goto pindir;
	case CMD_PINDWRITE:
		goto pindwrite;
	case CMD_DEFAULT:
		goto assignment;
	default:
		break;
	}

/////////////////////////////////////////////////////////////////////////////// execute this line or the next
execnextline:
	// if processing direct commands, print prompt and do not proceed
	if (current_line == NULL)
		goto warm_reset;
	current_line += current_line[sizeof (uint16_t)];
execline:
	// "reset" if reached the end of program
	if (current_line == program_end)
		goto warm_reset;
	txtpos = current_line + sizeof (uint16_t) + sizeof (uint8_t);
	goto start_interpretation;

    
/////////////////////////////////////////////////////////////////////////////// eeprom related
elist:
	eeprom_ptr = 0;
	for (uint16_t i = 0 ; i < (E2END + 1); i++) {
		val = fgetc (&stream_eeprom);
		if (val == '\0')
            goto execnextline;
		if (((val < ' ') || (val > '~')) && (val != LF) && (val != CR))
            putchar ('?');
		else
            putchar (val);
	}
	newline (stdout);
	goto execnextline;
eformat:
	eeprom_ptr = 0;
	for (uint16_t i = 0 ; i < E2END ; i++) {
		fputc ('\0', &stream_eeprom);
		// print a dot every 64 bytes
		if ((i & 0x07f) == 0x40)
            fputc ('.', stdout);
	}
	newline (stdout);
	goto execnextline;
echain:
	main_config |= cfg_run_after_load;
eload:
	// read the first byte of eeprom
	// if it is a number, assume there is a program we can load
	eeprom_ptr = 0;
	val = fgetc (&stream_eeprom);
	if (val >= '0' && val <= '9') {
		eeprom_ptr = 0;
		program_end = program_start;
		main_config |= cfg_from_eeprom;
		goto warm_reset;
	} else {
		error_code = 0x9;
		error_message();
        goto warm_reset;
	}
esave:
	eeprom_ptr = 0;
	list_line = find_line();
	while (list_line != program_end)
        printline (&stream_eeprom);
	fputc (0, &stream_eeprom);
	goto execnextline;
/////////////////////////////////////////////////////////////////////////////// flow control
forloop: {
		uint8_t var;
		uint16_t initial, step, terminal;
		ignorespace();
		if (*txtpos < 'A' || *txtpos > 'Z') {
			error_code = 0x7;
			error_message();
            goto warm_reset;
		}
		var = *txtpos;
		txtpos++;
		ignorespace();
		if (*txtpos != '=') {
			error_code = 0xA;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		ignorespace();
		error_code = 0;
		initial = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		scantable (to_tab);
		if (table_index != 0) {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		terminal = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
        scantable (step_tab);
		if (table_index == 0) {
			step = parse_step1();
			if (error_code) {
                error_message();
            	goto warm_reset;
            }
		} else step = 1;
		ignorespace();
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		if (!error_code && *txtpos == LF) {
			struct stack_for_frame *f;
			if (sp + sizeof (struct stack_for_frame) < stack_limit) {
				error_code = 0x3;
                error_message();
                goto warm_reset;
			}
			sp -= sizeof (struct stack_for_frame);
			f = (struct stack_for_frame *)sp;
			((uint16_t *)variables_begin)[var - 'A'] = initial;
			f->frame_type = STACK_FOR_FLAG;
			f->for_var = var;
			f->terminal = terminal;
			f->step	 = step;
			f->txtpos	= txtpos;
			f->current_line = current_line;
			goto run_next_statement;
		}
	}
	error_code = 0x4;
	error_message();
	goto warm_reset;
gosub:
	error_code = 0;
	linenum = parse_step1();
	if (!error_code && *txtpos == LF) {
		struct stack_gosub_frame *f;
		if (sp + sizeof (struct stack_gosub_frame) < stack_limit) {
			error_code = 0x3;
			error_message();
            goto warm_reset;
		}
		sp -= sizeof (struct stack_gosub_frame);
		f = (struct stack_gosub_frame *)sp;
		f->frame_type = STACK_GOSUB_FLAG;
		f->txtpos = txtpos;
		f->current_line = current_line;
		current_line = find_line();
		goto execline;
	}
	error_code = 0x4;
	error_message();
    goto warm_reset;
next:
	// find the variable name
	ignorespace();
	if (*txtpos < 'A' || *txtpos > 'Z') {
		error_code = 0x7;
		error_message();
        goto warm_reset;
	}
	txtpos++;
	ignorespace();
	if (*txtpos != ':' && *txtpos != LF) {
		error_code = 0x2;
		error_message();
        goto warm_reset;
	}
gosub_return:
	// walk up the stack frames and find the frame we want -- if present
	tempsp = sp;
	while (tempsp < program + MEMORY_SIZE - 1) {
		switch (tempsp[0]) {
		case STACK_GOSUB_FLAG:
			if (table_index == CMD_RETURN) {
				struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
				current_line	= f->current_line;
				txtpos			= f->txtpos;
				sp += sizeof (struct stack_gosub_frame);
				goto run_next_statement;
			}
			// This is not the loop you are looking for... go up in the stack
			tempsp += sizeof (struct stack_gosub_frame);
			break;
		case STACK_FOR_FLAG:
			// Flag, Var, Final, Step
			if (table_index == CMD_NEXT) {
				struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
				// Is the variable we are looking for?
				if (txtpos[-1] == f->for_var) {
					uint16_t *varaddr = ((uint16_t *)variables_begin) + txtpos[-1] - 'A';
					*varaddr = *varaddr + f->step;
					// Use a different test depending on the sign of the step increment
					if ((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal)) {
						// We have to loop so don't pop the stack
						txtpos = f->txtpos;
						current_line = f->current_line;
						goto run_next_statement;
					}
					// We've run to the end of the loop. drop out of the loop, popping the stack
					sp = tempsp + sizeof (struct stack_for_frame);
					goto run_next_statement;
				}
			}
			// This is not the loop you are looking for... go up in the stack
			tempsp += sizeof (struct stack_for_frame);
			break;
		default:
			//printf( "Stack is stuffed!\n" );
			goto warm_reset;
		}
	}
	// Didn't find the variable we've been looking for
	error_code = 0x8;
	error_message();
    goto warm_reset;

/////////////////////////////////////////////////////////////////////////////// variable definition statements
input: {
		uint8_t chr = 0;
		uint8_t cnt = 0;
		int16_t *var;
		int16_t test;
		// variable to store user value
		ignorespace();
		if (*txtpos < 'A' || *txtpos > 'Z') {
			error_code = 0x7;
			error_message();
            goto warm_reset;
		}
		var = (int16_t *)variables_begin + *txtpos - 'A';
		// check for proper statement termination
		txtpos++;
		ignorespace();
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		// get user value (accept only digits)
		inptr = in_buffer;
		inptr[0] = 0;
		cnt = 0;
		EIMSK |= BREAK_INT; //enable emergency break key (INT2)
		while (chr != LF && chr != CR) {
			chr = fgetc (stdin);
			if (break_flow == 1)
                goto run_next_statement;
			switch (chr) {
			case '-':
				// only accept minus sign as first character
				if (cnt == 0) {
					putchar (chr);
					inptr[0] = chr;
					cnt++;
					inptr++;
					inptr[0] = 0;
				}
				break;
			case LF:
			case CR:
				inptr[0] = LF;
				break;
			case BS:
				if (inptr <= in_buffer) do_beep();
				else {
					putchar (BS);
					inptr--;
					cnt--;
				}
				break;
			default:
				if (chr >= '0' && chr <= '9') {
					if (cnt < INPUT_BUFFER_SIZE) {
						putchar (chr);
						inptr[0] = chr;
						cnt++;
						inptr++;
						// mark last character with ASCII code 0
						inptr[0] = 0;
					} else do_beep();
				}
			}
		}
		newline (stdout);
		// parse and store user value
		*var = str_to_num (in_buffer);
	}
	goto run_next_statement;
assignment: {
		int16_t value, *var;
		// check if invalid character (non-letter)
		if (*txtpos < 'A' || *txtpos > 'Z') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		var = (int16_t *)variables_begin + *txtpos - 'A';
		txtpos++;
		// check if invalid variable name (more than one letters)
		while (*txtpos >= 'A' && *txtpos <= 'Z') {
			error_code = 0xFF;
			txtpos++;
		}
		// check for missing assignment operator
		ignorespace();
		if (*txtpos != '=') {
			error_code = 0xF;
			error_message();
            goto warm_reset;
		} else {
			// check if variable name is rendered invalid
			if (error_code == 0xFF) {
				error_code = 0x11;
				error_message();
                goto warm_reset;
			}
		}
		txtpos++;
		ignorespace();
		value = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		// Check that we are at the end of the statement
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		*var = value;
	}
	goto run_next_statement;
poke: {
		int16_t value, address;
		// get the address
		address = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (address > MEMORY_SIZE) {
			error_code = 0x13;
			error_message();
            goto warm_reset;
		}
		// check for comma
		ignorespace();
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		// get the value to assign
		ignorespace();
		value = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (value < 0 || value > 255) {
			error_code = 0x12;
			error_message();
            goto warm_reset;
		}
		// check for proper statement termination
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		// assign value to specified location in memory
		program[address] = value;
	}
	goto run_next_statement;
pset: {
		uint16_t x, y, col;
		// get x-coordinate
		x = parse_step1();
		if (error_code) {
			error_message();
            goto warm_reset;
        }
		if (x < 0 || x > 255) {
			error_code = 0x10;
			error_message();
            goto warm_reset;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		// get y-coordinate
		y = parse_step1();
		if (error_code) {
			error_message();
            goto warm_reset;
        }
		if (y < 0 || y > 239) {
			error_code = 0x10;
			error_message();
            goto warm_reset;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
        }
		txtpos++;
		// get color
		col = parse_step1();
		if (error_code) {
			error_message();
            goto warm_reset;
        }
		if (col < 0 || col > 127) {
			error_code = 0x14;
			error_message();
            goto warm_reset;
		}
		put_pixel ((uint8_t)x, (uint8_t)y, (uint8_t)col);
	}
	goto execnextline;
/////////////////////////////////////////////////////////////////////////////// pin control
pindir: {
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
			error_message();
		    goto warm_reset;
        }
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		// get direction [0/1]
		b = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
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
			error_message();
            goto warm_reset;
			break;
		}
	}
	goto execnextline;
pindwrite: {
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
			error_message();
            goto warm_reset;
		}
		// check for comma -- two parameters expected
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		// get value [0/1]
		b = parse_step1();
        if (error_code) {
            error_message();
            goto warm_reset;
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
			error_message();
            goto warm_reset;
			break;
		}
	}
	goto execnextline;
/////////////////////////////////////////////////////////////////////////////// video card related
reset_display:
	putchar (vid_reset);
	uart_ansi_rst_clr();
	printmsg (msg_welcome, stdout);
	program_end = program_start;
	goto prompt;
clear_screen:
	putchar (vid_clear);
	uart_ansi_rst_clr();
	goto execnextline;
pen: {
		uint16_t col;
		// get color value
		col = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (col < 0 || col > 127) {
			error_code = 0x14;
			error_message();
            goto warm_reset;
		}
		text_color ((uint8_t)col);
	}
	goto execnextline;
paper: {
		uint16_t col;
		// get color value
		col = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (col < 0 || col > 127) {
			error_code = 0x14;
			error_message();
            goto warm_reset;
		}
		paper_color ((uint8_t)col);
	}
	goto execnextline;
locate: {
		uint16_t line, column;
		// get target line
		line = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (line < 0 || line > 23) {
			error_code = 0x10;
			error_message();
            goto warm_reset;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
		txtpos++;
		// get target line
		column = parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		if (column < 0 || column > 31) {
			error_code = 0x10;
			error_message();
            goto warm_reset;
		}
		locate_cursor (line, column);
	}
	goto execnextline;
print:
	// If we have an empty list then just put out a LF
	if (*txtpos == ':') {
		newline (stdout);
		txtpos++;
		goto run_next_statement;
	}
	if (*txtpos == LF)
		goto execnextline;
	while (1) {
		ignorespace();
		if (print_string())
			;
		else if (*txtpos == '"' || *txtpos == '\'') {
			error_code = 0x4;
			error_message();
            goto warm_reset;
		} else {
			uint16_t e;
			error_code = 0;
			e = parse_step1();
			if (error_code) {
                error_message();
                goto warm_reset;
            }
			printnum (e, stdout);
		}
		// at this point we have three options, a comma or a new line
		if (*txtpos == ',') txtpos++;   // skip the comma and move on
		else if (txtpos[0] == ';' && (txtpos[1] == LF || txtpos[1] == ':')) {
			txtpos++; // end of print without newline
			break;
		} else if (*txtpos == LF || *txtpos == ':') {
			newline (stdout);
			break;
		} else {
			error_code = 0x2;
			error_message();
            goto warm_reset;
		}
	}
	goto run_next_statement;
/////////////////////////////////////////////////////////////////////////////// audio card related
play:
	send_to_apu (snd_play);
	goto run_next_statement;
stop:
	send_to_apu (snd_stop);
	goto run_next_statement;
tempo: {
		uint16_t specified_tempo;
		ignorespace();
		specified_tempo = parse_step1();
		if (error_code) {
			error_message();
            goto warm_reset;
        }
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
	}
	goto run_next_statement;
music: {
		uint8_t delim;
		ignorespace();
		error_code = 0;
		delim = *txtpos;
		// check for opening delimiter
		if (delim != '"' && delim != '\'') {
			error_code = 0x2;
			error_message();
            goto warm_reset;
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
				error_message();
                goto warm_reset;
			}
			// process next character
			txtpos++;
		}
		// skip closing delimiter
		txtpos++;
	}
	goto run_next_statement;
/////////////////////////////////////////////////////////////////////////////// other commands
list:
	linenum = linenum_test(); // returns 0 if no line found
	// should be EOL
	if (txtpos[0] != LF) {
		error_code = 0x4;
		error_message();
        goto warm_reset;
	}
	// find the line
	list_line = find_line();
	while (list_line != program_end)
        printline (stdout);
	goto warm_reset;
mem: {
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
	}
	goto run_next_statement;
randomize:
	srand (TCNT2);
	goto run_next_statement;
rndseed: {
		uint16_t param;
		error_code = 0;
		// get seed for PRNG
		param = (uint16_t)parse_step1();
		if (error_code) {
            error_message();
            goto warm_reset;
        }
		srand (param);
	}
	goto run_next_statement;
}
