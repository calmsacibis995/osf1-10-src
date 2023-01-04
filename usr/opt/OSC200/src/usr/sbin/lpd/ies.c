/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ies.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:14:14 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * ies: Applications level handler for Imagen IMPRINT-10 TCP/IP ethernet
 * unit.  This unit forks off a separate process to gather status information.
 */

/* Get environment definitions */

# include "ies.h"			/* also includes site.h */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/file.h>

#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <locale.h>
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)


#define STREAMMARK 0400			/* EOF detected on input */
#define ENDMARK	   0404			/* redundant STREAMMARK */

#define IPPORT_PRINTER 35
#define STATUSINT      15000		/* status update interval (msecs) */

/* Globals */
extern int errno;
extern *sys_errlist[];

char *udpstatus();
extern int udpready;

char *hname = NIL;			/* host name of printer */

struct sockaddr_in sin = { AF_INET };
int s = -1;				/* socket file descriptor */

int pid;				/* process id of fork getting
					   status information */

/* input source information */
struct infile {
	char	*name;			/* file name, if appropriate */
	BOOL	stdiswitch;		/* TRUE if input is from user's
					   terminal */
	BOOL	rwdin;			/* TRUE if input can be rewound */
	BOOL	finished;		/* TRUE if we have reached EOF */
} infile;

char *logname = DEFAULTLOG;		/* Name of log file ("-" if user's
							terminal */
char *attnFile = DEFAULTATTN;		/* Name of file in which to deposit
					   status notes */
char *hostFile = NIL;			/* file containing printer host name
					   and accounting file */
char *prestring = 0;			/* A string to be prepended to the
					   file or stream being sent */
int preindex = 0;			/* index into prestring */

char Printcap[1024];			/* for holding /etc/printcap entry */
BOOL hardset = FALSE;			/* true if host name explicitly set */
BOOL hold = FALSE;			/* true if we are a child and should
					   not be sending to the log file */

/* Enter here */

main(argc, argv)
int argc;
char *argv[];
{
	char name[50];
	struct hostent *host = NIL;	/* host table entry pointer */
	int onoff;

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
	PHintrp();			/* set up interrupts */
	SETDEFAULTS
	PHscanargs(argc, argv);

	/* get our log file if any */
	if (!PHl_open(logname, FALSE))
		PHdie(-1, MSGSTR(IES_1, "%s: can't open for write as log file\n"), logname);

	/* open the input file */
	if (!PHi_open(infile.name)) 
		PHdie(-1, MSGSTR(IES_2, "%s: can't open for read\n"), infile.name);
	infile.finished = FALSE;

	/* open file containing host name, if we don't already have a name */
	if (hname == NIL)
		PHdie(-1, MSGSTR(IES_3, "No printer identification provided\n"));
	PHnote(MSGSTR(IES_4, "host name %s\n"), hname);
	
	/* get host table entry for the host, and set up local socket
	   structure */
	host = gethostbyname(hname);
	if (host != NIL) {
		sin.sin_family = host->h_addrtype;
		bcopy(host->h_addr, (caddr_t)&sin.sin_addr, host->h_length);
	}
	else {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(hname);
		if (sin.sin_addr.s_addr == -1)
			PHdie(-1, MSGSTR(IES_5, "Can't get internet address for %s\n"),
								     hname);
	}
	sin.sin_port = htons(IPPORT_PRINTER);

	/* get a socket on this machine */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		PHdie(-1, MSGSTR(IES_6, "No sockets available\n"));

	/* set up socket options */
	onoff = 1;
#ifdef DEBUG
	if (setsockopt(s, SOL_SOCKET, SO_DEBUG, &onoff, sizeof(onoff)) < 0)
		PHdie(-1, MSGSTR(IES_7, "Could not set up socket debug option\n"));
#endif DEBUG
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &onoff, sizeof(onoff)) < 0)
		PHdie(-1, MSGSTR(IES_8, "Could not set up socket keepalive option\n"));
