/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
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

#ifndef _HAD_SRC_GLOBAL_H
#define _HAD_SRC_GLOBAL_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// For compatiblity with other compilers to prevent them
// failing at things like: __attribute__((noreturn))
#ifndef __GNUC__
#  define  __attribute__(x)  /* NO-OP */
#endif

#include <support.h>
#include <stdint.h>
#include <inttypes.h>


#ifdef USE64
// The type of the data cells
#  define FUNGEDATATYPE int_fast64_t
#  define FUNGEDATAPRI PRIdFAST64
// And of vector values
#  define FUNGEVECTORTYPE int_fast64_t
#  define FUNGEVECTORPRI PRIdFAST64
#else
// The type of the data cells
#  define FUNGEDATATYPE int32_t
#  define FUNGEDATAPRI PRId32
// And of vector values
#  define FUNGEVECTORTYPE int32_t
#  define FUNGEVECTORPRI PRId32

#endif

// Handprint: CFUN
#define FUNGEHANDPRINT 0x4346554e
// Version: 0.0.1
// Version: 0.0.1
#define APPVERSION  "0.1.0"
#define FUNGEVERSION 10

#endif
