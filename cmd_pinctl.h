#ifndef CMD_PINCTL_H
#define CMD_PINCTL_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

/*******************************************************************************
 ** PROTOTYPES FOR NON-STATIC FUNCTIONS
 **/

uint8_t pindwrite (void);
uint8_t pindir (void);

#endif