/**
 * @file interpreter.h
 * @brief Prototypes for interpreting and execution functions.
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "cmd_audio.h"
#include "cmd_screen.h"
#include "cmd_flow.h"
#include "cmd_eeprom.h"
#include "cmd_pinctl.h"
#include "cmd_other.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint16_t get_linenumber (void);
void basic_init (void);
void interpreter (void);

// =============================================================================
// CONSTANTS AND CUSTOM DATA TYPES
// =============================================================================

/*
 * MEMORY_SIZE = PROGRAM_SPACE + VAR_SIZE + STACK_SIZE
 * 1200 is the approximate footprint of all variables and CPU stack
 */

#define MEMORY_SIZE (RAMEND - 1200)

#define HIGHLOW_HIGH	1
#define HIGHLOW_UNKNOWN 4

#define INPUT_BUFFER_SIZE 6
#define STACK_SIZE ( sizeof( struct stack_for_frame ) * 5 )
#define VAR_SIZE sizeof( int16_t )

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'


typedef uint16_t LINE_NUMBER;
typedef uint8_t LINE_LENGTH;

enum {
	POST_CMD_NOTHING = 0,
	POST_CMD_EXEC_LINE = 1,
	POST_CMD_NEXT_LINE = 2,
	POST_CMD_NEXT_STATEMENT = 3,
	POST_CMD_WARM_RESET = 4,
	POST_CMD_PROMPT = 5,
	POST_CMD_LOOP = 6
};

struct stack_for_frame {
	uint8_t frame_type;
	uint8_t for_var;
	uint16_t terminal;
	uint16_t step;
	uint8_t *current_line;
	uint8_t *txtpos;
};

struct stack_gosub_frame {
	uint16_t frame_type;
	uint8_t *current_line;
	uint8_t *txtpos;
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

const uint8_t msg_welcome[25];
const uint8_t msg_ram_bytes[11];
const uint8_t msg_rom_bytes[11];
const uint8_t msg_available[17];
const uint8_t msg_break[7];
const uint8_t msg_ok[3];

uint8_t program[MEMORY_SIZE];
uint8_t input_buffer[INPUT_BUFFER_SIZE];
uint8_t *txtpos, *list_line;

uint8_t error_code;
uint8_t *program_start, *program_end;
uint8_t *stack_limit;
uint8_t *variables_begin;
uint8_t *current_line;
uint8_t *stack_ptr;

LINE_LENGTH linelen;
LINE_NUMBER linenum;

#endif
