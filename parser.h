/**
 * @file parser.h
 * @brief Prototypes for parsing and arithmetic evaluation functions.
 */

#ifndef PARSER_H
#define PARSER_H

// ------------------------------------------------------------------------------
// INCLUDES
// ------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "interpreter.h"

// ------------------------------------------------------------------------------
// PROTOTYPES
// ------------------------------------------------------------------------------

int8_t scantable (const uint8_t *table);
void parse_channel (void);
void parse_notes (void);
int16_t parse_expr_s1 (void);

// ------------------------------------------------------------------------------
// ENUMERATORS
// ------------------------------------------------------------------------------

enum {
        CMD_LIST = 0,
        CMD_NEW,
        CMD_RUN,
        CMD_NEXT,
        CMD_LET,
        CMD_IF,
        CMD_GOTO,
        CMD_MPLAY,
        CMD_MSTOP,
        CMD_TEMPO,
        CMD_MUSIC,
        CMD_GOSUB,
        CMD_RETURN,
        CMD_RANDOMIZE,
        CMD_RNDSEED,
        CMD_RST,
        CMD_CLS,
        CMD_REM,
        CMD_FOR,
        CMD_INPUT,
        CMD_BEEP,
        CMD_PRINT,
        CMD_LOCATE,
        CMD_POKE,
        CMD_PSET,
        CMD_STOP,
        CMD_END,
        CMD_MEM,
        CMD_PEN,
        CMD_PAPER,
        CMD_QMARK,
        CMD_HASH,
        CMD_QUOTE,
        CMD_DELAY,
        CMD_ELIST,
        CMD_EFORMAT,
        CMD_ECHAIN,
        CMD_ESAVE,
        CMD_ELOAD,
        CMD_FILES,
        CMD_CFORMAT,
        CMD_CCHAIN,
        CMD_CSAVE,
        CMD_CLOAD,
        CMD_PINDIR,
        CMD_PINDWRITE,
        CMD_DEFAULT
};
enum {
        FN_PEEK = 0,
        FN_ABS,
        FN_RND,
        FN_PINDREAD,
        FN_PINAREAD,
        FN_UNKNOWN
};
enum {
        RELOP_GE = 0,
        RELOP_NE,
        RELOP_GT,
        RELOP_EQ,
        RELOP_LE,
        RELOP_LT,
        RELOP_NE_BANG,
        RELOP_UNKNOWN
};

#endif
