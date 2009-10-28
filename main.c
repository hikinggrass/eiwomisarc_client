#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

//argtable
#include "argtable2/argtable2.h"

//regular expressions
#include <regex.h>

//logging - später evtl mal
//#include <asl.h>

/* get integer random number in range a <= x <= e */
//source: http://cplus.kompf.de/artikel/random.html
int irand( int a, int e)
{
    double r = e - a + 1;
    return a + (int)(r * rand()/(RAND_MAX+1.0));
}

unsigned char itouc(int pInt){
	//int Wert = atoi(i_values[0]);
	//int Wert = i_values[0];
	//int Wert;
	unsigned char Byte;

	//Byte1 = ( unsigned char ) ( Wert >> 8 );
	return Byte = ( unsigned char ) ( pInt );
}

void Die(char *message)
{
	perror(message);
	exit(1);
}

int split(char *str, int size, int *rueck)
{	
	char *p;
	printf("Split \"%s\" in tokens:\n", str); //debug
	
	p = strtok (str,",");
	
	int i=0;
	
	while ( (p != NULL) && (i < size) )
	{
		rueck[i] = atoi(p);
		printf ("%s\n", p); //debug
		p = strtok (NULL, " ,");
		i++;
	}
	return i;
}

void fillsending(int val, int ch, int *psending)
{
	//random?
	if(val<0){
		val = irand(0, 255);
	}

	//value
	if(val>254){
		psending[1] = 254; //Values1
		psending[2] = 1;   //Values2
	}else{
		psending[1] = val; //Values1
		psending[2] = 0;   //Values2
	}

	//channel
	if(ch>512){
		psending[3] = 254;		//Channel1
		psending[4] = 254;		//Channel2
		psending[5] = 4;		//Channel3
	}else if(ch>508){
		psending[3] = 254;		//Channel1
		psending[4] = 254;		//Channel2
		psending[5] = ch-508;	//Channel3
	}else if(ch>254){
		psending[3] = 254;		//Channel1
		psending[4] = ch-254;	//Channel2
		psending[5] = 0;		//Channel3
	}else{
		psending[3] = ch;		//Channel1
		psending[4] = 0;		//Channel2
		psending[5] = 0;		//Channel3
	}
}

void sendoverudp(char *pip, int pport, int *psending)
{
	int sock;
	struct sockaddr_in echoserver;
	unsigned int echolen;

	unsigned char transmit[6];

	//convert psending int -> unsigned char for transmission
	for(int i=0; i<6; i++){
		transmit[i] = itouc(psending[i]);
	}

	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));		/* Clear struct */
	echoserver.sin_family = AF_INET;				/* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(pip);	/* IP address */
	echoserver.sin_port = htons(pport);				/* server port */		

	echolen = 6;

	//Send the data
	if (sendto(sock, transmit, echolen, 0,
			   (struct sockaddr *) &echoserver,
			   sizeof(echoserver)) != echolen) {
		Die("Mismatch in number of sent bytes");
	}

	//close the socket
	close(sock);
}