#else
#ifdef DEBUG
	if (setsockopt(s, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
		PHdie(-1, MSGSTR(IES_7, "Could not set up socket debug option\n"));
#endif DEBUG
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0) < 0)
		PHdie(-1, MSGSTR(IES_8, "Could not set up socket keepalive option\n"));

	/* connect our socket to one on the printer */
	while (TRUE) {
		char *cp;

		/* wait for printer to become ready */
		PHnote(MSGSTR(IES_9, "Checking if printer is ready\n"));
		PHnote(MSGSTR(IES_10, "doing udpstatus: sin_family %d sin_port %d sin_addr %x\n"), sin.sin_family, sin.sin_port, sin.sin_addr.s_addr);
		cp = udpstatus(0, FIRSTTIME);	/* get printer status */
		PHnote(MSGSTR(IES_11, "did udpstatus: sin_family %d sin_port %d sin_addr %x\n"), sin.sin_family, sin.sin_port, sin.sin_addr.s_addr);
		while (udpready == 0) {
			PHnote(MSGSTR(IES_12, "IMPRINT status: Delaying connection: %s\n"),
									  cp);
			PHsleep(15000);
		PHnote(MSGSTR(IES_13, "doing udpstatus again: sin_family %d sin_port %d sin_addr %x\n"), sin.sin_family, sin.sin_port, sin.sin_addr.s_addr);
			cp = udpstatus(0);
		PHnote(MSGSTR(IES_14, "did udpstatus again: sin_family %d sin_port %d sin_addr %x\n"), sin.sin_family, sin.sin_port, sin.sin_addr.s_addr);
		}
		PHnote(MSGSTR(IES_15, "IMPRINT status: %s\n"), cp);

		/* report and attempt to make connection */
		PHnote(MSGSTR(IES_16, "Attempting to open connection\n"));
		PHnote(MSGSTR(IES_17, "doing connect: sin_family %d sin_port %d sin_addr %x\n"), sin.sin_family, sin.sin_port, sin.sin_addr.s_addr);
		if (connect(s, &sin, sizeof(sin))) {
			/* check to see if we timed out */
			if (errno == ETIMEDOUT) {
				PHnote(MSGSTR(IES_18, "Connect timeout, retrying\n"));
				PHAttR(3,
				MSGSTR(IES_19, "Imprint status: Connect timeout, retrying\n")
									    );
				PHsleep(60000);
			}
			else 
				PHdie(-1, MSGSTR(IES_20, "Connect failure: %s\n"),
							sys_errlist[errno]);
		}
		else {
			/* report completion of connection */
			PHnote(MSGSTR(IES_21, "Connected to %s, sending job\n"), hname);
			break;
		}
	}

	/* fork off a child to do the status gathering.  First flush
	   logging information to avoid duplication */
	PHl_flush();
	if ((pid = fork()) < 0) 
		PHdie(-1, MSGSTR(IES_22, "Unable to start fork.\n"));
	if (pid == 0) { 
		int PHqdie;

		/* close the socket */
		close(s);
		s = -1;		/* to prevent attempting to close on death */

		/* close the log file, and set up for a quiet death */
		hold = TRUE;
		PHl_close();

		/* keep gathering status data until killed */
		updatestatus();
	}
	else {
		/* send the file */
		PHnote(MSGSTR(IES_23, "Sending File\n"));
		PHsend(s);

		/* kill the status gathering process */
		kill(pid, SIGINT);

		/* wait for the child process to die */
		while (wait(0) > 0);

		/* report successful completion of the job */
		PHAttR(6, MSGSTR(IES_24, "IMPRINT status: Job successfully sent\n"));

		/* and exit */
		PHdie(0, MSGSTR(IES_25, "Normal completion\n"));
	}
}


