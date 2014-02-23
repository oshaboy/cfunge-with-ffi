/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2013 Arvid Norlander <anmaster AT tele2 DOT se>
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

#include "../../src/global.h"
#include "funge_string.h"

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE
funge_cell *funge_strchr(const funge_cell *s,
                         const funge_cell c)
{
	while (*s != '\0') {
		FUNGE_WARNING_IGNORE("-Wcast-qual")
		if (*s == c)
			return (funge_cell *)s;
		FUNGE_WARNING_RESTORE()
		s++;
	}
	return NULL;
}