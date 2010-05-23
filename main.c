/*****************************************************************************
 * eiwomisarc_client:	A UDP-client that sends messages
 *						to the eiwomisarc_server
 *****************************************************************************
 * Copyright (C) 2009-2010 Kai Hermann
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

#include "git_rev.h"

#include "worldmood.h"

#define VERSION "0.2"
#define PROGNAME "eiwomisarc_client"
#define COPYRIGHT "2009-2010, Kai Hermann"

#define EIWOMISA 0
#define ARTNET 1

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "messages.h"

/* argtable */
#include "argtable2/argtable2.h"

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


int mymain(const char* progname, char *ip, int port, struct arg_str *values, struct arg_str *channels, struct arg_str *mixed)
{
	//FIXME: request_twitter();
	
	/* check if ip & port are set, otherwise use defaults */
	if(ip == NULL) {
		printf("No Server specified - trying localhost...\n",progname);
		ip = "127.0.0.1";
	}
	if(port == -1) {
		printf("No Port specified - using 1337.\n",progname);
		port = 1337;
	}

	/* init sending array */
	int sending[6];
	sending[0] = 255; /* Startbyte */

	/* still needed? FIXME! 
	//sending[1] = 254; //Values1
	//sending[2] = 1;   //Values2
	//sending[3] = 0;   //Channel1
	//sending[4] = 0;   //Channel2
	//sending[5] = 0;   //Channel3 */

	int packets = 0;
	int channels_count = 0;

	if(values->count>0) {
		/* we have teh valuez */

		/* split the -v command-line option */
		char t_values[strlen(*values->sval)];
		strncpy( t_values, values->sval[0], strlen(*values->sval));

		int i_values[8];

		/* how many values (packets)? */
		packets = split(t_values,8, i_values);

		if(channels->count>0) {
			/* split the -c command-line option */
			char t_channels[strlen(*channels->sval)];
			strncpy(t_channels, channels->sval[0], strlen(*channels->sval));

			int i_channels[8];

			/* how many packets? */
			channels_count = split(t_channels,8, i_channels);

			if(channels->count>=packets) {
				/* use only as many values as channels */
				for(int i=0; i<packets; i++) {
					fillsending(i_values[i], i_channels[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else if(channels->count<packets) {
				/* use only as many values as channels */
				for(int i=0; i<channels->count; i++) {
					fillsending(i_values[i], i_channels[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else {
				/* number of values equals number of channels */
				die("values=channels, This could and should never happen\n");
			}
		}else {
			/* default-channels */
			for(int i=0; i<packets; i++) {
				fillsending(i_values[i], i, sending);
				sendoverudp(ip, port, sending);
			}
			int sending_artnet[531];
			fill_artnet(i_values, packets, sending_artnet);
			if(packets>1)
			{
				packets=1;
			}
		}
	}else if(mixed->count>0) { /* fixme! */
		/* mixed-mode "hell yeah" */

		/* split the command-line optins */
		char t_values[strlen(*values->sval)];
		strncpy(t_values, values->sval[0], strlen(*values->sval));

		int i_values[8];

		/* how many packets? */
		packets = split(t_values,8, i_values);
		
		if( (packets%2) > 0)
			packets -= 1;
		
		for(int i=0; i<packets; i=i+2) {
			fillsending(i_values[i], i, sending);
			sendoverudp(ip, port, sending);
		}
	}
    return 0;
}


int main(int argc, char **argv)
{	
	/* init random number generator */
	srand(time(0));

	/* server (ip adress, FIXME: check with regex) */
	struct arg_str *serverip = arg_str0("sS","server,ip","","ip address of the server, default: localhost");

	struct arg_int *serverport = arg_int0("pP","port","","serverport, default: 1337");
	struct arg_str *values = arg_strn("vV","values","",0,1,"up to 4 values separated by ',' - range 0-255, default: 0, negative values: random");
	struct arg_str *channels = arg_strn("cC","channels","",0,1,"up to 4 channels separated by ',' - range 0-512, default 0-3");
	struct arg_str *mixed = arg_strn("mM","mixed","",0,1,"set values for corresponding channels. Format: <channel0>,<value0>,<channel1>,[...]");

	struct arg_lit  *worldmood = arg_lit0(NULL,"mood",                 "experimental (twitter) worldmood mode");

    struct arg_lit  *help    = arg_lit0("hH","help",                    "print this help and exit");
    struct arg_lit  *version = arg_lit0(NULL,"version",                 "print version information and exit");

    struct arg_end  *end     = arg_end(20);

    void* argtable[] = {serverip,serverport,values,channels,mixed,worldmood,help,version,end};    

    int nerrors;
    int exitcode=0;

    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0) {
        /* NULL entries were detected, some allocations must have failed */
        printf("%s: insufficient memory\n",PROGNAME);
        exitcode=1;
        goto exit;
	}
	
    /* set any command line default values prior to parsing */
	/* nothing */

    /* Parse the command line as defined by argtable[] */
    nerrors = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
		printf("Usage: %s", PROGNAME);
        arg_print_syntax(stdout,argtable,"\n");
        printf("A UDP-client that sends messages to the eiwomisarc_server\n");
        arg_print_glossary(stdout,argtable,"  %-25s %s\n");
        exitcode=0;
        goto exit;
	}

    /* special case: '--version' takes precedence error reporting */
    if (version->count > 0) {
		printf("'%s' version ",PROGNAME);
		printf(VERSION);
		printf("\nGIT-REVISION: ");
		printf(GITREV);
        printf("\nA UDP-client that sends messages to the eiwomisarc_server\n");
        printf(COPYRIGHT);
		printf("\n");
		exitcode=0;
        goto exit;
	}

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0) {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout,end,PROGNAME);
        printf("Try '%s --help' for more information.\n",PROGNAME);
        exitcode=1;
        goto exit;
	}

    /* special case: uname with no command line options induces brief help */
    if (argc==1) {
        printf("Try '%s --help' for more information.\n",PROGNAME);
        exitcode=0;
        goto exit;
	}

	/* special case: more values than channels & channels > 0 */
	if( (channels->count != values->count) && (channels->count > 0) ) {
		printf("Number of channels does not match the number of values!\n",PROGNAME);
        exitcode=1;
	}

	/* special case: values or channels + mixed mode */
	if( (mixed->count > 0) && ( (channels->count > 0) || (values->count > 0) ) ) {
		printf("Please do not mix --values or --channels with --mixed!\n",PROGNAME);
        exitcode=1;
		goto exit;
	}

    /* normal case: take the command line options at face value */

	/* check if server ip is set */
	char *c_serverip = NULL;
	if(serverip->count>0)
		c_serverip = (char *)serverip->sval[0];

	/* check if server port is set */
	int i_serverport = -1;
	if(serverport->count>0)
		i_serverport = (int)serverport->ival[0];

	exitcode = mymain(PROGNAME, c_serverip, i_serverport, values, channels, mixed);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
}
