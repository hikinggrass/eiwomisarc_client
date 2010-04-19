/* beautiful debug messages */
#include <stdarg.h>

/* FIXME! parse msglevel parameter, default 0 = no messages*/
int msglevel = 2;

void msg_Dbg(char *fmt, ...)
{
	if (msglevel>1) {
		va_list argp;
		printf("debug: ");
		va_start(argp, fmt);
		vprintf(fmt, argp);
		va_end(argp);
		printf("\n");
	}
}
