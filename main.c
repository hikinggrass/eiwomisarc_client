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

#define VERSION "0.2"
#define PROGNAME "eiwomisarc_client"
#define COPYRIGHT "2009-2010, Kai Hermann"

#define EIWOMISA 0
#define ARTNET 1

//#include <stdio.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <netinet/in.h>

#include "messages.h"
#include "functions.h"

/* argtable */
#include "argtable2/argtable2.h"

int normalmode(const char* progname, char *ip, int port, struct arg_str *values, struct arg_str *channels, struct arg_str *mixed)
{
	/* check if ip & port are set, otherwise use defaults */
	if(ip == NULL) {
		msg_Info("No Server set - trying localhost...");
		ip = "127.0.0.1";
	}
	if(port == -1) {
		msg_Info("No Port set - using 1337.");
		port = 1337;
	}

	/* init sending array */
	int sending[6];
	sending[0] = 255; /* Startbyte */

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
				int i;
				for(i=0; i<packets; i++) {
					fillsending(i_values[i], i_channels[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else if(channels->count<packets) {
				/* use only as many values as channels */
				int i;
				for(i=0; i<channels->count; i++) {
					fillsending(i_values[i], i_channels[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else {
				/* number of values equals number of channels */
				die("values=channels, This could and should never happen\n");
			}
		}else {
			/* default-channels */
			int i;
			for(i=0; i<packets; i++) {
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
		int i;
		for(i=0; i<packets; i=i+2) {
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

    struct arg_lit  *help    = arg_lit0("hH","help",                    "print this help and exit");
    struct arg_lit  *version = arg_lit0(NULL,"version",                 "print version information and exit");

	struct arg_lit  *debug = arg_lit0(NULL,"debug","print debug messages");
    struct arg_lit  *silent = arg_lit0(NULL,"silent","print no messages");

    struct arg_end  *end     = arg_end(20);

    void* argtable[] = {serverip,serverport,values,channels,mixed,help,version,debug,silent,end};

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

	/* --debug enables debug messages */
    if (debug->count > 0) {
		printf("debug messages enabled\n");
		msglevel = 3;
	}

	/* --silent disables all (!) messages */
    if (silent->count > 0) {
		printf("i'll be silent now...\n");
		msglevel = 0;
	}
	
	exitcode = normalmode(PROGNAME, c_serverip, i_serverport, values, channels, mixed);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
}
