// ----------------------------------------------------------------------------
// nstBASIC :: not-so-tinyBASIC
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

#include "parser.h"



// ----------------------------------------------------------------------------
// search for valid function and command names
// ----------------------------------------------------------------------------
void scantable (const uint8_t *table)
{
	uint16_t i = 0;
	table_index = 0;
	while (1) {
		// end of given table...
		if (pgm_read_byte (table) == 0)
            return;
		// character match...
		if (txtpos[i] == pgm_read_byte (table)) {
			i++;
			table++;
		} else {
			// match the last character of keyword (0x80 added)
			if (txtpos[i] + 0x80 == pgm_read_byte (table)) {
				txtpos += i + 1;	// point after the scanned keyword
				ignorespace();
				return;
			}
			// move to the end of this keyword
			while ((pgm_read_byte (table) & 0x80) == 0)
                table++;
			// move to first character of next keyword and reset position index
			table++;
			table_index++;
			ignorespace();
			i = 0;
		}
	}
}

// ----------------------------------------------------------------------------
// scan string for a single note
// ----------------------------------------------------------------------------
uint8_t get_note (void)
{
	// NOTE		RETURN VALUE
	//	C			01
	//	C#			02
	//	D			03
	//	Eb			04
	//	E			05
	//	F			06
	//	F#			07
	//	G			08
	//	G#			09
	//	A			10
	//	Bb			11
	//	B			12
	//	PAUSE		13
	txtpos++;
	switch (*txtpos) {
        case 'C':
        case 'c':
            txtpos++;
            if (*txtpos == '#')
                return 2;
            else {
                txtpos--;
                return 1;
            }
            break;
        case 'D':
        case 'd':
            return 3;
            break;
        case 'E':
        case 'e':
            txtpos++;
            if (*txtpos == 'B' || *txtpos == 'b')
                return 4;
            else {
                txtpos--;
                return 5;
            }
            break;
        case 'F':
        case 'f':
            txtpos++;
            if (*txtpos == '#')
                return 7;
            else {
                txtpos--;
                return 6;
            }
            break;
        case 'G':
        case 'g':
            txtpos++;
            if (*txtpos == '#')
                return 9;
            else {
                txtpos--;
                return 8;
            }
            break;
        case 'A':
        case 'a':
            return 10;
            break;
        case 'B':
        case 'b':
            txtpos++;
            if (*txtpos == 'B' || *txtpos == 'b')
                return 11;
            else {
                txtpos--;
                return 12;
            }
            break;
        case 'P':
        case 'p':
            return 13;
            break;
        default:
            error_code = 0x4;
            break;
	}
}

// ----------------------------------------------------------------------------
// scan string for octave value
// ----------------------------------------------------------------------------
uint8_t get_octave (void)
{
	// OCTAVE		RETURN VALUE
	//	2				2
	//	...				...
	//	7				7
	txtpos++;
	if (*txtpos >= '2' && *txtpos <= '7')
		return *txtpos - '0';
	else if (*txtpos == '"' || *txtpos == '\'') {
		txtpos--;
		return 0;
	} else {
		error_code = 0x4;
		return 0;
	}
}

// ----------------------------------------------------------------------------
// scan string for duration value
// ----------------------------------------------------------------------------
uint8_t get_duration (void)
{
	// DURATION			RETURN VALUE
	//	1/32				1
	//	1/16				2
	//	1/16* (3/32)		3
	//	1/8					4
	//	1/8*  (3/16)		5
	//	1/4					6
	//	1/4*  (3/8)			7
	//	1/2					8
	txtpos++;
	if (*txtpos >= '1' && *txtpos <= '8')
		return *txtpos - '0';
	else {
		error_code = 0x4;
		return 0;
	}
}

// ----------------------------------------------------------------------------
// scan string for duration value
// ----------------------------------------------------------------------------
uint8_t get_effect (void)
{
	// EFFECT			RETURN VALUE
	//	none				0
	//	bend up				1
	//	bend down			2
	//	vibrato				3
	txtpos++;
	switch (*txtpos) {
        case '=':	// vibrato
            return 3;
            break;
        case '-':	// bend down
            return 2;
            break;
        case '+':	// bend up
            return 1;
            break;
        default:	// no effect
            txtpos--;
            return 0;
            break;
	}
}

// ----------------------------------------------------------------------------
// scan string for channel number
// ----------------------------------------------------------------------------
void parse_channel (void)
{
	txtpos++;
	if (*txtpos >= '1' && *txtpos <= '4') {
		send_to_apu (*txtpos - '0');
		return;
	} else {
		error_code = 0x4;
		return;
	}
}

