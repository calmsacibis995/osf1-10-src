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
static char *rcsid = "@(#)$RCSfile: scholar.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/07 15:08:42 $";
#endif
/*
 * scholar.c
 *
 *	Modification History:
 *
 * 18-Feb-1992
 *	Initial version - D. Parker
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

 */
/* 
 * COMPONENT_NAME: UUCP scholar.c
 * 
 * FUNCTIONS: MSGSTR, detect, schol_abort, 
 *            schol_dialer, schol_disconnect, schol_sync, min, sigALRM 
 *
 * ORIGINS: 10  26  27 
 *
 */

/*
 * Routines for calling up on a Scholar Modem
 * (based on the old VenTel driver).
 * The modem is expected to be strapped for "No input character echo".
 * The modem is also expected to be configured so that "auto answer" is
 * enabled.  "Modem response" must be configured to be "FULL".
 * DSR should also be set to "NORMAL".
 */
#include <sys/termio.h>
#include "tip.h"
#ifdef MSG
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_TIP,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#define	min(a,b)	((a < b) ? a : b)

static	void sigALRM();
static	int timeout = 0;
static	jmp_buf timeoutbuf;
#define DUMBUFLEN	40
static char dumbuf[DUMBUFLEN];
struct termio tty_termio;

schol_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
	char dummy;
#ifdef ACULOG
	char line[80];
#endif
	if (schol_sync() == 0)		/* make sure we can talk to the modem */
		return(0);
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
	ioctl(FD, TIOCHPCL, 0);
	ioctl(FD, TIOCFLUSH, 0);	/* get rid of garbage */
	write(FD, "T", 1);	/* send dial command */
	write(FD, num, strlen(num));
	write(FD, "!", 1);	/* start dialing */
	connected = 0;
	if (!detect("\r\nAttached\r\n")) 
		connected = 0;  
	else
		connected = 1;
	if (!connected)
		return (connected);	/* lets get out of here.. */
	ioctl(FD, TIOCFLUSH, 0);
#ifdef ACULOG
	if (timeout) {
		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "scholar", line);
	}
#endif
	if (timeout)
		schol_disconnect();	/* insurance */
	return (connected);
}


schol_disconnect()
{
	char c;
	int len, rlen;

	/* first hang up the modem*/
#ifdef DEBUG
	printf(MSGSTR(DISCONNECTING2, "\rdisconnecting modem....\n\r")); /*MSG*/
#endif
	ioctl(FD, TIOCCDTR, 0);
	sleep(1);
	ioctl(FD, TIOCSDTR, 0);
	/* turn CLOCAL off */
        ioctl(FD, TCGETA, &tty_termio);
        tty_termio.c_cflag &= ~CLOCAL;
        ioctl(FD, TCSETA, &tty_termio);
}

schol_abort()
{

	char c;

	write(FD, "\r", 1);	/* send anything to abort the call */
	schol_disconnect();
}

static void
sigALRM()
{

	printf(MSGSTR(TIMEOUT2, "\07timeout waiting for reply\n\r")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
detect(s)
	register char *s;
{
	void sigALRM();
	char c;
	void (*f)();
	f = signal(SIGALRM, sigALRM);
	timeout = 0;
#ifdef DEBUG
	printf(MSGSTR(DIALINGIT, "\ndetect: waiting for %s\n"), s);
#endif
	while (*s) {
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, f);
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
#ifdef DEBUG
		printf(MSGSTR(DIALINGIT, "%c 0x%x "), c, c);
#endif
		if (c != *s++)
			return (0);
	}
	signal(SIGALRM, f);
#ifdef DEBUG
	printf("\n");
#endif
	return (timeout == 0);
}

#define MAXRETRY	5

schol_sync()
{
	int len, retry = 0;

	while (retry++ <= MAXRETRY) {
		write(FD, "\002", 1);	/* send CTRL-B */
  		sleep(1);
		ioctl(FD, FIONREAD, &len);
		if (len) {
			len = read(FD, dumbuf, min(len, DUMBUFLEN));
			if (index(dumbuf, 'R') && index(dumbuf, 'e') &&
			  index(dumbuf, 'a') && index(dumbuf, 'd') &&
			  index(dumbuf, 'y'))
				return(1);
#ifdef DEBUG
			dumbuf[len] = '\0';
			printf(MSGSTR(DIALINGIT, "schol_sync2: (\"%s\") %d\n\r"), dumbuf, retry);
#endif
		}
		ioctl(FD, TIOCCDTR, 0);
		ioctl(FD, TIOCSDTR, 0);
	}
	printf(MSGSTR(CANTSYNC2, "Cannot synchronize with the modem...\n\r")); /*MSG*/
	return(0);
}
