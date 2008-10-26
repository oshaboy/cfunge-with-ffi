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

#include "FILE.h"
#include "../../stack.h"
#include "../../../lib/stringbuffer/stringbuffer.h"
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

// Based on how CCBI does it.

typedef struct sFungeFileHandle {
	FILE      * file;
	fungeVector buffvect; // IO buffer in Funge-Space
} FungeFileHandle;

#define ALLOCCHUNK 2
// Array of pointers
static FungeFileHandle** handles = NULL;
static size_t maxHandle = 0;

/// Used by allocate_handle() below to find next free handle.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell findNextfree_handle(void)
{
	for (size_t i = 0; i < maxHandle; i++) {
		if (handles[i] == NULL)
			return i;
	}
	// No free one, extend array..
	{
		FungeFileHandle** newlist = (FungeFileHandle**)cf_realloc(handles, (maxHandle + ALLOCCHUNK) * sizeof(FungeFileHandle*));
		if (!newlist)
			return -1;
		handles = newlist;
		for (size_t i = maxHandle; i < (maxHandle + ALLOCCHUNK); i++)
			handles[i] = NULL;
		maxHandle += ALLOCCHUNK;
		return (maxHandle - ALLOCCHUNK);
	}
}

/// Get a new handle to use for a file, also allocates buffer for it.
/// @return Handle, or -1 on failure
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell allocate_handle(void)
{
	fungeCell h;

	h = findNextfree_handle();
	if (h < 0)
		return -1;

	handles[h] = cf_malloc(sizeof(FungeFileHandle));
	if (!handles[h])
		return -1;
	return h;
}

/// Free a handle. fclose() the file before calling this.
FUNGE_ATTR_FAST
static inline void free_handle(fungeCell h)
{
	if (!handles[h])
		return;
	// Should be closed first!
	if (handles[h]->file != NULL) {
		handles[h]->file = NULL;
	}
	cf_free(handles[h]);
	handles[h] = NULL;
}

/// Checks if handle is valid.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool valid_handle(fungeCell h)
{
	if ((h < 0) || ((size_t)h >= maxHandle) || (!handles[h])) {
		return false;
	} else {
		return true;
	}
}

/// C - Close a file
static void finger_FILE_fclose(instructionPointer * ip)
{
	fungeCell h;

	h = stack_pop(ip->stack);
	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	if (fclose(handles[h]->file) != 0)
		ip_reverse(ip);

	free_handle(h);
}

/// C - Delete specified file
static void finger_FILE_delete(instructionPointer * ip)
{
	char * restrict filename;

	filename = (char*)stack_pop_string(ip->stack);
	if (unlink(filename) != 0) {
		ip_reverse(ip);
	}

	stack_freeString(filename);
	return;
}


/// G - Get string from file (like c fgets)
static void finger_FILE_fgets(instructionPointer * ip)
{
	fungeCell h;
	FILE * fp;

	h = stack_peek(ip->stack);
	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	fp = handles[h]->file;

	{
		StringBuffer *sb;
		int ch;
		sb = stringbuffer_new();
		if (!sb) {
			ip_reverse(ip);
			return;
		}

		while (true) {
			ch = fgetc(fp);
			switch (ch) {
				case '\r':
					stringbuffer_append_char(sb, (char)ch);
					ch = fgetc(fp);
					if (ch != '\n') {
						ungetc(ch, fp);
						goto endofloop;
					}
				// Fallthrough intentional.
				case '\n':
					stringbuffer_append_char(sb, (char)ch);
					goto endofloop;

				case EOF:
					if (ferror(fp)) {
						clearerr(fp);
						ip_reverse(ip);
						stringbuffer_destroy(sb);
						return;
					} else {
						goto endofloop;
					}

				default:
					stringbuffer_append_char(sb, (char)ch);
					break;
			}
		}
	// Yeah, can't break two levels otherwise...
	endofloop:
		{
			char * str;
			size_t len;
			str = stringbuffer_finish(sb);
			len = strlen(str);
			stack_push_string(ip->stack, (unsigned char*)str, len);
			stack_push(ip->stack, (fungeCell)len);
			free_nogc(str);
			return;
		}
	}
}

/// L - Get current location in file
static void finger_FILE_ftell(instructionPointer * ip)
{
	fungeCell h;
	long pos;

	h = stack_peek(ip->stack);
	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	pos = ftell(handles[h]->file);

	if (pos == -1) {
		clearerr(handles[h]->file);
		ip_reverse(ip);
		return;
	}

	stack_push(ip->stack, (fungeCell)pos);
}