// ----------------------------------------------------------------------------
// parse string for notes
// ----------------------------------------------------------------------------
void parse_notes (void)
{
	uint8_t tmp1, tmp2, tmp3, tmp4;
	uint8_t params, note;
	while (1) {
		//txtpos++;
		tmp1 = get_octave();
		// check separately -- exit if no other notes
		if (tmp1 == 0)
			return;
		tmp2 = get_note();
		tmp3 = get_duration();
		tmp4 = get_effect();
		// check separately -- exit on errors
		if ((tmp2 | tmp3 | tmp4) == 0)
			return;
		if (tmp2 == 13)
			note = 144;
		else
			note = (24 * (tmp1 - 2)) + 2 * (tmp2 - 1);
		params = (tmp3 - 1) + (tmp4 * 64);
		send_to_apu (params);
		send_to_apu (note);
	}
}

// ----------------------------------------------------------------------------
// relational operators
// ----------------------------------------------------------------------------
int16_t parse_step1 (void)
{
	int16_t value1, value2;
	value1 = parse_step2();
	// check for error
	if (error_code)
        return value1;
	scantable (relop_table);
	if (table_index == RELOP_UNKNOWN)
        return value1;
	switch (table_index) {
        case RELOP_GE:
            value2 = parse_step2();
            if (value1 >= value2)
                return 1;
            break;
        case RELOP_NE:
        case RELOP_NE_BANG:
            value2 = parse_step2();
            if (value1 != value2)
                return 1;
            break;
        case RELOP_GT:
            value2 = parse_step2();
            if (value1 > value2)
                return 1;
            break;
        case RELOP_EQ:
            value2 = parse_step2();
            if (value1 == value2)
                return 1;
            break;
        case RELOP_LE:
            value2 = parse_step2();
            if (value1 <= value2)
                return 1;
            break;
        case RELOP_LT:
            value2 = parse_step2();
            if (value1 < value2)
                return 1;
            break;
	}
	return 0;
}

// ----------------------------------------------------------------------------
// additions and subtractions
// ----------------------------------------------------------------------------
int16_t parse_step2 (void)
{
	int16_t value1, value2;
	if (*txtpos == '-' || *txtpos == '+')
		value1 = 0;
	else
		value1 = parse_step3();
	while (1) {
		if (*txtpos == '-') {
			txtpos++;
			value2 = parse_step3();
			value1 -= value2;
		} else if (*txtpos == '+') {
			txtpos++;
			value2 = parse_step3();
			value1 += value2;
		} else
            return value1;
	}
}

// ----------------------------------------------------------------------------
// multiplications and divisions
// ----------------------------------------------------------------------------
int16_t parse_step3 (void)
{
	int16_t value1, value2;
	value1 = parse_step4();
	ignorespace();
	while (1) {
		if (*txtpos == '*') {
			txtpos++;
			value2 = parse_step4();
			value1 *= value2;
		} else if (*txtpos == '/') {
			txtpos++;
			value2 = parse_step4();
			if (value2 != 0)
				value1 /= value2;
			else
				error_code = 0xB;
		} else
            return value1;
	}
}

