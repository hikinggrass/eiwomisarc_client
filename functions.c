/*****************************************************************************
 * functions.c: global functions
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

#include "functions.h"

/* get integer random number in range a <= x <= e 
 source: http://cplus.kompf.de/artikel/random.html*/
int irand( int a, int e)
{
    double r = e - a + 1;
    return a + (int)(r * rand()/(RAND_MAX+1.0));
}

unsigned char itouc(int pInt)
{
	unsigned char byte;
	return byte = ( unsigned char ) ( pInt );
}

int split(char *str, int size, int *rueck)
{	
	char *p;
	msg_Dbg("Split \"%s\" in tokens:", str);
	p = strtok(str,",");
	
	int i=0;
	
	while( (p != NULL) && (i < size) )
	{
		rueck[i] = atoi(p);
		msg_Dbg("%s", p); /* debug */
		p = strtok (NULL, " ,");
		i++;
	}
	return i;
}

void fillsending(int val, int ch, int *psending)
{
	/* negative value = random value! */
	if(val<0) {
		val = irand(0, 255);
	}
	
	/* value */
	if(val>254) {
		psending[1] = 254; /* Values1 */
		psending[2] = 1;   /* Values2 */
	}else {
		psending[1] = val; /* Values1 */
		psending[2] = 0;   /* Values2 */
	}
	
	/* channel */
	if(ch>512) {
		psending[3] = 254;		/* Channel1 */
		psending[4] = 254;		/* Channel2 */
		psending[5] = 4;		/* Channel3 */
	}else if(ch>508) {
		psending[3] = 254;		/* Channel1 */
		psending[4] = 254;		/* Channel2 */
		psending[5] = ch-508;	/* Channel3 */
	}else if(ch>254) {
		psending[3] = 254;		/* Channel1 */
		psending[4] = ch-254;	/* Channel2 */
		psending[5] = 0;		/* Channel3 */
	}else {
		psending[3] = ch;		/* Channel1 */
		psending[4] = 0;		/* Channel2 */
		psending[5] = 0;		/* Channel3 */
	}
}

/* FIXME: ARTNET Protocol */
void fill_artnet(int *val, int channel, int *data) {
	
	// ID
	data[0] = 'A';
	data[1] = 'r';
	data[2] = 't';
	data[3] = '-';
	data[4] = 'N';
	data[5] = 'e';
	data[6] = 't';
	data[7] = 0x00;
	
	// OpCode
	data[8] = 0x00;
	data[9] = 0x50;
	
	// ProtVerH
	data[10] = 0x00;
	//ProtVer
	data[11] = 0x0E;
	
	// Sequence
	data[12] = 0x00;
	
	// Physical
	data[13] = 0x00;
	
	// Universe
	data[14] = 0x00; // <- Universe Setting
	data[15] = 0x00;
	
	// LengthHi
	data[16] = (channel / 0xFF); // Length High Byte
	// Length
	data[17] = (channel % 0xFF); // Length Low Byte
	
	// Data[Length]
	for (int i = 0; i < channel; i++) {
		data[18 + i] = val[i]; // Insert useful data here ;)
	}
}

void sendoverudp(char *pip, int pport, int *psending)
{
	int sock;
	struct sockaddr_in echoserver;
	unsigned int echolen;
	
	unsigned char transmit[6];
	
	/* convert psending int -> unsigned char for transmission */
	for(int i=0; i<6; i++) {
		transmit[i] = itouc(psending[i]);
	}
	
	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		die("Failed to create socket");
	}
	
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));		/* Clear struct */
	echoserver.sin_family = AF_INET;				/* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(pip);	/* IP address */
	echoserver.sin_port = htons(pport);				/* server port */		
	
	echolen = 6;
	
	/* Send the data */
	if (sendto(sock, transmit, echolen, 0,
			   (struct sockaddr *) &echoserver,
			   sizeof(echoserver)) != echolen) {
		die("Mismatch in number of sent bytes");
	}
	
	/* close the socket */
	close(sock);
}