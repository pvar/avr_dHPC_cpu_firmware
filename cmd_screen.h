#ifndef CMD_SCREEN_H
#define CMD_SCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

/**
 ** prototypes of non-static functions
 **
 **/

uint8_t reset_display (void);
uint8_t clear_screen (void);
uint8_t pen (void);
uint8_t paper (void);
uint8_t print (void);
uint8_t locate (void);
uint8_t pset (void);
 
#endif