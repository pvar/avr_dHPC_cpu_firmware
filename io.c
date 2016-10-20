/*
 * Peripheral interfacing functions for nstBASIC.
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

/**
 * @file io.c
 * @brief Exchange data with sub-systems and peripherals.
 *
 * The functions in this file handle all IO transactions with EEPROM and serial port,
 * decode data received from keyboard and transmit commands or raw data to the VGA
 * controller and / or the sound synthesizer.
 * @note In order to port nstBASIC to some other platform, this file will probably
 * have to be rewritten from scratch!
 *
*/

#include "io.h"

FILE stream_physical = FDEV_SETUP_STREAM (putchar_phy, getchar_phy, _FDEV_SETUP_RW);
FILE stream_serial = FDEV_SETUP_STREAM (putchar_ser, getchar_ser, _FDEV_SETUP_RW);
FILE stream_eeprom = FDEV_SETUP_STREAM (putchar_rom, getchar_rom, _FDEV_SETUP_RW);

static uint8_t edge, kb_bit_cnt;
static uint8_t kb_buffer_cnt;
static uint8_t kb_buffer[KB_BUFFER_SIZE];

// Array for the translation of keyboard scan codes to ASCII
// 1st col: ASCII code when: SHIFT = 0 & CAPS = 0
// 2nd col: ASCII code when: SHIFT = 1 & CAPS = 0
// 3rd col: ASCII code when: SHIFT = 0 & CAPS = 1
// 4th col: ASCII code when: SHIFT = 1 & CAPS = 1
static const uint8_t to_ascii[512] PROGMEM = {
        0, 0, 0, 0,             // 00
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        '`', '~', '`', '~',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 08
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        TAB, TAB, TAB, TAB,
        '`', '~', '`', '~',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 10
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        'q', 'Q', 'Q', 'q',
        '1', '!', '1', '!',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 18
        0, 0, 0, 0,
        'z', 'Z', 'Z', 'z',
        's', 'S', 'S', 's',
        'a', 'A', 'A', 'a',
        'w', 'W', 'W', 'w',
        '2', '@', '2', '@',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 20
        'c', 'C', 'C', 'c',
        'x', 'X', 'X', 'x',
        'd', 'D', 'D', 'd',
        'e', 'E', 'E', 'e',
        '4', '$', '4', '$',
        '3', '#', '3', '#',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 28
        ' ', ' ', ' ', ' ',
        'v', 'V', 'V', 'v',
        'f', 'F', 'F', 'f',
        't', 'T', 'T', 't',
        'r', 'R', 'R', 'r',
        '5', '%', '5', '%',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 30
        'n', 'N', 'N', 'n',
        'b', 'B', 'B', 'b',
        'h', 'H', 'H', 'h',
        'g', 'G', 'G', 'g',
        'y', 'Y', 'Y', 'y',
        '6', '^', '6', '^',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 38
        ',', '<', ',', '<',
        'm', 'M', 'M', 'm',
        'j', 'J', 'J', 'j',
        'u', 'U', 'U', 'u',
        '7', '&', '7', '&',
        '8', '*', '8', '*',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 40
        ',', '<', ',', '<',
        'k', 'K', 'K', 'k',
        'i', 'I', 'I', 'i',
        'o', 'O', 'O', 'o',
        '0', ')', '0', ')',
        '9', '(', '9', '(',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 48
        '.', '>', '.', '>',
        '/', '?', '/', '?',
        'l', 'L', 'L', 'l',
        ';', ':', ';', ':',
        'p', 'P', 'P', 'p',
        '-', '_', '-', '_',
        0, 0, 0, 0,
        0, 0, 0, 0,             // 50
        0, 0, 0, 0,
        '\'', '"', '\'', '"',
        0, 0, 0, 0,
        '[', '{', '[', '{',
        '=', '+', '=', '+',
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,             // 58
        0, 0, 0, 0,
        CR, CR, CR, CR,
        ']', '}', ']', '}',
        0, 0, 0, 0,
        '\\', '|', '\\', '|',
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,             // 60
        '<', '<', '<', '<',
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        BS, BS, BS, BS,
        0, 0, 0, 0,
        0, 0, 0, 0,             // 68
        '1', '1', '1', '1',
        0, 0, 0, 0,
        '4', '4', '4', '4',
        '7', '7', '7', '7',
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        '0', '0', '0', '0',     // 70
        '.', '.', '.', '.',
        '2', '2', '2', '2',
        '5', '5', '5', '5',
        '6', '6', '6', '6',
        '8', '8', '8', '8',
        ESC, ESC, ESC, ESC,
        0, 0, 0, 0,
        0, 0, 0, 0,             // 78
        '+', '+', '+', '+',
        '3', '3', '3', '3',
        '-', '-', '-', '-',
        '*', '*', '*', '*',
        '9', '9', '9', '9',
        0, 0, 0, 0,
        0, 0, 0, 0,
};

