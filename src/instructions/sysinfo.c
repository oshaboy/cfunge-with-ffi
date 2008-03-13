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

#include "../global.h"
#include <unistd.h>
#include "sysinfo.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../rect.h"
#include "../stack.h"
#include "../ip.h"
#include <time.h>
#include <string.h>


// Push a single request value
static void PushRequest(FUNGEDATATYPE request, instructionPointer * ip)
{
	switch (request) {
		case 1: // Flags
			//StackPush(0x20, ip->stack);
			StackPush(0x0, ip->stack);
			break;
		case 2: // Cell size
			StackPush(sizeof(FUNGEDATATYPE), ip->stack);
			break;
		case 3: // Handprint
			StackPush(FUNGEHANDPRINT, ip->stack);
			break;
		case 4: // Version
			StackPush(FUNGEVERSION, ip->stack);
			break;
		case 5: // Operating paradigm
			StackPush(0, ip->stack);
			break;
		case 6: // Path separator
			StackPush('/', ip->stack);
			break;
		case 7: // Scalars / vector
			StackPush(2, ip->stack);
			break;
		case 8: // IP ID
			StackPush(0, ip->stack);
			break;
		case 9: // TEAM ID
			StackPush(0, ip->stack);
			break;
		case 10: // Vector of current IP position
			StackPushVector(&ip->position, ip->stack);
			break;
		case 11: // Delta of current IP position
			StackPushVector(&ip->delta, ip->stack);
			break;
		case 12: // Storage offset of current IP position
			StackPushVector(&ip->storageOffset, ip->stack);
			break;
		case 13: // Least point
			{
				fungeRect rect;
				fungeSpaceGetBoundRect(fspace, &rect);
				StackPushVector(& (fungePosition) { .x = rect.x, .y = rect.y }, ip->stack);
				break;
			}
		case 14: // Greatest point
			{
				fungeRect rect;
				fungeSpaceGetBoundRect(fspace, &rect);
				StackPushVector(& (fungePosition) { .x = rect.w, .y = rect.h }, ip->stack);
				break;
			}
		case 15: // Time ((year - 1900) * 256 * 256) + (month * 256) + (day of month)
			{
			time_t now;
			struct tm curTime;
			now = time(NULL);
			gmtime_r(&now, &curTime);
			StackPush((FUNGEDATATYPE)(curTime.tm_year * 256 * 256 + curTime.tm_mon * 256 + curTime.tm_mday), ip->stack);
			break;
			}
		case 16: // Time (hour * 256 * 256) + (minute * 256) + (second)
			{
			time_t now;
			struct tm curTime;
			now = time(NULL);
			gmtime_r(&now, &curTime);
			StackPush((FUNGEDATATYPE)(curTime.tm_hour * 256 * 256 + curTime.tm_min * 256 + curTime.tm_sec), ip->stack);
			break;
			}
		case 17: // Number of stacks on stack stack
			StackPush(ip->stackstack->size, ip->stack);
			break;
		case 18: // Number of elements on all stacks (TODO)
			StackPush(ip->stack->top, ip->stack);
			break;
		case 19: // Command line arguments
			StackPush('\0', ip->stack);
			for (int i = fungeargc - 1; i >= 0; i--) {
				StackPushString(strlen(fungeargv[i]) + 1, fungeargv[i], ip->stack);
			}
			break;
		case 20: // Environment variables
			{
				char * tmp;
				int i = 0;
				while (true) {
					tmp = environ[i];
					if (!tmp || *tmp == '\0')
						break;
					StackPushString(strlen(tmp), tmp, ip->stack);
					i++;
				}
				break;
			}
		default:
			ipReverse(ip);
	}
}

#define HIGHESTREQUEST 20

void RunSysInfo(instructionPointer *ip)
{
	FUNGEDATATYPE request = StackPop(ip->stack);
	if (request == 23)
		PushRequest(18, ip);
	else if (request > 0)
		PushRequest(request, ip);
	else
		for (int i = HIGHESTREQUEST; i > 0; i--)
			PushRequest(i, ip);
}