/*
 * Scan the arguments.  The following is a list of arguments which the
 * various people who work on this program will find time to update:
 *
 *	ips [switches] [input file name]
 *
 * The specification of an input file name is optional.  If none is provided,
 * stdin is assumed.
 *
 * possible switches:
 *
 *	-D	The following argument is taken as a string to be prepended
 *		to the file being sent.
 *
 *	-a	The following argument is assumed to be the name of the
 *		file to which status data is written.  If no further 
 *		arguments are provided, the program aborts with an
 *		error message.
 *
 *	-h	The following argument is name of a file, the first line of
 *		which is the host name of the printer to be employed, and the
 *		second line is the name of the accounting file to be used.
 *		An internet address may be substituted for the host name in
 *		the file, though this is discouraged.
 *
 *	-i	The following argument is used as the name of the IMPRINT-10.
 *		If no further arguments are provided, the program aborts
 *		with an error message.  An internet address may be supplied
 *		instead of a host name, but this is discouraged.
 *
 *	-l	The following argument is taken to be the log file.  If
 *		the following argument is '-', the user's terminal will be
 *		used as the log file (for UNIX, stdout is used for the log
 *		file).  If no further arguments are provided, the program
 *		aborts with an error message.
 *
 *	-r	If the program accepts input from the users terminal, the 
 *		input is assumed not rewindable, whereas if an explicit file
 *		name is given it is assumed rewindable.  This switch toggles
 *		this assumption.
 *
 *	-u	The following argument is assumed to be a string representing
 *		the users identity.  This is passed to a low level routine
 *		for accounting purposes (on UNIX, the string is the uid # in
 *		decimal form). (CURRENTLY NOT IMPLEMENTED)
 */

PHscanargs(argc, argv)
int argc;
char *argv[];
{
	char *cp;
	static char area[256];
	char *tpntr;

	/* Scan args */
	while (--argc > 0) {
		char *pp;

		cp = *++argv;		/* pick up next arg */

		/* Non-switch must be our filespec */
		if (*cp++ != '-') {
			infile.name = *argv;
			infile.stdiswitch = FALSE;
			infile.rwdin = TRUE;
		}

		/* Otherwise, a switch */
		else switch (*cp++) {

			/* string to prepend */
		case 'D':
			if (--argc < 0)
				PHdie(-1, MSGSTR(IES_26, "usage: is -D string ...\n"));
			prestring = *++argv;
			break;

			/* Filename for attention writing */
		case 'a':
			if (--argc < 0)
				PHdie(-1, MSGSTR(IES_27, "usage: ies -a attnfile ...\n"));
			attnFile = *++argv;
			unlink(attnFile);
			break;

			/* Name of file containing printer host name
			   and accounting file */
		case 'P':
			if ((*argv)[2]) {
				pp = &((*argv)[2]);
			}
			else if (--argc < 0)
				PHdie(-1, MSGSTR(IES_28, "usage: ies -P printer ...\n"));
			else
				pp = *++argv;
#ifdef DEBUG
			fprintf(stderr,MSGSTR(IES_29, "printer id: %s\n"), pp);
#endif
			if (tgetent(Printcap, pp) <= 0) {
				PHdie(-1, MSGSTR(IES_30, "unknown printer %s\n"), pp);
				break;
			}
			if (hardset) break;
			tpntr = area;
			if (tgetstr("hn", &tpntr) == 0)
				PHdie(-1, MSGSTR(IES_31, "No hostname specified\n"));
			*tpntr = '\0';
			hname = area;
			break;

			/* Hostname or internet address of printer */
		case 'i':
			if (--argc < 0)
				PHdie(-1,
				   MSGSTR(IES_32, "usage: ies -i hostname ...\n"));
			hname = *++argv;
			hardset = TRUE;
			break;

			/* Filename for logging information */
		case 'l':
			if (--argc < 0)
				PHdie(-1, MSGSTR(IES_33, "usage: is -l logfile ...\n"));
			logname = *++argv;
			break;

			/* toggle rewindability assumption */
		case 'r':
			infile.rwdin = !infile.rwdin;
			break;

		case 'h':
			--argc;
			++argv;
			break;

		case 'n':
			--argc;
			++argv;
			break;

		}
	}
}


