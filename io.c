// ----------------------------------------------------------------------------
// Peripheral interfacing functions (GPU, APU, PS2 keyboard, serial port)
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

#include "io.h"

static void kb_decode (uint8_t sc);
static void kb_to_buffer (uint8_t chr);

// streams of data to and from IO devices
FILE stream_physical = FDEV_SETUP_STREAM (putchar_phy, getchar_phy, _FDEV_SETUP_RW);
FILE stream_pseudo = FDEV_SETUP_STREAM (putchar_ser, getchar_ser, _FDEV_SETUP_RW);
FILE stream_eeprom = FDEV_SETUP_STREAM (putchar_rom, getchar_rom, _FDEV_SETUP_RW);

// keyboard connectivity messages
const uint8_t kb_fail_msg[26] PROGMEM = "Keyboard self-test failed\0";
const uint8_t kb_success_msg[32] PROGMEM = "Keyboard connected successfully\0";

// Array for translating scan codes to ASCII
//	1st column:	SHIFT=0 CAPS=0
//	2nd column: SHIFT=1 CAPS=0
//	3th column: SHIFT=0 CAPS=1
//	4th column: SHIFT=1 CAPS=1
const uint8_t to_ascii[512] PROGMEM = {
	0, 0, 0, 0,			// 00
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	'`', '~', '`', '~',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 08
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	TAB, TAB, TAB, TAB,
	'`', '~', '`', '~',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 10
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	'q', 'Q', 'Q', 'q',
	'1', '!', '1', '!',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 18
	0, 0, 0, 0,
	'z', 'Z', 'Z', 'z',
	's', 'S', 'S', 's',
	'a', 'A', 'A', 'a',
	'w', 'W', 'W', 'w',
	'2', '@', '2', '@',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 20
	'c', 'C', 'C', 'c',
	'x', 'X', 'X', 'x',
	'd', 'D', 'D', 'd',
	'e', 'E', 'E', 'e',
	'4', '$', '4', '$',
	'3', '#', '3', '#',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 28
	' ', ' ', ' ', ' ',
	'v', 'V', 'V', 'v',
	'f', 'F', 'F', 'f',
	't', 'T', 'T', 't',
	'r', 'R', 'R', 'r',
	'5', '%', '5', '%',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 30
	'n', 'N', 'N', 'n',
	'b', 'B', 'B', 'b',
	'h', 'H', 'H', 'h',
	'g', 'G', 'G', 'g',
	'y', 'Y', 'Y', 'y',
	'6', '^', '6', '^',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 38
	',', '<', ',', '<',
	'm', 'M', 'M', 'm',
	'j', 'J', 'J', 'j',
	'u', 'U', 'U', 'u',
	'7', '&', '7', '&',
	'8', '*', '8', '*',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 40
	',', '<', ',', '<',
	'k', 'K', 'K', 'k',
	'i', 'I', 'I', 'i',
	'o', 'O', 'O', 'o',
	'0', ')', '0', ')',
	'9', '(', '9', '(',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 48
	'.', '>', '.', '>',
	'/', '?', '/', '?',
	'l', 'L', 'L', 'l',
	';', ':', ';', ':',
	'p', 'P', 'P', 'p',
	'-', '_', '-', '_',
	0, 0, 0, 0,
	0, 0, 0, 0,			// 50
	0, 0, 0, 0,
	'\'', '"', '\'', '"',
	0, 0, 0, 0,
	'[', '{', '[', '{',
	'=', '+', '=', '+',
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,			// 58
	0, 0, 0, 0,
	CR, CR, CR, CR,
	']', '}', ']', '}',
	0, 0, 0, 0,
	'\\', '|', '\\', '|',
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,			// 60
	'<', '<', '<', '<',
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	BS, BS, BS, BS,
	0, 0, 0, 0,
	0, 0, 0, 0,			// 68
	'1', '1', '1', '1',
	0, 0, 0, 0,
	'4', '4', '4', '4',
	'7', '7', '7', '7',
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	'0', '0', '0', '0',	// 70
	'.', '.', '.', '.',
	'2', '2', '2', '2',
	'5', '5', '5', '5',
	'6', '6', '6', '6',
	'8', '8', '8', '8',
	ESC, ESC, ESC, ESC,
	0, 0, 0, 0,
	0, 0, 0, 0,			// 78
	'+', '+', '+', '+',
	'3', '3', '3', '3',
	'-', '-', '-', '-',
	'*', '*', '*', '*',
	'9', '9', '9', '9',
	0, 0, 0, 0,
	0, 0, 0, 0,
};


