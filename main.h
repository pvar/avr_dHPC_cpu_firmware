// ----------------------------------------------------------------------------
// program function prototypes
// ----------------------------------------------------------------------------

int main (void);

// message and data printing
void do_beep (void);
void printnum (int16_t num, FILE *stream);
void printUnum (uint16_t num, FILE *stream);
void printmsgNoNL (const uint8_t *msg, FILE *stream);
void printmsg (const uint8_t *msg, FILE *stream);
void printline (FILE *stream);
void newline (FILE *stream);
uint8_t print_string (void);

// internal data handling
void uppercase (void);
void ignorespace (void);
void get_line (uint8_t prompt);
void push_byte (uint8_t b);
uint8_t pop_byte (void);
uint8_t *find_line (void);
int16_t str_to_num (uint8_t *strptr);

uint8_t break_test (void);
uint16_t linenum_test (void);

// file-access related
uint16_t isValidFnChar (uint8_t c);
uint8_t * filenameWord (void);

// delay functions
void fx_delay_ms (uint16_t ms);
void fx_delay_us (uint16_t us);

// ----------------------------------------------------------------------------
// constants, variables and structures
// ----------------------------------------------------------------------------

#define MEMORY_SIZE (RAMEND - 1200)
// MEMORY_SIZE = PROGRAM_SPACE + VAR_SIZE + STACK_SIZE
// 1200 is the approximate footprint of all variables and CPU stack

#define MAXCPL 32
#define TXT_COL_DEFAULT 76
#define TXT_COL_ERROR 3
#define boolean uint8_t			// remove pseudo-type boolean!!!
#define true 1
#define false 0

#define cfg_auto_run 		1 // 1st bit
#define cfg_run_after_load	2 // 2nd bit
#define cfg_from_sdcard		4 // 3rd bit
#define cfg_from_eeprom 	8 // 4th bit

uint8_t main_config;
