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


//#define BUFFSIZE 255

void Die(char *message)
{
	perror(message);
	exit(1);
}

int split(char str[], int size, char *rueck[])
{	
	char *p;

	printf ("Split \"%s\" in tokens:\n", str);
	
	p = strtok (str,",");
	
	int i=0;
	
	while ( (p != NULL) && (i < size) )
	{
		rueck[i] = p;
		printf ("%s\n", p);
		p = strtok (NULL, " ,");
		i++;
	}
	return i;
}

void fillsending(int val, int ch, unsigned char psending[]){
	//value
	
	if(val>254){
		psending[1] = 254; //Wert1
		psending[2] = 1;   //Wert2
	}else{
		psending[1] = val; //Wert1
		psending[2] = 0;   //Wert2
	}
	
	//channel
	if(ch>512){
		psending[3] = 254;		//Kanal1
		psending[4] = 254;		//Kanal2
		psending[5] = 4;		//Kanal3
	}else if(ch>508){
		psending[3] = 254;		//Kanal1
		psending[4] = 254;		//Kanal2
		psending[5] = ch-508;	//Kanal3
	}else if(ch>254){
		psending[3] = 254;		//Kanal1
		psending[4] = ch-254;	//Kanal2
		psending[5] = 0;		//Kanal3
	}else{
		psending[3] = ch;		//Kanal1
		psending[4] = 0;		//Kanal2
		psending[5] = 0;		//Kanal3
	}
}

void sendoverudp(char *pip, int pport, unsigned char psending[]){
	int sock;
	struct sockaddr_in echoserver;
	//struct sockaddr_in echoclient;
	//char buffer[BUFFSIZE];
	unsigned int echolen;			
	
	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(pip);  /* IP address */
	echoserver.sin_port = htons(pport);       /* server port */		
	
	echolen = 6;
	
	//Send the data
	if (sendto(sock, psending, echolen, 0,
			   (struct sockaddr *) &echoserver,
			   sizeof(echoserver)) != echolen) {
		Die("Mismatch in number of sent bytes");
	}
	
	//close the socket
	close(sock);
	
}