// ----------------------------------------------------------------------------
// translation of keyboard scan codes to ASCII
// ----------------------------------------------------------------------------
static void kb_decode (uint8_t sc)
{
	static uint8_t kb_status = 0;
	uint8_t tmp;
	if (! (kb_status & BREAKCODE)) {	// a key was pressed and/or held...
		switch (sc) {
            case 0xAA:					// Basic Assurance Test (BAT) succeeded
                break;
            case 0xFC:					// Basic Assurance Test (BAT) failed
                printmsg (kb_fail_msg, stdout);
                do_beep();
                do_beep();
                break;
            case 0xE0:					// extended key make-code
                kb_status |= EXTENDEDKEY;
                break;
            case 0xF0:					// break-code
                kb_status |= BREAKCODE;
                kb_status &= ~EXTENDEDKEY;
                break;
            case 0x58:					// Caps Lock
                kb_status ^= CAPSLOCK;
                break;
            case 0x77:					// Num Lock
                kb_status ^= NUMLOCK;
                break;
            case 0x14:					// left CONTROL
                kb_status |= CONTROL;
                break;
            case 0x12:					// left SHIFT
            case 0x59:					// right SHIFT
                kb_status |= SHIFT;
                break;
            default:
                // if CONTROL key is pressed ----------------------------------
                if (kb_status & CONTROL) {
                    if (sc == 0x21) break_flow = 1;			// CTRL+C
                    if (sc == 0x34) {						// CTRL+G
                        // this has to be done right away,
                        // since "beep" function disables keyboard ISR
                        kb_status &= ~CONTROL;
                        kb_to_buffer (BELL);
                    }
                    if (sc == 0x4B) kb_to_buffer (FF);		// CTRL+L
                    if (sc == 0x1C) kb_to_buffer (HOME);	// CTRL+A
                    if (sc == 0x24) kb_to_buffer (END);		// CTRL+E
                }
                // if an EXTENDED KEY is pressed ------------------------------
                else if (kb_status & EXTENDEDKEY) {
                    if (sc == 0x5A) kb_to_buffer (CR);		// ENTER
                    if (sc == 0x75) kb_to_buffer (ARUP);	// ARROW UP
                    if (sc == 0x72) kb_to_buffer (ARDN);	// ARROW DOWN
                    if (sc == 0x6B) kb_to_buffer (ARLT);	// ARROW LEFT
                    if (sc == 0x74) kb_to_buffer (ARRT);	// ARROW RIGHT
                    if (sc == 0x6C) kb_to_buffer (HOME);	// HOME
                    if (sc == 0x69) kb_to_buffer (END);		// END
                    kb_status &= ~EXTENDEDKEY;
                }
                // in any other case ------------------------------------------
                else {
                    tmp = 0;
                    if (kb_status & SHIFT)
                        tmp += 1;
                    if (kb_status & CAPSLOCK)
                        tmp += 2;
                    kb_to_buffer (pgm_read_byte (to_ascii + 4 * sc + tmp));
                }
                break;
		}
	} else {							// a pressed key was just released...
		kb_status &= ~BREAKCODE;
		switch (sc) {
            case 0x14:					// left CONTROL
                kb_status &= ~CONTROL;
                break;
            case 0x12:					// left SHIFT
            case 0x59:					// right SHIFT
                kb_status &= ~SHIFT;
                break;
		}
	}
}

// ----------------------------------------------------------------------------
// write to keyboard buffer
// ----------------------------------------------------------------------------
static void kb_to_buffer (uint8_t chr)
{
	// only proceed if keyboard buffer is not full
	if (kb_buffer_cnt < KB_BUFFER_SIZE) {
		// store incoming byte in keyboard buffer
		kb_buffer[kb_write_ptr] = chr;
		// update buffer pointer and data counter
		kb_write_ptr++;
		kb_buffer_cnt++;
		// pointer wrapping
		if (kb_write_ptr == KB_BUFFER_SIZE)
			kb_write_ptr = 0;
	} else
		do_beep();	// should not beep while running a program !!!
}

