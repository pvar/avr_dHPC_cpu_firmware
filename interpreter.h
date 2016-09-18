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

#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4

#define INPUT_BUFFER_SIZE 6
#define STACK_SIZE ( sizeof( struct stack_for_frame ) * 5 )
#define VAR_SIZE sizeof( int16_t )

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'


typedef uint16_t LINE_NUMBER;
typedef uint8_t LINE_LENGTH;

enum EXECUTION_STATUS {
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

// general messages (definitions reside in printing.c)
extern const uint8_t msg_welcome[25];
extern const uint8_t msg_ram_bytes[11];
extern const uint8_t msg_rom_bytes[11];
extern const uint8_t msg_available[17];
extern const uint8_t msg_break[7];
extern const uint8_t msg_ok[3];

// error messages (definitions reside in printing.c)
extern const uint8_t err_msgxl[6];
extern const uint8_t err_msgxr[7];
extern const uint8_t err_msg01[20];
extern const uint8_t err_msg02[13];
extern const uint8_t err_msg03[15];
extern const uint8_t err_msg04[21];
extern const uint8_t err_msg05[20];
extern const uint8_t err_msg07[18];
extern const uint8_t err_msg08[21];
extern const uint8_t err_msg09[20];
extern const uint8_t err_msg0A[18];
extern const uint8_t err_msg0B[17];
extern const uint8_t err_msg0C[19];
extern const uint8_t err_msg0D[14];
extern const uint8_t err_msg0E[17];
extern const uint8_t err_msg0F[16];
extern const uint8_t err_msg10[20];
extern const uint8_t err_msg11[22];
extern const uint8_t err_msg12[23];
extern const uint8_t err_msg13[13];
extern const uint8_t err_msg14[24];

// functions that return nothing / might print a value (definitions reside in parser.c)
extern const uint8_t commands[208];

// functions that return a value / print nothing (definitions reside in parser.c)
extern const uint8_t functions[27];

// relational operators (definitions reside in parser.c)
extern const uint8_t relop_table[12];

// other keywords (definitions reside in parser.c)
extern const uint8_t to_tab[3];
extern const uint8_t step_tab[5];
extern const uint8_t highlow_tab[12];

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