// ----------------------------------------------------------------------------
// numbers, variables and functions
// ----------------------------------------------------------------------------
int16_t parse_step4 (void)
{
	int16_t value1 = 0;
	int16_t value2 = 0;
	/////////////////////////////////////////////////////////////////////////// numbers
	ignorespace();
	// check for minus sign
	if (*txtpos == '-') {
		txtpos++;
		return -parse_step4();
	}
	// leading zeros are not allowed
	if (*txtpos == '0') {
		txtpos++;
		return 0;
	}
	// calculate value of given number
	if (*txtpos >= '1' && *txtpos <= '9') {
		do {
			value1 = value1 * 10 + *txtpos - '0';
			txtpos++;
		} while (*txtpos >= '0' && *txtpos <= '9');
		return value1;
	}
	/////////////////////////////////////////////////////////////////////////// variables
	// check if we have letters
	if (txtpos[0] >= 'A' && txtpos[0] <= 'Z') {
		// check next character -- variable names are single letters
		if (txtpos[1] < 'A' || txtpos[1] > 'Z') {
			// return a pointer to the referenced variable
			value2 = ((int16_t *)variables_begin)[*txtpos - 'A'];
			txtpos++;
			return value2;
		}
		/////////////////////////////////////////////////////////////////////////// functions
		// search table with function names
		scantable (functions);
		if (table_index == FN_UNKNOWN) {
			error_code = 0xE;
			return 0;
		}
		uint8_t func_ptr = table_index;
		// check for left parenthesis
		if (*txtpos != '(') {
			error_code = 0x5;
			return 0;
		}
		txtpos++;
		// get parameter
		value1 = parse_step1();
		// check for right parenthesis
		if (*txtpos != ')') {
			error_code = 0x6;
			return 0;
		}
		txtpos++;
		switch (func_ptr) {
		//-----------------------------------------------------------------
		case FN_PEEK:
			if (value1 > MEMORY_SIZE) {
				error_code = 0x13;
				return 0;
			}
			return program[value1];
		//-----------------------------------------------------------------
		case FN_ABS:
			if (value1 < 0)
                return -value1;
			return value1;
		//-----------------------------------------------------------------
		case FN_RND:
			return rand() % value1;
		//-----------------------------------------------------------------
		case FN_PINDREAD:
			// expected a pin number (0..7)
			if (value1 < 0 || value1 > 7) {
				error_code = 0xC;
				return 0;
			}
			// create bit mask for following checks
			value1 = 1 << value1;
			// selected pin should be configured as input
			if (sec_data_bus_dir & value1) {
				error_code = 0xD;
				return 0;
			}
			// get the digital value
			if (sec_data_bus_in & value1)
                return 1;
			return 0;
		//-----------------------------------------------------------------
		case FN_PINAREAD:
			// expected a pin number (0..7)
			if (value1 < 0 || value1 > 7) {
				error_code = 0xC;
				return 0;
			}
			// enable specified input channel
			ADMUX = value1;
			// create bit mask for following checks
			value1 = 1 << value1;
			// disable pull-up on selected pin
			value2 = sec_data_bus_out;
			sec_data_bus_out &= ~value1;
			// selected pin should be configured as input
			if (sec_data_bus_dir & value1) {
				error_code = 0xD;
				return 0;
			}
			// get the analog value
			ADCSRA |= _BV (ADSC);
			while (ADCSRA & _BV (ADSC));
			// restore state of pull-up
			sec_data_bus_out = value2;
			return ADCW >> 1;
		}
	}
// ---------------------------------------------------------------------------- expression in parenthesis
	if (*txtpos == '(') {
		txtpos++;
		value1 = parse_step1();
		if (*txtpos != ')') {
			error_code = 0x6;
			return 0;
		}
		txtpos++;
		return value1;
	}
}

/*
warm_reset()
{
	// turn-on cursor
	putchar (vid_cursor_on);
	// turn-on scroll
	putchar (vid_scroll_on);
	// reset program-memory pointer
	current_line = 0;
	sp = program + MEMORY_SIZE;
	printmsg (msg_ok, stdout);

    while(1) {
        if autorun: EXECLINE
        get new line
        if no line number:  return DIRECT
        on error:           return ERROR_MESSAGE
        on just delete loop
        on prog merge loop
    }
}
*/

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
	get_line ('>');
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
		goto error_message;
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

/////////////////////////////////////////////////////////////////////////////// print error message
error_message:
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
	goto warm_reset;



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
			goto error_message;
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
			goto error_message;
		}
		if (val != 0) goto start_interpretation;
		goto execnextline;
	case CMD_GOTO:
		linenum = parse_step1();
		if (error_code || *txtpos != LF) {
			error_code = 0x4;
			goto error_message;
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
			goto error_message;
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
		goto error_message;
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
			goto error_message;
		}
		var = *txtpos;
		txtpos++;
		ignorespace();
		if (*txtpos != '=') {
			error_code = 0xA;
			goto error_message;
		}
		txtpos++;
		ignorespace();
		error_code = 0;
		initial = parse_step1();
		if (error_code)
            goto error_message;
		scantable (to_tab);
		if (table_index != 0) {
			error_code = 0x2;
			goto error_message;
		}
		terminal = parse_step1();
		if (error_code)
            goto error_message;
		scantable (step_tab);
		if (table_index == 0) {
			step = parse_step1();
			if (error_code)
                goto error_message;
		} else step = 1;
		ignorespace();
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			goto error_message;
		}
		if (!error_code && *txtpos == LF) {
			struct stack_for_frame *f;
			if (sp + sizeof (struct stack_for_frame) < stack_limit) {
				error_code = 0x3;
				goto error_message;
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
	goto error_message;
gosub:
	error_code = 0;
	linenum = parse_step1();
	if (!error_code && *txtpos == LF) {
		struct stack_gosub_frame *f;
		if (sp + sizeof (struct stack_gosub_frame) < stack_limit) {
			error_code = 0x3;
			goto error_message;
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
	goto error_message;
next:
	// find the variable name
	ignorespace();
	if (*txtpos < 'A' || *txtpos > 'Z') {
		error_code = 0x7;
		goto error_message;
	}
	txtpos++;
	ignorespace();
	if (*txtpos != ':' && *txtpos != LF) {
		error_code = 0x2;
		goto error_message;
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
	goto error_message;
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
			goto error_message;
		}
		var = (int16_t *)variables_begin + *txtpos - 'A';
		// check for proper statement termination
		txtpos++;
		ignorespace();
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			goto error_message;
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
			goto error_message;
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
			goto error_message;
		} else {
			// check if variable name is rendered invalid
			if (error_code == 0xFF) {
				error_code = 0x11;
				goto error_message;
			}
		}
		txtpos++;
		ignorespace();
		value = parse_step1();
		if (error_code)
            goto error_message;
		// Check that we are at the end of the statement
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			goto error_message;
		}
		*var = value;
	}
	goto run_next_statement;
poke: {
		int16_t value, address;
		// get the address
		address = parse_step1();
		if (error_code)
            goto error_message;
		if (address > MEMORY_SIZE) {
			error_code = 0x13;
			goto error_message;
		}
		// check for comma
		ignorespace();
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get the value to assign
		ignorespace();
		value = parse_step1();
		if (error_code)
            goto error_message;
		if (value < 0 || value > 255) {
			error_code = 0x12;
			goto error_message;
		}
		// check for proper statement termination
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
			goto error_message;
		}
		// assign value to specified location in memory
		program[address] = value;
	}
	goto run_next_statement;
