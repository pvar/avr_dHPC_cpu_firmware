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

/*******************************************************************************
 ** PROTOTYPES FOR NON-STATIC FUNCTIONS
 **/

uint16_t get_linenumber (void);
void basic_init (void);
void interpreter (void);

/*******************************************************************************
 ** CONSTANTS AND CUSTOM DATA TYPES
 **/

#define MEMORY_SIZE (RAMEND - 1200)
// MEMORY_SIZE = PROGRAM_SPACE + VAR_SIZE + STACK_SIZE
// 1200 is the approximate footprint of all variables and CPU stack

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

/*******************************************************************************
 ** GLOBAL VARIABLES
 **/

const uint8_t msg_welcome[25];
const uint8_t msg_ram_bytes[11];
const uint8_t msg_rom_bytes[11];
const uint8_t msg_available[17];
const uint8_t msg_break[7];
const uint8_t msg_ok[3];

/*
const uint8_t err_msgxl[6];
const uint8_t err_msgxr[7];
const uint8_t err_msg01[20];
const uint8_t err_msg02[13];
const uint8_t err_msg03[15];
const uint8_t err_msg04[21];
const uint8_t err_msg05[20];
const uint8_t err_msg07[18];
const uint8_t err_msg08[21];
const uint8_t err_msg09[20];
const uint8_t err_msg0A[18];
const uint8_t err_msg0B[17];
const uint8_t err_msg0C[19];
const uint8_t err_msg0D[14];
const uint8_t err_msg0E[17];
const uint8_t err_msg0F[16];
const uint8_t err_msg10[20];
const uint8_t err_msg11[22];
const uint8_t err_msg12[23];
const uint8_t err_msg13[13];
const uint8_t err_msg14[24];
*/

uint8_t error_code;

uint8_t program[MEMORY_SIZE];
uint8_t *txtpos, *list_line;
uint8_t in_buffer[INPUT_BUFFER_SIZE];

uint8_t *program_start;
uint8_t *program_end;
uint8_t *stack; // software stack for calls in nstBASIC
uint8_t *stack_limit;
uint8_t *variables_begin;
uint8_t *sp;
uint8_t *current_line;

uint8_t *start;
uint8_t *new_end;
uint8_t linelen;
uint8_t cmd_status;

LINE_NUMBER linenum;

#endif