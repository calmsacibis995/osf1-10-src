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
static char rcsid[] = "@(#)$RCSfile: hayes.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/30 18:39:29 $";
#endif
/*
hayes.c	1.2  com/cmd/tip/aculib,3.1,9013 10/17/89 16:57:18";
 */
/* 
 * COMPONENT_NAME: UUCP hayes.c
 * 
 * FUNCTIONS: MSGSTR, error_rep, gobble, goodbye, hay_abort, 
 *            hay_dialer, hay_disconnect, hay_sync, min, sigALRM 
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

/* static char sccsid[] = "hayes.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for calling up on a Hayes Modem
 * (based on the old VenTel driver).
 * The modem is expected to be strapped for "echo".
 * Also, the switches enabling the DTR and CD lines
 * must be set correctly.
 * NOTICE:
 * The easy way to hang up a modem is always simply to
 * clear the DTR signal. However, if the +++ sequence
 * (which switches the modem back to local mode) is sent
 * before modem is hung up, removal of the DTR signal
 * has no effect (except that it prevents the modem from
 * recognizing commands).
 * (by Helge Skrivervik, Calma Company, Sunnyvale, CA. 1984) 
 */
/*
 * TODO:
 * It is probably not a good idea to switch the modem
 * state between 'verbose' and terse (status messages).
 * This should be kicked out and we should use verbose 
 * mode only. This would make it consistent with normal
 * interactive use thru the command 'tip dialer'.
 */
#include "tip.h"

#define	min(a,b)	((a < b) ? a : b)

static	void sigALRM();
static	int timeout = 0;
static	jmp_buf timeoutbuf;
static 	char gobble();
#define DUMBUFLEN	40
static char dumbuf[DUMBUFLEN];

#define	DIALING		1
#define IDLE		2
#define CONNECTED	3
#define	FAILED		4
static	int state = IDLE;

hay_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
	char dummy;
#ifdef ACULOG
	char line[80];
#endif
	if (hay_sync() == 0)		/* make sure we can talk to the modem */
		return(0);
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
	ioctl(FD, TIOCHPCL, 0);
	ioctl(FD, TIOCFLUSH, 0);	/* get rid of garbage */
	write(FD, "ATv0\r", 5);	/* tell modem to use short status codes */
	gobble("\r");
	gobble("\r");
	write(FD, "ATTD", 4);	/* send dial command */
	write(FD, num, strlen(num));
	state = DIALING;
	write(FD, "\r", 1);
	connected = 0;
        if (gobble("\r")) { /* Replaced 'if' construct  CMR     */
                        switch (dummy = gobble("012345")) {
                        case '1':
                        case '5':
                                connected = 1;
                                break;
                        case '2':
                        case '3':
                                dummy= gobble("012345");
                                if (dummy == '2' || dummy == '3')
                                        connected = 1;
                                else
                                        error_rep(dummy);
				break;
			default:
				error_rep(dummy);
				break;
			}
	}
	if (connected)
		state = CONNECTED;
	else {
		state = FAILED;
		return (connected);	/* lets get out of here.. */
	}
	ioctl(FD, TIOCFLUSH, 0);
#ifdef ACULOG
	if (timeout) {
		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "hayes", line);
	}
#endif
	if (timeout)
		hay_disconnect();	/* insurance */
	return (connected);
}


hay_disconnect()
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
	goodbye();
}

hay_abort()
{

	char c;

	write(FD, "\r", 1);	/* send anything to abort the call */
	hay_disconnect();
}