int mymain(const char* progname, char *ip, int port, struct arg_str *values, struct arg_str *channels, struct arg_str *mixed)
{
	//check if ip & port are set, otherwise use defaults
	if(ip == NULL) {
		printf("No Server specified - trying localhost...\n",progname);
		ip = "127.0.0.1";
	}
	if(port == -1) {
		printf("No Port specified - using 1337.\n",progname);
		port = 1337;
	}

	//init sending array
	int sending[6];
	sending[0] = 255; //Startbyte

	//still needed? FIXME! 
	sending[1] = 254; //Values1
	sending[2] = 1;   //Values2
	sending[3] = 0;   //Channel1
	sending[4] = 0;   //Channel2
	sending[5] = 0;   //Channel3

	//how many packets?
	int packets = 0;

	if(values->count>0){
		//we have teh valuez

		//split the command-line optins
		char t_values[strlen(*values->sval)];
		//char *t_values;
		int i_values[8];
		strcpy ( t_values, values->sval[0] );
		//how many packets?
		packets = split(t_values,8, i_values);

		if(channels->count>0) {
			if(channels->count>=packets) {
				//use only so many channels as values
				for(int i=0; i<packets; i++){
					fillsending((int)values->sval[i], (int)channels->sval[i], sending);
					sendoverudp(ip, port, sending);
				}

			}else if(channels->count<packets){
				//use only so many values as channels
				for(int i=0; i<channels->count; i++){
					fillsending((int)values->sval[i], (int)channels->sval[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else{
				//number of values equals number of channels
				Die("values=channels, This could and should never happen\n");
			}
		}else{
			//default-channels
			for(int i=0; i<packets; i++){
				fillsending(atoi(values->sval[i]), i, sending);
				sendoverudp(ip, port, sending);
			}
		}
	}else if(mixed->count>0){
		//mixed-mode "hell yeah"

		//split the command-line optins
		char t_values[strlen(*values->sval)];
		//char *t_values;
		int i_values[8];
		strcpy ( t_values, values->sval[0] );
		//how many packets?
		packets = split(t_values,8, i_values);
		
		if( (packets%2) > 0)
			packets -= 1;
		
		for(int i=0; i<packets; i=i+2){
			fillsending(atoi(values->sval[i]), i, sending);
			sendoverudp(ip, port, sending);
		}
	}
    return 0;
}


int main(int argc, char **argv)
{	
	//init random number generator
	srand(time(0));

	//server (ip adress, FIXME: check with regex)
	struct arg_str *serverip = arg_str0("sS","server,ip","","specify the ip address of the server, default: localhost");

	struct arg_int *serverport = arg_int0("pP","port","","specify the serverport, default: 1337");
	struct arg_str *values = arg_strn("vV","values","",0,1,"specify up to 4 values separated by ',' - range 0-255, default: 0, negative values: random");
	struct arg_str *channels = arg_strn("cC","channels","",0,1,"specify up to 4 channels separated by ',' - range 0-512, default 0-3");
	struct arg_str *mixed = arg_strn("mM","mixed","",0,1,"set values for corresponding channels. Format: <channel0>,<value0>,<channel1>,[...]");


    struct arg_lit  *help    = arg_lit0("hH","help",                    "print this help and exit");
    struct arg_lit  *version = arg_lit0(NULL,"version",                 "print version information and exit");

    struct arg_end  *end     = arg_end(20);

    void* argtable[] = {serverip,serverport,values,channels,mixed,help,version,end};    

	const char* progname = "udp_client_cmd"; //fixme!
    int nerrors;
    int exitcode=0;

    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0)
	{
        /* NULL entries were detected, some allocations must have failed */
        printf("%s: insufficient memory\n",progname);
        exitcode=1;
        goto exit;
	}
	
    /* set any command line default values prior to parsing */
	//nothing

    /* Parse the command line as defined by argtable[] */
    nerrors = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0)
	{
        printf("Usage: %s", progname);
        arg_print_syntax(stdout,argtable,"\n");
        printf("A client that sends udp-packets to a udp-server which controls\n");
		printf("the EIWOMISA controller over RS-232\n");
        arg_print_glossary(stdout,argtable,"  %-25s %s\n");
        exitcode=0;
        goto exit;
	}

    /* special case: '--version' takes precedence error reporting */
    if (version->count > 0)
	{
        printf("'%s' version 0.1\n",progname);
        printf("A client that sends udp-packets to a udp-server which controls\n");
		printf("the EIWOMISA controller over RS-232\n");
        printf("September 2009, Kai Hermann\n");
        exitcode=0;
        goto exit;
	}

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0)
	{
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout,end,progname);
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=1;
        goto exit;
	}

    /* special case: uname with no command line options induces brief help */
    if (argc==1)
	{
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=0;
        goto exit;
	}

	/* special case: more values than channels & channels > 0 */
	if( (channels->count != values->count) && (channels->count > 0) )
	{
		printf("Number of specified channels does not match the number of specified values!\n",progname);
        exitcode=1;
	}

	/* special case: values or channels + mixed mode */
	if( (mixed->count > 0) && ( (channels->count > 0) || (values->count > 0) ) )
	{
		printf("Please do not mix --values or --channels with --mixed!\n",progname);
        exitcode=1;
		goto exit;
	}

    /* normal case: take the command line options at face value */

	//check if server ip is set
	char *c_serverip = NULL;
	if(serverip->count>0)
		c_serverip = (char *)serverip->sval[0];

	//check if server port is set
	int i_serverport = -1;
	if(serverport->count>0)
		i_serverport = (int)serverport->ival[0];

	exitcode = mymain(progname, c_serverip, i_serverport, values, channels, mixed);

exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
}

      