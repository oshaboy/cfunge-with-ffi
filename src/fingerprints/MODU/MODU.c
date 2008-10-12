/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at the proxy's option) any later version. Arvid Norlander is a
 * proxy who can decide which future versions of the GNU General Public
 * License can be used.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MODU.h"
#include "../../stack.h"

// This was more or less taken from CCBI
FUNGE_ATTR_CONST FUNGE_ATTR_FAST
static inline fungeCell floordiv(fungeCell x, fungeCell y)
{
	x /= y;
	if (x < 0)
		return x - 1;
	else
		return x;
}

static void finger_MODU_signedResult(instructionPointer * ip)
{
	fungeCell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		stack_push(ip->stack, x - floordiv(x, y) * y);
	} else {
		stack_push(ip->stack, 0);
	}
}

static void finger_MODU_unsignedResult(instructionPointer * ip)
{
	fungeCell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		stack_push(ip->stack, ABS(x % y));
	} else {
		stack_push(ip->stack, 0);
	}
}

// C style reminder.
static void finger_MODU_remainder(instructionPointer * ip)
{
	fungeCell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		// Well that's easy, this *is* C.
		stack_push(ip->stack, x % y);
	} else {
		stack_push(ip->stack, 0);
	}
}


bool finger_MODU_load(instructionPointer * ip)
{
	manager_add_opcode(MODU,  'M', signedResult)
	manager_add_opcode(MODU,  'R', remainder)
	manager_add_opcode(MODU,  'U', unsignedResult)
	return true;
}