static void
sigALRM()
{

	printf(MSGSTR(TIMEOUT2, "\07timeout waiting for reply\n\r")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static char
gobble(match)
	register char *match;
{
	char c;
	int (*f)();
	int i, status = 0;

	signal(SIGALRM, sigALRM);
	timeout = 0;
#ifdef DEBUG
	printf("\ngobble: waiting for %s\n", match);
#endif
	do {
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, (void (*)(int ))f);
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
#ifdef DEBUG
		printf("%c 0x%x ", c, c);
#endif
		for (i = 0; i < strlen(match); i++)
			if (c == match[i])
				status = c;
	} while (status == 0);
	signal(SIGALRM, SIG_DFL);
#ifdef DEBUG
	printf("\n");
#endif
	return (status);
}

error_rep(c)
	register char c;
{
	printf("\n\r");
	switch (c) {

	case '0':
		printf(MSGSTR(OK, "OK")); /*MSG*/
		break;

	case '1':
		printf(MSGSTR(CONNECT, "CONNECT")); /*MSG*/
		break;
	
	case '2':
		printf(MSGSTR(RING, "RING")); /*MSG*/
		break;
	
	case '3':
		printf(MSGSTR(NOCARRIER, "NO CARRIER")); /*MSG*/
		break;
	
	case '4':
		printf(MSGSTR(ERROR, "ERROR in input")); /*MSG*/
		break;
	
	case '5':
		printf(MSGSTR(CONNECT12, "CONNECT 1200")); /*MSG*/
		break;
	
	default:
		printf(MSGSTR(UNKERR, "Unknown Modem error: %c (0x%x)"), c, c); /*MSG*/
	}
	printf("\n\r");
	return;
}

/*
 * set modem back to normal verbose status codes.
 */
goodbye()
{
	int len, rlen;
	char c;

	/* this probably never worked since len was never initialized */
	ioctl(FD, TIOCFLUSH, &len);	/* get rid of trash */
	if (hay_sync()) {
		sleep(1);
#ifndef DEBUG
	ioctl(FD, TIOCFLUSH, 0);
#endif
		write(FD, "ATH0\r", 5);		/* insurance */
#ifndef DEBUG
		c = gobble("03");
		if (c != '0' && c != '3') {
			printf(MSGSTR(CANTHANGUP, "cannot hang up modem\n\r")); /*MSG*/
			printf(MSGSTR(PLEASE, "please use 'tip dialer' to make sure the line is hung up\n\r")); /*MSG*/
		}
#endif
		sleep(1);
		ioctl(FD, FIONREAD, &len);
#ifdef DEBUG
		printf("goodbye1: len=%d -- ", len);
		rlen = read(FD, dumbuf, min(len, DUMBUFLEN));
		dumbuf[rlen] = '\0';
		printf("read (%d): %s\r\n", rlen, dumbuf);
#endif
		write(FD, "ATv1\r", 5);
		sleep(1);
#ifdef DEBUG
		ioctl(FD, FIONREAD, &len);
		printf("goodbye2: len=%d -- ", len);
		rlen = read(FD, dumbuf, min(len, DUMBUFLEN));
		dumbuf[rlen] = '\0';
		printf("read (%d): %s\r\n", rlen, dumbuf);
#endif
	}
	ioctl(FD, TIOCFLUSH, 0);	/* clear the input buffer */
	ioctl(FD, TIOCCDTR, 0);		/* clear DTR (insurance) */
	close(FD);
}

#define MAXRETRY	5

hay_sync()
{
	int len, retry = 0;

	while (retry++ <= MAXRETRY) {
		write(FD, "AT\r", 3);
		sleep(1);
		ioctl(FD, FIONREAD, &len);
		if (len) {
			len = read(FD, dumbuf, min(len, DUMBUFLEN));
			if (index(dumbuf, '0') || 
		   	(index(dumbuf, 'O') && index(dumbuf, 'K')))
				return(1);
#ifdef DEBUG
			dumbuf[len] = '\0';
			printf("hay_sync: (\"%s\") %d\n\r", dumbuf, retry);
#endif
		}
		ioctl(FD, TIOCCDTR, 0);
		ioctl(FD, TIOCSDTR, 0);
	}
	printf(MSGSTR(CANTSYNC3, "Cannot synchronize with hayes...\n\r")); /*MSG*/
	return(0);
}
