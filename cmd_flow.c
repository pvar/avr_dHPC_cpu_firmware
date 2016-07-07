// ----------------------------------------------------------------------------
// Implementation of flow-control commands
// ----------------------------------------------------------------------------
//
// nstBASIC (not-so-tiny BASIC) is limited implementation
// of the venerable BASIC language. The code is heavily tied
// to the hardware of a homebrew computer.
//
// Created by Panos Varelas <varelaspanos@gmail.com>
//
// Based on "TinyBasic Plus" by:
//   - Mike Field <hamster@snap.net.nz>,
//   - Scott Lawrence <yorgle@gmail.com> and
//   - Jurg Wullschleger <wullschleger@gmail.com>
//     (fixed whitespace and unary operations)
//
// ----------------------------------------------------------------------------

#include "interpreter.h"
#include "cmd_flow.h"

uint8_t gotoline (void)
{
    linenum = parse_step1();
    if (error_code || *txtpos != LF) {
        error_code = 0x4;
        return POST_CMD_WARM_RESET;
    }
    current_line = find_line();
    return POST_CMD_EXEC_LINE;
}

uint8_t check (void)
{
    val = parse_step1();
    if (error_code || *txtpos == LF) {
        error_code = 0x4;
        return POST_CMD_WARM_RESET;
    }
    if (val != 0)
        return POST_CMD_LOOP;
    else
        return POST_CMD_NEXT_LINE;
}

uint8_t loopfor (void)
{
		uint8_t var;
		uint16_t initial, step, terminal;
		ignorespace();
		if (*txtpos < 'A' || *txtpos > 'Z') {
			error_code = 0x7;
            return POST_CMD_WARM_RESET;
		}
		var = *txtpos;
		txtpos++;
		ignorespace();
		if (*txtpos != '=') {
			error_code = 0xA;
            return POST_CMD_WARM_RESET;
		}
		txtpos++;
		ignorespace();
		error_code = 0;
		initial = parse_step1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
		scantable (to_tab);
		if (table_index != 0) {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
		terminal = parse_step1();
		if (error_code) {
            return POST_CMD_WARM_RESET;
        }
        scantable (step_tab);
		if (table_index == 0) {
			step = parse_step1();
			if (error_code) {
                return POST_CMD_WARM_RESET;
            }
		} else step = 1;
		ignorespace();
		if (*txtpos != LF && *txtpos != ':') {
			error_code = 0x2;
            return POST_CMD_WARM_RESET;
		}
		if (!error_code && *txtpos == LF) {
			struct stack_for_frame *f;
			if (sp + sizeof (struct stack_for_frame) < stack_limit) {
				error_code = 0x3;
                return POST_CMD_WARM_RESET;
			}
			sp -= sizeof (struct stack_for_frame);
			f = (struct stack_for_frame *)sp;
			((uint16_t *)variables_begin)[var - 'A'] = initial;
			f->frame_type = STACK_FOR_FLAG;
			f->for_var = var;
			f->terminal = terminal;
			f->step = step;
			f->txtpos = txtpos;
			f->current_line = current_line;
			return POST_CMD_NEXT_STATEMENT;
		}
	error_code = 0x4;
    return POST_CMD_WARM_RESET;
}

uint8_t gosub (void)
{
	error_code = 0;
	linenum = parse_step1();
	if (!error_code && *txtpos == LF) {
		struct stack_gosub_frame *f;
		if (sp + sizeof (struct stack_gosub_frame) < stack_limit) {
			error_code = 0x3;
            return POST_CMD_WARM_RESET;
		}
		sp -= sizeof (struct stack_gosub_frame);
		f = (struct stack_gosub_frame *)sp;
		f->frame_type = STACK_GOSUB_FLAG;
		f->txtpos = txtpos;
		f->current_line = current_line;
		current_line = find_line();
		return POST_CMD_EXEC_LINE;
	}
	error_code = 0x4;
    return POST_CMD_WARM_RESET;
}

uint8_t next (void)
{
	// find the variable name
	ignorespace();
	if (*txtpos < 'A' || *txtpos > 'Z') {
		error_code = 0x7;
        return POST_CMD_WARM_RESET;
	}
	txtpos++;
	ignorespace();
	if (*txtpos != ':' && *txtpos != LF) {
		error_code = 0x2;
        return POST_CMD_WARM_RESET;
	}
    return POST_CMD_NOTHING;
}

uint8_t gosub_return (void)
{
	// walk up the stack frames and find the frame we want -- if present
	tempsp = sp;
	while (tempsp < program + MEMORY_SIZE - 1) {
		switch (tempsp[0]) {
		case STACK_GOSUB_FLAG:
			if (table_index == CMD_RETURN) {
				struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
				current_line	= f->current_line;
				txtpos			= f->txtpos;
				sp += sizeof (struct stack_gosub_frame);
				return POST_CMD_NEXT_STATEMENT;
			}
			// This is not the loop you are looking for... go up in the stack
			tempsp += sizeof (struct stack_gosub_frame);
			break;
		case STACK_FOR_FLAG:
			// Flag, Var, Final, Step
			if (table_index == CMD_NEXT) {
				struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
				// Is the variable we are looking for?
				if (txtpos[-1] == f->for_var) {
					uint16_t *varaddr = ((uint16_t *)variables_begin) + txtpos[-1] - 'A';
					*varaddr = *varaddr + f->step;
					// Use a different test depending on the sign of the step increment
					if ((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal)) {
						// We have to loop so don't pop the stack
						txtpos = f->txtpos;
						current_line = f->current_line;
						return POST_CMD_NEXT_STATEMENT;
					}
					// We've run to the end of the loop. drop out of the loop, popping the stack
					sp = tempsp + sizeof (struct stack_for_frame);
					return POST_CMD_NEXT_STATEMENT;
				}
			}
			// This is not the loop you are looking for... go up in the stack
			tempsp += sizeof (struct stack_for_frame);
			break;
		default:
			//printf( "Stack is full!\n" );
            return POST_CMD_WARM_RESET;
		}
	}
	// Didn't find the variable we've been looking for
	error_code = 0x8;
    return POST_CMD_WARM_RESET;
}