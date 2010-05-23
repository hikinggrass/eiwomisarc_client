/*****************************************************************************
 * functions.h: global functions
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

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* get integer random number in range a <= x <= e 
 source: http://cplus.kompf.de/artikel/random.html*/
int irand( int a, int e);

unsigned char itouc(int pInt);

int split(char *str, int size, int *rueck);

void fillsending(int val, int ch, int *psending);

/* FIXME: ARTNET Protocol */
void fill_artnet(int *val, int channel, int *data);

void sendoverudp(char *pip, int pport, int *psending);