#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include "main.h"

/**
 ** prototypes of non-static functions
 **
 **/

void init_io (void);
void init_kb (void);

void uart_ansi_rst_clr (void);
void uart_ansi_move_cursor (uint8_t row, uint8_t col);

void text_color (uint8_t color);
void paper_color (uint8_t color);
void locate_cursor (uint8_t line, uint8_t column);
void put_pixel (uint8_t x, uint8_t y, uint8_t color);

void send_to_apu (uint8_t cbyte);

// can not use uintxx_t because of how FILE is defined
int putchar_ser (char c, FILE *stream);
int getchar_ser (FILE *stream);
int putchar_phy (char c, FILE *stream);
int getchar_phy (FILE *stream);
int putchar_rom (char c, FILE *stream);
int getchar_rom (FILE *stream);

/**
 ** definition of constants and custom data types
 **
 **/

#define pri_data_bus_dir DDRC
#define pri_data_bus_out PORTC
#define pri_data_bus_in PINC
#define sec_data_bus_dir DDRA
#define sec_data_bus_out PORTA
#define sec_data_bus_in PINA
#define peripheral_bus_dir DDRD
#define peripheral_bus_out PORTD
#define peripheral_bus_in PIND
#define aux_ctl_bus_dir DDRB
#define aux_ctl_bus_out PORTB
#define aux_ctl_bus_in PINB

#define to_gpu			128	// 8th bit (PD7)
#define from_gpu		64	// 7th bit (PD6)
#define to_apu			32	// 6th bit (PD5)
#define from_apu		16	// 5th bit (PD4)
#define buzzer_led		1	// 1st bit (PB0)
#define break_key		4	// 3rd bit (PB2)

#define kb_clk_pin		4	// 3rd bit (PD2)
#define kb_dat_pin		8	// 4th bit (PD3)

#define KEYBOARD_INT	1	// INT0 
#define BREAK_INT		4	// INT2

#define KB_BUFFER_SIZE  16

// keyboard status bits
#define BREAKCODE	    1
#define EXTENDEDKEY	    2
#define CONTROL		    4
#define NUMLOCK		    8
#define SHIFT		    16
#define CAPSLOCK	    32
// ASCII special characters
#define LF		0x0A		// ENTER
#define FF		0x0C		// CTRL+L
#define CR		0x0D		// RETURN
#define TAB		0x09		// TAB
#define BELL	0x07		// CTRL+G
#define SPACE	0x20		// SPACE
#define BS		0x08		// BACKSPACE
#define ESC		0x1B		// ESC
#define ETX		0x03		// CTRL+C
#define DQUOTE	0x22		//
#define SQUOTE	0x27		//
// other special characters (arbitrarily defined)
#define HOME	0x01		// HOME
#define END		0x02		// END
#define ARUP	0x04		// ARROW UP
#define ARDN	0x05		// ARROW DOWN
#define ARLT	0x10		// ARROW LEFT
#define ARRT	0x11		// ARROW RIGHT
// GPU special characters
#define vid_tosol	    1
#define vid_toeol	    2
#define vid_tolft	    16
#define vid_torgt	    17
#define vid_toup	    19
#define vid_todn	    20
// GPU directives
#define vid_reset       200
#define vid_clear       201
#define vid_pixel       202
#define vid_line        203
#define vid_box         204
#define vid_locate	    205
#define vid_color 	    206
#define vid_paper 	    207
#define vid_cursor_off  208
#define vid_cursor_on   209
#define vid_scroll_off  210
#define vid_scroll_on   211
// APU directives
#define snd_play        207
#define snd_stop        206
#define snd_notes       205
#define snd_tempo       204
#define snd_clr         203
#define snd_dis         202
#define snd_ena         201
#define snd_abort       200

/**
 ** declarations of global variables
 **
 **/

// streams of data to and from IO devices
FILE stream_physical, stream_pseudo, stream_eeprom;

// global variables
uint16_t eeprom_ptr;
uint8_t break_flow;

// keyboard connectivity messages
const uint8_t kb_fail_msg[26];
const uint8_t kb_success_msg[32];

/**
 ** assembler macros
 **
 **/

#define tovga() asm volatile \
	( \
		"sbi 0x0b, 7 \n\t" \
		"sbis 0x09, 6 \n\t" \
		"rjmp .-4 \n\t" \
		"cbi 0x0b, 7 \n\t" \
	)

#define vgaready() asm volatile \
	( \
		"sbic 0x09, 6 \n\t" \
		"rjmp .-4 \n\t" \
	)

#define toapu() asm volatile \
	( \
		"sbi 0x0b, 5 \n\t" \
		"sbis 0x09, 4 \n\t" \
		"rjmp .-4 \n\t" \
		"cbi 0x0b, 5 \n\t" \
	)

#define apuready() asm volatile \
	( \
		"sbic 0x09, 4 \n\t" \
		"rjmp .-4 \n\t" \
	)

#define delay1us() asm volatile \
	( \
		"nop \n\t nop \n\t nop \n\t nop \n\t" \
		"nop \n\t nop \n\t nop \n\t nop \n\t" \
		"nop \n\t nop \n\t nop \n\t nop \n\t" \
		"nop \n\t nop \n\t nop \n\t nop \n\t" \
		"nop \n\t nop \n\t nop \n\t nop \n\t" \
	)

#endif