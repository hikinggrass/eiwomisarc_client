/*****************************************************************************
 * messages.h: show messages
 *****************************************************************************
 * Copyright (C) 2010 Kai Hermann
 *
 * Authors: Kai Hermann <kai.uwe.hermann at gmail dot com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "messages.h"

/* default message level: 2 = error&info messages*/
int msglevel = 2;

void msg_Dbg(char *fmt, ...)
{
	if (msglevel>2) {
		va_list argp;
		printf("debug: ");
		va_start(argp, fmt);
		vprintf(fmt, argp);
		va_end(argp);
		printf("\n");
	}
}

void msg_Info(char *fmt, ...)
{
	if (msglevel>1) {
		va_list argp;
		printf("info: ");
		va_start(argp, fmt);
		vprintf(fmt, argp);
		va_end(argp);
		printf("\n");
	}
}

void msg_Err(char *fmt, ...)
{
	if (msglevel>0) {
		va_list argp;
		printf("ERROR: ");
		va_start(argp, fmt);
		vprintf(fmt, argp);
		va_end(argp);
		printf("\n");
	}
}

void die(char *message)
{
	perror(message);
	exit(1);
}