/** ***************************************************************************
 * @brief Keyboard data decoding.
 *
 * This functions decodes the data transmitted by the keyboard and updates some
 * variables which hold he state of key modifiers (CAPSLOCK, NUMLOCK, SHIFT and
 * CTRL). The array to_ascii is only used for decoding the scancodes of normal
 * (printable) characters. The decoded data are placed in a special buffer,
 * with the help of put_kb_buffer() function.
 *****************************************************************************/
void kb_decode (uint8_t sc)
{
        static uint8_t kb_status = 0;
        uint8_t tmp;
        if (! (kb_status & BREAKCODE)) {        // a key was pressed and/or held...
                switch (sc) {
                        case 0xAA:              // Basic Assurance Test (BAT) succeeded
                                break;
                        case 0xFC:              // Basic Assurance Test (BAT) failed
                                printmsg (kb_fail_msg, stdout);
                                do_beep();
                                do_beep();
                                break;
                        case 0xE0:              // extended key make-code
                                kb_status |= EXTENDEDKEY;
                                break;
                        case 0xF0:              // break-code
                                kb_status |= BREAKCODE;
                                kb_status &= ~EXTENDEDKEY;
                                break;
                        case 0x58:              // Caps Lock
                                kb_status ^= CAPSLOCK;
                                break;
                        case 0x77:              // Num Lock
                                kb_status ^= NUMLOCK;
                                break;
                        case 0x14:              // left CONTROL
                                kb_status |= CONTROL;
                                break;
                        case 0x12:              // left SHIFT
                        case 0x59:              // right SHIFT
                                kb_status |= SHIFT;
                                break;
                        default:
                                // if CONTROL key is pressed ----------------------------------
                                if (kb_status & CONTROL) {
                                        if (sc == 0x21) break_flow = 1;         // CTRL+C
                                        if (sc == 0x34) {                       // CTRL+G
                                                // this has to be done right away,
                                                // since "beep" function disables keyboard ISR
                                                kb_status &= ~CONTROL;
                                                put_kb_buffer (BELL);
                                        }
                                        if (sc == 0x4B) put_kb_buffer (FF);     // CTRL+L
                                        if (sc == 0x1C) put_kb_buffer (HOME);   // CTRL+A
                                        if (sc == 0x24) put_kb_buffer (END);    // CTRL+E
                                }
                                // if an EXTENDED KEY is pressed ------------------------------
                                else if (kb_status & EXTENDEDKEY) {
                                        if (sc == 0x5A) put_kb_buffer (CR);     // ENTER
                                        if (sc == 0x75) put_kb_buffer (ARUP);   // ARROW UP
                                        if (sc == 0x72) put_kb_buffer (ARDN);   // ARROW DOWN
                                        if (sc == 0x6B) put_kb_buffer (ARLT);   // ARROW LEFT
                                        if (sc == 0x74) put_kb_buffer (ARRT);   // ARROW RIGHT
                                        if (sc == 0x6C) put_kb_buffer (HOME);   // HOME
                                        if (sc == 0x69) put_kb_buffer (END);    // END
                                        kb_status &= ~EXTENDEDKEY;
                                }
                                // in any other case ------------------------------------------
                                else {
                                        tmp = 0;
                                        if (kb_status & SHIFT)
                                                tmp += 1;
                                        if (kb_status & CAPSLOCK)
                                                tmp += 2;
                                        put_kb_buffer (pgm_read_byte (to_ascii + 4 * sc + tmp));
                                }
                                break;
                }
        } else {                        // a pressed key was just released...
                kb_status &= ~BREAKCODE;
                switch (sc) {
                        case 0x14:      // left CONTROL
                                kb_status &= ~CONTROL;
                                break;
                        case 0x12:      // left SHIFT
                        case 0x59:      // right SHIFT
                                kb_status &= ~SHIFT;
                                break;
                }
        }
}

/** ***************************************************************************
 * @brief Put incoming data to keyboard-buffer.
 *
 * This function puts incomming data to a special buffer for later use. When
 * the buffer is full, the system makes a beep sound.
 *****************************************************************************/