// ----------------------------------------------------------------------------
// IO initialization
// ----------------------------------------------------------------------------
void init_io (void)
{
	// setup fundamental stream
	stdout = stdin = &stream_physical;
	// configure analog to digital converter
	ADMUX = 0;
	ADCSRA = _BV (ADEN) | _BV (ADPS2) | _BV (ADPS1) | _BV (ADPS0);
	// do the first conversion (initialization)
	ADCSRA |= _BV (ADSC);
	while (ADCSRA & _BV (ADSC));
	// configure secondary data bus pins (inputs with pull-up resistors)
	sec_data_bus_dir = 0;
	sec_data_bus_out = 255;
	// configure buzzer and LED pin
	aux_ctl_bus_dir |= buzzer_led;
	aux_ctl_bus_out |= buzzer_led;
	// setup keyboard connection
	init_kb();
	// initial data bus value
	pri_data_bus_dir = 255;
	// setup GPU control pins
	peripheral_bus_dir &= ~from_gpu;
	peripheral_bus_dir |= to_gpu;
	// setup APU control pins
	peripheral_bus_dir &= ~from_apu;
	peripheral_bus_dir |= to_apu;
	// display "boot" message
	//putchar( vid_reset );
	// setup UART connection
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV (UCSZ01) | _BV (UCSZ00);	// 8bit data
	UCSR0B = _BV (RXEN0) | _BV (TXEN0);	// enable RX - TX
	uart_ansi_rst_clr();
}



// ----------------------------------------------------------------------------
// setup communication with keyboard
// ----------------------------------------------------------------------------
void init_kb (void)
{
	//enable emergency break key (INT2)
	aux_ctl_bus_dir	&= ~break_key;		// configure relevant pin as input
	aux_ctl_bus_out	|= break_key;		// enable pull up resistor
	EIMSK |= BREAK_INT;					// enable INT2 interrupt
	// setup clock and data pins as inputs
	peripheral_bus_dir &= ~kb_dat_pin;
	peripheral_bus_dir &= ~kb_clk_pin;
	peripheral_bus_out |= kb_dat_pin;
	peripheral_bus_out |= kb_clk_pin;
	// setup timer0 for keyboard time-out
	TCCR0A = _BV (WGM01);
	TCCR0B = 0;
	OCR0A = 100;
	TIMSK0 = _BV (OCIE0A);
	break_flow = 0;
	// bit counter
	kb_bit_cnt = 11;
	// enable keyboard transmit interrupt (INT0)
	EICRA = 2;
	edge = 0;
	EIMSK |= KEYBOARD_INT;
	sei();
}

// ----------------------------------------------------------------------------
// ISR for receiving BREAK signal
// ----------------------------------------------------------------------------
ISR (INT2_vect)
{
	// signal program-break
	break_flow = 1;
	// disable emergency break key
	EIMSK &= ~BREAK_INT;
}

// ----------------------------------------------------------------------------
// ISR for deleting residual bits
// ----------------------------------------------------------------------------
ISR (TIMER0_COMPA_vect)   // , ISR_NAKED )
{
	// reset bit counter
	kb_bit_cnt = 11;
	// stop timer
	TCCR0B = 0;
}

// ----------------------------------------------------------------------------
// ISR for reading and storing incoming bits (PS2 clock)
// ----------------------------------------------------------------------------
ISR (INT0_vect)
{
	uint8_t bit_val;						// incoming bit
	static uint8_t raw_data;				// received scan code
	// get bit value as quickly as possible!
	bit_val = peripheral_bus_in;
	bit_val &= kb_dat_pin;
	if (! edge) {
		// start timer
		if (kb_bit_cnt == 11)
            TCCR0B = _BV (CS02) | _BV (CS00);
		if (kb_bit_cnt < 11 && kb_bit_cnt > 2) {	// Bits 3 to 10 are useful data.
			// Parity, start and stop bits are ignored.
			raw_data = (raw_data >> 1);
			if (bit_val) raw_data = raw_data | 0x80;
		}
		EICRA = 3;								// set interrupt on rising edge
		edge = 1;								// 1: rising edge
	} else {
		kb_bit_cnt--;
		if (kb_bit_cnt == 0) {				// when all bits are received
			kb_decode (raw_data);
			kb_bit_cnt = 11;
		}
		EICRA = 2;								// set interrupt on falling edge
		edge = 0;								// 0: falling edge
	}
}

