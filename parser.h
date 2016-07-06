#ifndef PARSER_H
#define PARSER_H

// ----------------------------------------------------------------------------
// program function prototypes
// ----------------------------------------------------------------------------
void scantable (const uint8_t *table);
uint8_t get_note (void);
uint8_t get_effect (void);
uint8_t get_duration (void);
uint8_t get_octave (void);
int16_t parse_step1 (void);
int16_t parse_step2 (void);
int16_t parse_step3 (void);
int16_t parse_step4 (void);
void parse_channel (void);
void parse_notes (void);


// ----------------------------------------------------------------------------
// constants, variables and structures
// ----------------------------------------------------------------------------

// functions which cannot be part of a larger expression  (return nothing / might print a value)
const uint8_t commands[] PROGMEM = {
	'L', 'I', 'S', 'T' + 0x80,
	'N', 'E', 'W' + 0x80,
	'R', 'U', 'N' + 0x80,
	'N', 'E', 'X', 'T' + 0x80,
	'L', 'E', 'T' + 0x80,
	'I', 'F' + 0x80,
	'G', 'O', 'T', 'O' + 0x80,
	'M', 'P', 'L', 'A', 'Y' + 0x80,
	'M', 'S', 'T', 'O', 'P' + 0x80,
	'T', 'E', 'M', 'P', 'O' + 0x80,
	'M', 'U', 'S', 'I', 'C' + 0x80,
	'G', 'O', 'S', 'U', 'B' + 0x80,
	'R', 'E', 'T', 'U', 'R', 'N' + 0x80,
	'R', 'A', 'N', 'D', 'O', 'M', 'I', 'Z', 'E' + 0x80,
	'R', 'N', 'D', 'S', 'E', 'E', 'D' + 0x80,
	'R', 'S', 'T' + 0x80,
	'C', 'L', 'S' + 0x80,
	'R', 'E', 'M' + 0x80,
	'F', 'O', 'R' + 0x80,
	'I', 'N', 'P', 'U', 'T' + 0x80,
	'B', 'E', 'E', 'P' + 0x80,
	'P', 'R', 'I', 'N', 'T' + 0x80,
	'L', 'O', 'C', 'A', 'T', 'E' + 0x80,
	'P', 'O', 'K', 'E' + 0x80,
	'P', 'S', 'E', 'T' + 0x80,
	'S', 'T', 'O', 'P' + 0x80,
	'E', 'N', 'D' + 0x80,
	'M', 'E', 'M' + 0x80,
	'P', 'E', 'N' + 0x80,
	'P', 'A', 'P', 'E', 'R' + 0x80,
	'?' + 0x80,
	'#' + 0x80,
	'\'' + 0x80,
	'D', 'E', 'L', 'A', 'Y' + 0x80,
	'E', 'L', 'I', 'S', 'T' + 0x80,
	'E', 'F', 'O', 'R', 'M', 'A', 'T' + 0x80,
	'E', 'C', 'H', 'A', 'I', 'N' + 0x80,
	'E', 'S', 'A', 'V', 'E' + 0x80,
	'E', 'L', 'O', 'A', 'D' + 0x80,
	'F', 'I', 'L', 'E', 'S' + 0x80,
	'C', 'F', 'O', 'R', 'M', 'A', 'T' + 0x80,
	'C', 'C', 'H', 'A', 'I', 'N' + 0x80,
	'C', 'S', 'A', 'V', 'E' + 0x80,
	'C', 'L', 'O', 'A', 'D' + 0x80,
	'P', 'I', 'N', 'D', 'I', 'R' + 0x80,
	'P', 'I', 'N', 'D', 'W', 'R', 'I', 'T', 'E' + 0x80,
	0
};
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

// functions that must be part of a larger expression (return a value / print nothing)
const uint8_t functions[] PROGMEM = {
	'P', 'E', 'E', 'K' + 0x80,
	'A', 'B', 'S' + 0x80,
	'R', 'N', 'D' + 0x80,
	'P', 'I', 'N', 'D', 'R', 'E', 'A', 'D' + 0x80,
	'P', 'I', 'N', 'A', 'R', 'E', 'A', 'D' + 0x80,
	0
};
enum {
	FN_PEEK = 0,
	FN_ABS,
	FN_RND,
	FN_PINDREAD,
	FN_PINAREAD,
	FN_UNKNOWN
};

// relational operators
const uint8_t relop_table[] PROGMEM = {
	'>', '=' + 0x80,
	'<', '>' + 0x80,
	'>' + 0x80,
	'=' + 0x80,
	'<', '=' + 0x80,
	'<' + 0x80,
	'!', '=' + 0x80,
	0
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

const uint8_t to_tab[] PROGMEM = { 'T', 'O' + 0x80, 0 };
const uint8_t step_tab[] PROGMEM = { 'S', 'T', 'E', 'P' + 0x80, 0 };
const uint8_t highlow_tab[] PROGMEM = { 'H', 'I', 'G', 'H' + 0x80, 'H', 'I' + 0x80, 'L', 'O', 'W' + 0x80, 'L', 'O' + 0x80, 0 };


#endif