int mymain(char *ip, int port, struct arg_str *values, struct arg_str *channels, struct arg_str *mixed)
		   /*int l, int R, int k,
           const char **defines, int ndefines,
           const char *outfile,
           int v,
           const char **infiles, int ninfiles*/
	{
/*
	int i;
	
    if (l>0) printf("list files (-l)\n");
    if (R>0) printf("recurse through directories (-R)\n");
    if (v>0) printf("verbose is enabled (-v)\n");
    printf("scalar k=%d\n",k);
    printf("output is \"%s\"\n", outfile);
	
    for (i=0; i<ndefines; i++)
        printf("user defined macro \"%s\"\n",defines[i]);
	
    for (i=0; i<ninfiles; i++)
        printf("infile[%d]=\"%s\"\n",i,infiles[i]);
*/
	
	//my f*cking code!
	
	//ip? port?
	if(ip == NULL) {
		fprintf(stdout,"No Server specified - trying localhost...\n");
		ip = "127.0.0.1";
	}
	if(port == -1) {
		fprintf(stdout,"No Port specified - using 1337.\n");
		port = 1337;
	}
		
		
	//init sending array
	unsigned char sending[6];
	sending[0] = 255; //Startbyte
		sending[1] = 254; //Wert1
		sending[2] = 1;   //Wert2
		sending[3] = 0;   //Kanal1
		sending[4] = 0;   //Kanal2
		sending[5] = 0;   //Kanal3
		
	//how many packets?
	int packets = 0;
	
		
	if(values->count>0){
		//we have teh valuez
		
		char *test;
		char *i_values[4];
		strcpy ( test, values->sval[0] );
		//how many packets?
		packets = split(test,4, i_values);
		
		if(channels->count>0) {
			if(channels->count>=packets) {
				//nur die menge channels wie values
				for(int i=0; i<packets; i++){
					fillsending((int)values->sval[i], (int)channels->sval[i], sending);
					sendoverudp(ip, port, sending);
				}
					
			}else if(channels->count<packets){
				//nur menge values wie channels
				for(int i=0; i<channels->count; i++){
					fillsending((int)values->sval[i], (int)channels->sval[i], sending);
					sendoverudp(ip, port, sending);
				}
			}else{
				//menge values = menge channels
				Die("values=channels, This could and should never happen");
			}
		}else{
			//standard-channels
			for(int i=0; i<packets; i++){
				fillsending((int)i_values[i], i, sending);
				sendoverudp(ip, port, sending);
			}
		}
	}else if(mixed->count>0){
		//mixed-mode "hell yeah"
		
		//split the command-line optins
		char *t_mixed;
		char *i_mixed[8];
		strcpy ( t_mixed, mixed->sval[0] );
		//how many packets?
		packets = split(t_mixed,8, i_mixed);
		
		if( (packets%2) > 0)
			packets -= 1;
		
		for(int i=0; i<packets; i=i+2){
			fillsending((int)i_mixed[i+1],(int)i_mixed[i], sending);
			sendoverudp(ip, port, sending);
		}
		
	}
		
		
/*	if(values->count<=0){
		//FIXME!
		fprintf(stdout, "lalala");  //FIXME - irgendwas stimmt hier noch nicht
	}else{ //Values set
		
		
		//values behandeln
		packets = values->count;
		
		for(int i=0; i<packets; i++){
			int temp = (int)values->sval[i];
			if(temp>254) {
				sending[1] = 254; //Wert
				sending[2] = 1;   //Wert
			}else{
				sending[1] = temp;
				sending[2] = 0;
			}
			
			sending[3] = 0;   //Kanal
			sending[4] = 0;   //Kanal
			sending[5] = 0;   //Kanal
			
			if(channels->count == 0){
				sending[3] = i;
			} else {
				temp = (int)channels->sval[i];
				if(temp>0)
				{
					if(temp>254){
						sending[3] = 254;
					}else{
						sending[3] = temp;
					}
				
					if(temp>508) {
						sending[4] = 254;   //Kanal
					}else{
						sending[4] = temp-254;
					}
					if(temp>512) {
						sending[5] = 4;   //Kanal
					}else{
						sending[5] = temp-508; //Kanal
					}
				}
			}
						
		}
	}
*/			
		
		
	/*int sock;
	struct sockaddr_in echoserver;
	//struct sockaddr_in echoclient;
	//char buffer[BUFFSIZE];
	unsigned int echolen;
	//unsigned in clientlen;
	//int received = 0;
	
	//if (argc != 4) {
	//	fprintf(stderr, "USAGE: %s <server_ip> <word> <port>\n", argv[0]);
	//	exit(1);
	//}
	*/
	/* Create the UDP socket */
	/*if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}*/
	/* Construct the server sockaddr_in structure */
	//memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	//echoserver.sin_family = AF_INET;                  /* Internet/IP */
	//echoserver.sin_addr.s_addr = inet_addr(ip);  /* IP address */
	//echoserver.sin_port = htons(port);       /* server port */
	
	/* Send the word to the server */
	/*char sending[6];
	 sending[0] = 'a'; //Startbyte
	 sending[1] = 'b'; //Wert
	 sending[2] = 'c';   //Wert
	 sending[3] = 'd';   //Kanal
	 sending[4] = 'e';   //Kanal
	 sending[5] = 'f';   //Kanal
	 */
	//unsigned char sending[6];
	/*sending[0] = 255; //Startbyte
	sending[1] = 254; //Wert
	sending[2] = 1;   //Wert
	sending[3] = 0;   //Kanal
	sending[4] = 0;   //Kanal
	sending[5] = 0;   //Kanal
	
	echolen = 6;

	
	if (sendto(sock, sending, echolen, 0,
			   (struct sockaddr *) &echoserver,
			   sizeof(echoserver)) != echolen) {
		Die("Mismatch in number of sent bytes");
	}*/
	
	/* Receive the word back from the server */
	/* fprintf(stdout, "Received: ");
	 clientlen = sizeof(echoclient);
	 if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
	 (struct sockaddr *) &echoclient,
	 &clientlen)) != echolen) {
	 Die("Mismatch in number of received bytes");
	 }*/
	/* Check that client and server are using same socket */
	/* if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
	 Die("Received a packet from an unexpected server");
	 }*/
	//buffer[received] = '\0';        /* Assure null terminated string */
	/*fprintf(stdout, buffer);
	 fprintf(stdout, "\n");*/
	//close(sock);
	exit(0);
	
	//end of my f*ucking code!
	
    return 0;
}


int main(int argc, char **argv)
{	
	//server (ip adress, FIXME: check with regex)
	struct arg_str *serverip = arg_str0("sS","server,ip","","specify the ip address of the server, default: localhost");
	
	struct arg_int *serverport = arg_int0("pP","port","","specify the port of the server, default: 1337");
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
	
	exitcode = mymain(c_serverip, i_serverport, values, channels, mixed);
	
exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
	
    return exitcode;
}

      