pset: {
		uint16_t x, y, col;
		// get x-coordinate
		x = parse_step1();
		if (error_code)
			goto error_message;
		if (x < 0 || x > 255) {
			error_code = 0x10;
			goto error_message;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get y-coordinate
		y = parse_step1();
		if (error_code)
			goto error_message;
		if (y < 0 || y > 239) {
			error_code = 0x10;
			goto error_message;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get color
		col = parse_step1();
		if (error_code)
			goto error_message;
		if (col < 0 || col > 127) {
			error_code = 0x14;
			goto error_message;
		}
		put_pixel ((uint8_t)x, (uint8_t)y, (uint8_t)col);
	}
	goto execnextline;
/////////////////////////////////////////////////////////////////////////////// pin control
pindir: {
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code)
            goto error_message;
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
			goto error_message;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get direction [0/1]
		b = parse_step1();
		if (error_code)
            goto error_message;
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
			goto error_message;
			break;
		}
	}
	goto execnextline;
pindwrite: {
		uint16_t a, b;
		// get pin number [0..7]
		a = parse_step1();
		if (error_code)
            goto error_message;
		// check range
		if (a < 0 || a > 7) {
			error_code = 0xC;
			goto error_message;
		}
		// check for comma -- two parameters expected
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get value [0/1]
		b = parse_step1();
		if (error_code)
            goto error_message;
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
			goto error_message;
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
		if (error_code)
            goto error_message;
		if (col < 0 || col > 127) {
			error_code = 0x14;
			goto error_message;
		}
		text_color ((uint8_t)col);
	}
	goto execnextline;
paper: {
		uint16_t col;
		// get color value
		col = parse_step1();
		if (error_code)
            goto error_message;
		if (col < 0 || col > 127) {
			error_code = 0x14;
			goto error_message;
		}
		paper_color ((uint8_t)col);
	}
	goto execnextline;
locate: {
		uint16_t line, column;
		// get target line
		line = parse_step1();
		if (error_code)
            goto error_message;
		if (line < 0 || line > 23) {
			error_code = 0x10;
			goto error_message;
		}
		// check for comma
		if (*txtpos != ',') {
			error_code = 0x2;
			goto error_message;
		}
		txtpos++;
		// get target line
		column = parse_step1();
		if (error_code)
            goto error_message;
		if (column < 0 || column > 31) {
			error_code = 0x10;
			goto error_message;
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
			goto error_message;
		} else {
			uint16_t e;
			error_code = 0;
			e = parse_step1();
			if (error_code)
                goto error_message;
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
			goto error_message;
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
		if (error_code)
			goto error_message;
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
			goto error_message;
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
				goto error_message;
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
		goto error_message;
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
		//for( i = 0; ( i < ( E2END + 1 ) ) && ( val != '\0' ); i++ ) val = eeprom_read_byte( (uint8_t *)i );
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
		if (error_code)
            goto error_message;
		srand (param);
	}
	goto run_next_statement;
}
