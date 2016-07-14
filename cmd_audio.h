#ifndef CMD_AUDIO_H
#define CMD_AUDIO_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

/*******************************************************************************
 ** PROTOTYPES FOR NON-STATIC FUNCTIONS
 **/

uint8_t play (void);
uint8_t stop (void);
uint8_t tempo (void);
uint8_t music (void);

#endif