// ----------------------------------------------------------------------------
// program function prototypes
// ----------------------------------------------------------------------------
void basic_init (void);
void error_message (void);

// ----------------------------------------------------------------------------
// constants, variables and structures
// ----------------------------------------------------------------------------
const uint8_t msg_welcome[]		PROGMEM = "Welcome to nstBASIC v0.2";
const uint8_t msg_ram_bytes[]	PROGMEM = " bytes RAM";
const uint8_t msg_rom_bytes[]	PROGMEM = " bytes ROM";
const uint8_t msg_available[]	PROGMEM = " bytes available";
const uint8_t msg_break[]		PROGMEM = "Break!";
const uint8_t msg_ok[]			PROGMEM = "OK";

const uint8_t err_msgxl[]		PROGMEM = "Left ";
const uint8_t err_msgxr[]		PROGMEM = "Right ";
const uint8_t err_msg01[]		PROGMEM = "Not yet implemented";
const uint8_t err_msg02[]		PROGMEM = "Syntax error";
const uint8_t err_msg03[]		PROGMEM = "Stack overflow";
const uint8_t err_msg04[]		PROGMEM = "Unexpected character";
const uint8_t err_msg05_06[]	PROGMEM = "parenthesis missing";
const uint8_t err_msg07[]		PROGMEM = "Variable expected";
const uint8_t err_msg08[]		PROGMEM = "Jump point not found";
const uint8_t err_msg09[]		PROGMEM = "Invalid line number";
const uint8_t err_msg0A[]		PROGMEM = "Operator expected";
const uint8_t err_msg0B[]		PROGMEM = "Division by zero";
const uint8_t err_msg0C[]		PROGMEM = "Invalid pin [0..7]";
const uint8_t err_msg0D[]		PROGMEM = "Pin I/O error";
const uint8_t err_msg0E[]		PROGMEM = "Unknown function";
const uint8_t err_msg0F[]		PROGMEM = "Unknown command";
const uint8_t err_msg10[]		PROGMEM = "Invalid coordinates";
const uint8_t err_msg11[]		PROGMEM = "Invalid variable name";
const uint8_t err_msg12[]		PROGMEM = "Expected byte [0..255]";
const uint8_t err_msg13[]		PROGMEM = "Out of range";
const uint8_t err_msg14[]		PROGMEM = "Expected color [0..127]";

#define HIGHLOW_HIGH	1
#define HIGHLOW_UNKNOWN 4

#define INPUT_BUFFER_SIZE 6
#define STACK_SIZE ( sizeof( struct stack_for_frame ) * 5 )
#define VAR_SIZE sizeof( int16_t )

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'

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

uint8_t program[ MEMORY_SIZE ];
uint8_t *txtpos, *maxpos, *list_line;
uint8_t error_code;
uint8_t *tempsp;
uint8_t in_buffer[ INPUT_BUFFER_SIZE ];
uint8_t *inptr;

uint8_t *program_start;
uint8_t *program_end;
uint8_t *stack;				// software stack for jumps in BASIC
uint8_t *stack_limit;
uint8_t *variables_begin;
uint8_t *current_line;
uint8_t *sp;
uint8_t table_index;
uint16_t linenum;

uint8_t *start;
uint8_t *new_end;
uint8_t linelen;
boolean isDigital;
boolean alsoWait = false;
uint16_t val;