void put_kb_buffer (uint8_t chr)
{
        static uint8_t kb_write_ptr = 0;
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
                do_beep();
}

/** ***************************************************************************
 * @brief Microcontroiller peripheral initialization.
 *
 * This function configures the pins of the CPU (actually the microcontroller).
 * It prepares the ADC, initializes the keyboard and prints some messages.
 *****************************************************************************/
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
        aux_ctl_bus_out &= ~buzzer_led;

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

        // setup UART connection
        UBRR0H = UBRRH_VALUE;
        UBRR0L = UBRRL_VALUE;
        UCSR0C = _BV (UCSZ01) | _BV (UCSZ00);   // 8bit data
        UCSR0B = _BV (RXEN0) | _BV (TXEN0);     // enable RX - TX
        uart_ansi_rst_clr();
}

/** ***************************************************************************
 * @brief Setup communication with keyboard.
 *
 * This function configures the pins used for communicating with the keyboard
 * and enables an interrupt for receiving the incoming data.
 *****************************************************************************/
void init_kb (void)
{
        //enable emergency break key (INT2)
        aux_ctl_bus_dir &= ~break_key;          // configure relevant pin as input
        aux_ctl_bus_out |= break_key;           // enable pull up resistor
        EIMSK |= BREAK_INT;                                     // enable INT2 interrupt
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

/** ***************************************************************************
 * @brief BEEP or flash the LED.
 *
 * This function makes a sound (beep) or flashes an LED, to inform user about
 * an error (usually when the keyboard buffer is full).
 *****************************************************************************/
void do_beep (void)
{
        // disable keyboard interrupt
        EIMSK &= ~KEYBOARD_INT;
        uint8_t cnt = 100;
        while (cnt > 0) {
                aux_ctl_bus_out |= buzzer_led;
                fx_delay_us (600);
                aux_ctl_bus_out &= ~buzzer_led;
                fx_delay_us (600);
                cnt--;
        }
        // enable keyboard interrupt
        EIMSK |= KEYBOARD_INT;
}


/** ***************************************************************************
 * @brief Reset terminal attached to serial port.
 *
 * This function transmits a special sequrence of chartacters that is
 * interpreted as a request to reset the terminal.
 *****************************************************************************/
void uart_ansi_rst_clr (void)
{
        // ANSI reset
        fputc (27, &stream_serial);
        fputc ('[', &stream_serial);
        fputc ('0', &stream_serial);
        fputc ('m', &stream_serial);
        // ANSI clear
        fputc (27, &stream_serial);
        fputc ('[', &stream_serial);
        fputc ('H', &stream_serial);
        fputc (27, &stream_serial);
        fputc ('[', &stream_serial);
        fputc ('J', &stream_serial);
}

/** ***************************************************************************
 * @brief Move cursor on attached terminal.
 *
 * This function transmits a special sequrence of chartacters that moves the
 * cursor of the terminal to a specific position.
 *****************************************************************************/
void uart_ansi_move_cursor (uint8_t row, uint8_t col)
{
        fputc (27, &stream_serial);
        fputc ('[', &stream_serial);
        fprintf (&stream_serial, "%d", row);
        fputc (';', &stream_serial);
        fprintf (&stream_serial, "%d", col);
        fputc ('H', &stream_serial);
}

/** ***************************************************************************
 * @brief Send character to device attched on serial port.
 *****************************************************************************/
int putchar_ser (char chr, FILE *stream)
{
        if (chr == LF)
                putchar_ser (CR, stream);
        loop_until_bit_is_set (UCSR0A, UDRE0);
        UDR0 = chr;
        return 0;
}

/** ***************************************************************************
 * @brief Get character from device attched on serial port.
 *****************************************************************************/
int getchar_ser (FILE *stream)
{
        uint8_t chr;
        loop_until_bit_is_set (UCSR0A, RXC0);
        chr = UDR0;
        return chr;
}

/** ***************************************************************************
 * @brief Send character to VGA controller.
 *
 * This function sends a signle character to the graphics subsystem. If it is
 * a printable character, it will also be transmitted through the serial port.
 *****************************************************************************/
int putchar_phy (char chr, FILE *stream)
{
        // send to VGA
        vgaready();
        pri_data_bus_out = chr;
        tovga();
        pri_data_bus_out = 0;

        /*
        // echo on UART
        if (chr < 128) {
                if (chr == BS) {
                        fputc (BS, &stream_serial);
                        fputc (SPACE, &stream_serial);
                        fputc (BS, &stream_serial);
                } else
                        fputc (chr , &stream_serial);
        }
        */
        return 0;
}

/** ***************************************************************************
 * @brief Get character from keyboard.
 *
 * The default STANDARD INPUT for this system is the buffer holding the
 * keyboard keystrokes. This function gets a character from the said buffer.
 *****************************************************************************/
int getchar_phy (FILE *stream)
{
        static uint8_t kb_read_ptr;
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

/** ***************************************************************************
 * @brief Send character to sound controller.
 *
 * This function sends a signle character to the audio subsystem.
 *****************************************************************************/
void send_to_apu (uint8_t cbyte)
{
        pri_data_bus_out = cbyte;
        apuready();
        toapu();
}

/** ***************************************************************************
 * @brief Put character in EEPROM.
 *
 * This function writes a character in EEPROM. It is used by the EEPROM stream.
 *****************************************************************************/
int putchar_rom (char chr, FILE *stream)
{
        eeprom_update_byte ((uint8_t *) eeprom_ptr++, chr);
        return 0;
}

/** ***************************************************************************
 * @brief Get character in EEPROM.
 *
 * This function reads a character from EEPROM. It is used by the EEPROM stream.
 *****************************************************************************/
int getchar_rom (FILE *stream)
{
        uint8_t chr = eeprom_read_byte ((uint8_t *) eeprom_ptr++);
        return chr;
}

/** ***************************************************************************
 * @brief Change on-screen text colour.
 *
 * This function sends to the graphics subsystem the necessary control byte,
 * along with the selected "text" colour.
 *****************************************************************************/
void text_color (uint8_t color)
{
        putchar (vid_color);
        putchar (color);
}

/** ***************************************************************************
 * @brief Change on-screen background colour.
 *
 * This function sends to the graphics subsystem the necessary control byte,
 * along with the selected background colour -- paper colour, anyone?
 *****************************************************************************/
void paper_color (uint8_t color)
{
        putchar (vid_paper);
        putchar (color);
}

/** ***************************************************************************
 * @brief Move cursor to arbitrary location.
 *
 * This function sends to the graphics subsystem the necessary control byte,
 * along with the new location of the cursor.
 *****************************************************************************/
void locate_cursor (uint8_t line, uint8_t column)
{
        putchar (vid_locate);
        putchar (line);
        putchar (column);
}

/** ***************************************************************************
 * @brief Draw a pixel on the screen.
 *
 * This function sends to the graphics subsystem the necessary control byte,
 * along with data describing the new pixel (position and colour).
 *****************************************************************************/
void put_pixel (uint8_t x, uint8_t y, uint8_t color)
{
        putchar (vid_pixel);
        putchar (x);
        putchar (y);
        putchar (color);
}

/** ***************************************************************************
 * @brief ISR: Check if user pressed break button.
 *****************************************************************************/
ISR (INT2_vect)
{
        // signal program-break
        break_flow = 1;
        // disable emergency break key
        EIMSK &= ~BREAK_INT;
}

/** ***************************************************************************
 * @brief ISR: Ignore/delete bits that do not belong to a keyboard packet.
 *****************************************************************************/
ISR (TIMER0_COMPA_vect)   // , ISR_NAKED )
{
        // reset bit counter
        kb_bit_cnt = 11;
        // stop timer
        TCCR0B = 0;
}

/** ***************************************************************************
 * @brief ISR: Gather bits sent from the keyboard and form packets.
 *****************************************************************************/
ISR (INT0_vect)
{
        uint8_t bit_val;                                // incoming bit
        static uint8_t raw_data;                        // received scan code
        // get bit value as quickly as possible!
        bit_val = peripheral_bus_in;
        bit_val &= kb_dat_pin;
        if (!edge) {
                // start timer
                if (kb_bit_cnt == 11)
                        TCCR0B = _BV (CS02) | _BV (CS00);
                // useful data are in bits 3-10 (parity and start/stop bits are ignored)
                if (kb_bit_cnt < 11 && kb_bit_cnt > 2) {
                        raw_data = (raw_data >> 1);
                        if (bit_val)
                                raw_data = raw_data | 0x80;
                }
                EICRA = 3;                              // set interrupt on rising edge
                edge = 1;                               // 1: rising edge
        } else {
                kb_bit_cnt--;
                if (kb_bit_cnt == 0) {                  // when all bits are received
                        kb_decode (raw_data);
                        kb_bit_cnt = 11;
                }
                EICRA = 2;                              // set interrupt on falling edge
                edge = 0;                               // 0: falling edge
        }
}