// ----------------------------------------------------------------------------
// UART terminal reset
// ----------------------------------------------------------------------------
void uart_ansi_rst_clr (void)
{
	// ANSI reset
	fputc (27, &stream_pseudo);
	fputc ('[', &stream_pseudo);
	fputc ('0', &stream_pseudo);
	fputc ('m', &stream_pseudo);
	// ANSI clear
	fputc (27, &stream_pseudo);
	fputc ('[', &stream_pseudo);
	fputc ('H', &stream_pseudo);
	fputc (27, &stream_pseudo);
	fputc ('[', &stream_pseudo);
	fputc ('J', &stream_pseudo);
}

// ----------------------------------------------------------------------------
// UART terminal cursor movement
// ----------------------------------------------------------------------------
void uart_ansi_move_cursor (uint8_t row, uint8_t col)
{
	fputc (27, &stream_pseudo);
	fputc ('[', &stream_pseudo);
	fprintf (&stream_pseudo, "%d", row);
	fputc (';', &stream_pseudo);
	fprintf (&stream_pseudo, "%d", col);
	fputc ('H', &stream_pseudo);
}

// ----------------------------------------------------------------------------
// send character through UART
// ----------------------------------------------------------------------------
int putchar_ser (char chr, FILE *stream)
{
	if (chr == LF)
        putchar_ser (CR, stream);
	loop_until_bit_is_set (UCSR0A, UDRE0);
	UDR0 = chr;
    return 0;
}

// ----------------------------------------------------------------------------
// get character through UART
// ----------------------------------------------------------------------------
int getchar_ser (FILE *stream)
{
	uint8_t chr;
	loop_until_bit_is_set (UCSR0A, RXC0);
	chr = UDR0;
	return chr;
}

// ----------------------------------------------------------------------------
// send character to VGA and UART (STDOUT)
// ----------------------------------------------------------------------------
int putchar_phy (char chr, FILE *stream)
{
	// send to VGA
	pri_data_bus_out = chr;
	vgaready();
	tovga();
	// send to UART
	if (chr < 128) {
		if (chr == BS) {
			fputc (BS, &stream_pseudo);
			fputc (SPACE, &stream_pseudo);
			fputc (BS, &stream_pseudo);
		} else
            fputc (chr , &stream_pseudo);
	}
	return 0;
}

// ----------------------------------------------------------------------------
// get character from keyboard (STDIN)
// ----------------------------------------------------------------------------
int getchar_phy (FILE *stream)
{
	uint8_t chr;
	// wait for a key
	while (kb_buffer_cnt == 0)
        fx_delay_ms (15);
	// read key from keyboard buffer
	chr = kb_buffer[kb_read_ptr];
	// update buffer pointer and data counter
	kb_read_ptr++;
	kb_buffer_cnt--;
	// pointer wrapping
	if (kb_read_ptr == KB_BUFFER_SIZE)
		kb_read_ptr = 0;
	return chr;
}

// ----------------------------------------------------------------------------
// send character to APU
// ----------------------------------------------------------------------------
void send_to_apu (uint8_t cbyte)
{
	pri_data_bus_out = cbyte;
	apuready();
	toapu();
}

// ----------------------------------------------------------------------------
// send character to EEPROM
// ----------------------------------------------------------------------------
int putchar_rom (char chr, FILE *stream)
{
	eeprom_update_byte ((uint8_t *) eeprom_ptr++, chr);
	return 0;
}

// ----------------------------------------------------------------------------
// get character from EEPROM
// ----------------------------------------------------------------------------
int getchar_rom (FILE *stream)
{
	uint8_t chr = eeprom_read_byte ((uint8_t *) eeprom_ptr++);
	return chr;
}

// ----------------------------------------------------------------------------
// set text color
// ----------------------------------------------------------------------------
void text_color (uint8_t color)
{
	putchar (vid_color);
	putchar (color);
}

// ----------------------------------------------------------------------------
// set background color
// ----------------------------------------------------------------------------
void paper_color (uint8_t color)
{
	putchar (vid_paper);
	putchar (color);
}

// ----------------------------------------------------------------------------
// move cursor to specified location
// ----------------------------------------------------------------------------
void locate_cursor (uint8_t line, uint8_t column)
{
	putchar (vid_locate);
	putchar (line);
	putchar (column);
}

// ----------------------------------------------------------------------------
// put pixel at specified coordinates with given color
// ----------------------------------------------------------------------------
void put_pixel (uint8_t x, uint8_t y, uint8_t color)
{
	putchar (vid_pixel);
	putchar (x);
	putchar (y);
	putchar (color);
}