/*
 * updatestatus: Update the status every STATUSINT seconds.
 */

updatestatus()
{
	int i;

	/* let "connected" message show for a while */
	PHsleep(10000);
	for (i = 100;;i++) {
		PHAttR(i, MSGSTR(IES_34, "IMPRINT status: Job in progress, status: %s\n"),
						udpstatus(0, SECONDTIME));
		PHsleep(STATUSINT);
	}
}

/*
 * PHsend: send a file.  The argument is taken as a socket descriptor.
 * Files are sent in packets of size BUFSIZ.  Failure during a transmission
 * does not cause a retry currently, but this will be changed in the
 * future.
 */

PHsend(sck)
int sck;
{
	int i, j = 0;
	char buf[BUFSIZ];

	while (j != STREAMMARK) {
		/* form up a packet */
		i = 0;
		while ((i < BUFSIZ) && ((j = PHobyte()) != STREAMMARK))
			buf[i++] = j;

		/* send it off */
		if (write(sck, buf, i) != i)
			PHdie(-1, MSGSTR(IES_35, "IO write error: %s\n"), sys_errlist[errno]);
	}
}

/* get byte from input file */
int PHobyte()
{
	int i;

	/* check to see if we have already finished */
	if (infile.finished) return(ENDMARK);

	/* check if we have exhausted the prestring */
	if ((prestring != NIL) && ((i = prestring[preindex]) != 0)) {
		++preindex;
		return(i);
	}

	/* otherwise, try to get a byte */
	if ((i = PHgetc()) == -1) {
		infile.finished = TRUE;		/* we are done */
		return(STREAMMARK);
	}

	/* otherwise, pass on this byte */
	return(i);
}

/* 
 * rewind input files
 */

BOOL PHrewind()
{
	if (!infile.rwdin) return (FALSE);
	preindex = 0;
	PHrwnd();
	return(TRUE);
}

/*
 * Sleep for some number of mille seconds.  Often a lower level routine
 * will only be able to approximate this in some larger unit.
 */

PHsleep(ms)
int ms;
{
	int sec;

	/* sleep at least the minimum time if the requested time is not
	   zero but is shorter than minimum time.  Also, don't sleep
	   too long. */
	if (ms <= 0) return;
	else if(ms < 1000) sec = 1000;
	else if(ms > 300000) sec = 300000;
	else sec = ms;
	PHrsleep(sec);
}

/*
 * Asynchronous death -- death due to capture of an interrupt.
 */

PHadie(sig)
int sig;
{
	if (!hold || (sig != SIGINT))
		PHdie(-1, MSGSTR(IES_36, "Death on interrupt %d\n"), sig);
	else
		PHdie(0,"");
}

/*
 * Die.  Note that after mode, arguments are accepted in printf style.
 */

PHdie(mode, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
int mode;
char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{
	char buf[512];		/* large buffer for exit message */
	char *msg;

	if (!hold)
		PHnote(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	PHi_close();
	if (s < 0) close(s);
	PHl_close();

	(void) sprintf(buf, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	msg = buf;
	PHexit(mode, msg);
}

/*
 * Report system condition
 */

PHAttR(pstat, mp)
int pstat;
char *mp;
{
	static int lastpstat = -1;

#ifdef DEBUGV
	if (!hold)
		PHnote(MSGSTR(IES_37, "PHAttR: stat %o, last stat %o, message:\n%s"), pstat,
							lastpstat, mp);
#endif
	if (pstat == lastpstat)
		return;

	lastpstat = pstat;
	if (!PHa_open(attnFile, FALSE)) return;
	PHa_write(mp);
	PHa_close();
	PHa_protct(attnFile);
}
