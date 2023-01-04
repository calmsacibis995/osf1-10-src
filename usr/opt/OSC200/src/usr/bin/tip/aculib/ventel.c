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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: ventel.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/09/07 15:08:57 $";
#endif

/* ventel.c	1.4  com/cmd/tip/aculib,3.1,9013 12/5/89 11:27:10"; */
/* 
 * COMPONENT_NAME: UUCP ventel.c
 * 
 * FUNCTIONS: MSGSTR, busyloop, delay, echo, gobble, min, 
 *            sigALRM, ven_abort, ven_dialer, ven_disconnect, vensync 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "ventel.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for calling up on a Ventel Modem
 * The Ventel is expected to be strapped for local echo (just like uucp)
 */
#include "tip.h"

#define	MAXRETRY	5

static	void sigALRM();
static	int timeout = 0;
static	jmp_buf timeoutbuf;

/*
 * some sleep calls have been replaced by this macro
 * because some ventel modems require two <cr>s in less than
 * a second in order to 'wake up'... yes, it is dirty...
 */
#define delay(num,denom) busyloop(CPUSPEED*num/denom)
#define CPUSPEED 1000000	/* VAX 780 is 1MIPS */
busyloop(n) { register long N = (n); while (--N > 0); }

static	int echo();
static	int gobble();
static	int vensync();

ven_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
	char *msg, line[80];

	/*
	 * Get in synch with a couple of carriage returns
	 */
	if (!vensync(FD)) {
		printf(MSGSTR(CANTSYNC5, "can't synchronize with ventel\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "ventel", MSGSTR(CANTSYNC2, "can't synch up")); /*MSG*/
#endif
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
	ioctl(FD, TIOCHPCL, 0);
	echo("#k$\r$\n$D$I$A$L$:$ ");
	for (cp = num; *cp; cp++) {
		delay(1, 10);
		write(FD, cp, 1);
	}
	delay(1, 10);
	write(FD, "\r", 1);
	gobble('\n', line);
	if (gobble('\n', line))
		connected = gobble('!', line);
	ioctl(FD, TIOCFLUSH);
#ifdef ACULOG
	if (timeout) {
		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "ventel", line);
	}
#endif
	if (timeout)
		ven_disconnect();	/* insurance */
	if (connected || timeout || !boolean(value(VERBOSE)))
		return (connected);
	/* call failed, parse response for user */
	cp = (char *) index(line, '\r');
	if (cp)
		*cp = '\0';
	for (cp = line; cp = (char *)index(cp, ' '); cp++)
		if (cp[1] == ' ')
			break;
	if (cp) {
		while (*cp == ' ')
			cp++;
		msg = cp;
		while (*cp) {
			if (isupper(*cp))
				*cp = tolower(*cp);
			cp++;
		}
		printf("%s...", msg);
	}
	return (connected);
}

ven_disconnect()
{

	close(FD);
}

ven_abort()
{

	write(FD, "\03", 1);
	close(FD);
}

static int
echo(s)
	register char *s;
{
	char c;

	while (c = *s++) switch (c) {

	case '$':
		read(FD, &c, 1);
		s++;
		break;

	case '#':
		c = *s++;
		write(FD, &c, 1);
		break;

	default:
		write(FD, &c, 1);
		read(FD, &c, 1);
	}
}

static void
sigALRM()
{

	printf(MSGSTR(TIMEOUT, "\07timeout waiting for reply\n")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
gobble(match, response)
	register char match;
	char response[];
{
	register char *cp = response;
	char c;
	int (*f)();

	signal(SIGALRM, sigALRM);
	timeout = 0;
	do {
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, (void (*)())f);
			*cp = '\0';
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, cp, 1);
		alarm(0);
		c = (*cp++ &= 0177);
#ifdef notdef
		if (boolean(value(VERBOSE)))
			putchar(c);
#endif
	} while (c != '\n' && c != match);
	signal(SIGALRM, SIG_DFL);
	*cp = '\0';
	return (c == match);
}

#define min(a,b)	((a)>(b)?(b):(a))
/*
 * This convoluted piece of code attempts to get
 * the ventel in sync.  If you don't have FIONREAD
 * there are gory ways to simulate this.
 */
static int
vensync(fd)
{
	int already = 0, nread;
	char buf[60];
	/*
	 * Toggle DTR to force anyone off that might have left
	 * the modem connected, and insure a consistent state
	 * to start from.
	 *
	 * If you don't have the ioctl calls to diddle directly
	 * with DTR, you can always try setting the baud rate to 0.
	 */
	ioctl(FD, TIOCCDTR, 0);
	sleep(1);
	ioctl(FD, TIOCSDTR, 0);
	while (already < MAXRETRY) {
		/*
		 * After reseting the modem, send it two \r's to
		 * autobaud on. Make sure to delay between them
		 * so the modem can frame the incoming characters.
		 */
		write(fd, "\r", 1);
		delay(1,10);
		write(fd, "\r", 1);
		sleep(2);
		if (ioctl(fd, FIONREAD, (caddr_t)&nread) < 0) {
			perror(MSGSTR(TIPIOCTL, "tip: ioctl")); /*MSG*/
			continue;
		}
		while (nread > 0) {
			read(fd, buf, min(nread, 60));
			if ((buf[nread - 1] & 0177) == '$')
				return (1);
			nread -= min(nread, 60);
		}
		sleep(1);
		already++;
	}
	return (0);
}

