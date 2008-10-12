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

#include "../global.h"
#include "io.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../rect.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#include <assert.h>
#include <stdbool.h>

FUNGE_ATTR_FAST void run_file_input(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (setting_enable_sandbox) {
		ip_reverse(ip);
		return;
	}

	{
		char * restrict filename;
		bool binary;
		fungeVector offset;
		fungeVector size;

		// Pop stuff.
		filename = stack_pop_string(ip->stack);

		// Sanity test!
		if (*filename == '\0') {
			stack_freeString(filename);
			ip_reverse(ip);
			return;
		}

		binary = (bool)(stack_pop(ip->stack) & 1);
		offset = stack_pop_vector(ip->stack);

		if (!fungespace_load_at_offset(filename,
		                            vector_create_ref(offset.x + ip->storageOffset.x, offset.y + ip->storageOffset.y),
		                            &size, binary)) {
			ip_reverse(ip);
		} else {
			stack_push_vector(ip->stack, &size);
			stack_push_vector(ip->stack, &offset);
		}
		stack_freeString(filename);
	}
}

FUNGE_ATTR_FAST void run_file_output(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (setting_enable_sandbox) {
		ip_reverse(ip);
		return;
	}

	{
		char * restrict filename;
		bool textfile;
		fungeVector offset;
		fungeVector size;

		// Pop stuff.
		filename = stack_pop_string(ip->stack);
		textfile = (bool)(stack_pop(ip->stack) & 1);
		offset = stack_pop_vector(ip->stack);
		size = stack_pop_vector(ip->stack);

		// Sanity test!
		if (*filename == '\0' || size.x < 1 || size.y < 1) {
			stack_freeString(filename);
			ip_reverse(ip);
			return;
		}

		if (!fungespace_save_to_file(filename,
		                          vector_create_ref(offset.x + ip->storageOffset.x, offset.y + ip->storageOffset.y),
		                          &size, textfile))
			ip_reverse(ip);
			stack_freeString(filename);
	}

}