/// O - Open a file (Va = i/o buffer vector)
static void finger_FILE_fopen(instructionPointer * ip)
{
	char * restrict filename;
	fungeCell mode;
	fungeVector vect;
	fungeCell h;

	filename = (char*)stack_pop_string(ip->stack);
	mode = stack_pop(ip->stack);
	vect = stack_pop_vector(ip->stack);

	h = allocate_handle();
	if (h == -1) {
		goto error;
	}

	switch (mode) {
		case 0: handles[h]->file = fopen(filename, "rb");  break;
		case 1: handles[h]->file = fopen(filename, "wb");  break;
		case 2: handles[h]->file = fopen(filename, "ab");  break;
		case 3: handles[h]->file = fopen(filename, "r+b"); break;
		case 4: handles[h]->file = fopen(filename, "w+b"); break;
		case 5: handles[h]->file = fopen(filename, "a+b"); break;
		default:
			free_handle(h);
			goto error;
	}
	if (!handles[h]->file) {
		free_handle(h);
		goto error;
	}
	fcntl(fileno(handles[h]->file), F_SETFD, FD_CLOEXEC, 1);
	if ((mode == 2) || (mode == 5))
		rewind(handles[h]->file);

	handles[h]->buffvect = vect;
	stack_push(ip->stack, h);
	goto end;
// Look... The alternatives to the goto were worse...
error:
	ip_reverse(ip);
end:
	stack_freeString(filename);
}

/// P - Put string to file (like c fputs)
static void finger_FILE_fputs(instructionPointer * ip)
{
	char * restrict str;
	fungeCell h;

	str = (char*)stack_pop_string(ip->stack);
	h = stack_peek(ip->stack);
	if (!valid_handle(h)) {
		ip_reverse(ip);
	} else {
		if (fputs(str, handles[h]->file) == EOF) {
			clearerr(handles[h]->file);
			ip_reverse(ip);
		}
	}
	stack_freeString(str);
}

/// R - Read n bytes from file to i/o buffer
static void finger_FILE_fread(instructionPointer * ip)
{
	fungeCell n, h;

	n = stack_pop(ip->stack);
	h = stack_peek(ip->stack);

	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	if (n <= 0) {
		ip_reverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = calloc_nogc(n, sizeof(char));
		if (!buf) {
			ip_reverse(ip);
			return;
		}

		if (fread(buf, sizeof(unsigned char), n, fp) != (size_t)n) {
			if (ferror(fp)) {
				clearerr(fp);
				ip_reverse(ip);
				cf_free(buf);
				return;
			} else {
				assert(feof(fp));
			}
		}

		for (fungeCell i = 0; i < n; i++) {
			fungespace_set(buf[i], vector_create_ref(handles[h]->buffvect.x + i, handles[h]->buffvect.y));
		}
		free_nogc(buf);
	}
}

/// S - Seek to position in file
static void finger_FILE_fseek(instructionPointer * ip)
{
	fungeCell n, m, h;

	n = stack_pop(ip->stack);
	m = stack_pop(ip->stack);
	h = stack_peek(ip->stack);

	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	switch (m) {
		case 0:
			if (fseek(handles[h]->file, (long)n, SEEK_SET) != 0)
				break;
			else
				return;
		case 1:
			if (fseek(handles[h]->file, (long)n, SEEK_CUR) != 0)
				break;
			else
				return;
		case 2:
			if (fseek(handles[h]->file, (long)n, SEEK_END) != 0)
				break;
			else
				return;
		default:
			break;
	}
	// An error if we got here...
	clearerr(handles[h]->file);
	ip_reverse(ip);
}

/// W - Write n bytes from i/o buffer to file
static void finger_FILE_fwrite(instructionPointer * ip)
{
	fungeCell n, h;

	n = stack_pop(ip->stack);
	h = stack_peek(ip->stack);

	if (!valid_handle(h)) {
		ip_reverse(ip);
		return;
	}

	if (n <= 0) {
		ip_reverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = cf_malloc_noptr(n * sizeof(char));

		for (fungeCell i = 0; i < n; i++) {
			buf[i] = fungespace_get(vector_create_ref(handles[h]->buffvect.x + i, handles[h]->buffvect.y));
		}
		if (fwrite(buf, sizeof(unsigned char), n, fp) != (size_t)n) {
			if (ferror(fp)) {
				clearerr(fp);
				ip_reverse(ip);
			}
		}
		cf_free(buf);
	}
}

FUNGE_ATTR_FAST static inline bool init_handle_list(void)
{
	assert(!handles);
	handles = (FungeFileHandle**)cf_calloc(ALLOCCHUNK, sizeof(FungeFileHandle*));
	if (!handles)
		return false;
	maxHandle = ALLOCCHUNK;
	return true;
}

bool finger_FILE_load(instructionPointer * ip)
{
	if (!handles)
		if (!init_handle_list())
			return false;

	manager_add_opcode(FILE,  'C', fclose)
	manager_add_opcode(FILE,  'D', delete)
	manager_add_opcode(FILE,  'G', fgets)
	manager_add_opcode(FILE,  'L', ftell)
	manager_add_opcode(FILE,  'O', fopen)
	manager_add_opcode(FILE,  'P', fputs)
	manager_add_opcode(FILE,  'R', fread)
	manager_add_opcode(FILE,  'S', fseek)
	manager_add_opcode(FILE,  'W', fwrite)
	return true;
}
