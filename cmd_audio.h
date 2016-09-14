/**
 * @file cmd_audio.h
 * @brief Functions that can set notes for each of voice (sound channel),
 * specify reproduction rythm (tempo) and start or stop music playback.
 */

#ifndef CMD_AUDIO_H
#define CMD_AUDIO_H

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "parser.h"

// =============================================================================
// PROTOTYPES OF NON-STATIC FUNCTIONS
// =============================================================================

uint8_t play (void);
uint8_t stop (void);
uint8_t tempo (void);
uint8_t music (void);

#endif