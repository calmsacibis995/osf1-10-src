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
static char rcsid[] = "@(#)$RCSfile: biz22.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/09/07 15:07:27 $";
#endif
/* biz22.c	1.4  com/cmd/tip/aculib,3.1,9013 12/5/89 11:26:39"; */
/* 
 * COMPONENT_NAME: UUCP biz22.c
 * 
 * FUNCTIONS: MSGSTR, biz22_abort, biz22_disconnect, biz22f_dialer, 
 *            biz22w_dialer, biz_dialer, cmd, detect, sigALRM 
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

/*  "biz22.c	5.1 (Berkeley) 6/6/85"; */

#include "tip.h"

#define DISCONNECT_CMD	"\20\04"	/* disconnection string */

static	void sigALRM();
static	int timeout = 0;
static	jmp_buf timeoutbuf;
static	int cmd();
static	int detect();
/*
 * Dial up on a BIZCOMP Model 1022 with either
 * 	tone dialing (mod = "V")
 *	pulse dialing (mod = "W")
 */
static int
biz_dialer(num, mod)
	char *num, *mod;
{
	register int connected = 0;
	char cbuf[40];

	if (boolean(value(VERBOSE)))
		printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
	/*
	 * Disable auto-answer and configure for tone/pulse
	 *  dialing
	 */
	if (cmd("\02K\r")) {
		printf(MSGSTR(CANTINIT, "can't initialize bizcomp...")); /*MSG*/
		return (0);
	}
	strcpy(cbuf, "\02.\r");
	cbuf[1] = *mod;
	if (cmd(cbuf)) {
		printf(MSGSTR(CANTSET, "can't set dialing mode...")); /*MSG*/
		return (0);
	}
	strcpy(cbuf, "\02D");
	strcat(cbuf, num);
	strcat(cbuf, "\r");
	write(FD, cbuf, strlen(cbuf));
	if (!detect("7\r")) {
		printf(MSGSTR(CANTGET, "can't get dial tone...")); /*MSG*/
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(RINGING, "ringing...")); /*MSG*/
	/*
	 * The reply from the BIZCOMP should be:
	 *	2 \r or 7 \r	failure
	 *	1 \r		success
	 */
	connected = detect("1\r");
#ifdef ACULOG
	if (timeout) {
		char line[80];

		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "biz1022", line);
	}
#endif
	if (timeout)
		biz22_disconnect();	/* insurance */
	return (connected);
}

biz22w_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "W"));
}

biz22f_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "V"));
}

biz22_disconnect()
{
	int rw = 2;

	write(FD, DISCONNECT_CMD, 4);
	sleep(2);
	ioctl(FD, TIOCFLUSH, &rw);
}

biz22_abort()
{

	write(FD, "\02", 1);
}

static void
sigALRM()
{

	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
cmd(s)
	register char *s;
{
	char c;
	void (*f)();

	write(FD, s, strlen(s));
	f = signal(SIGALRM, sigALRM);
	if (setjmp(timeoutbuf)) {
		biz22_abort();
		signal(SIGALRM, f);
		return (1);
	}
	alarm(number(value(DIALTIMEOUT)));
	read(FD, &c, 1);
	alarm(0);
	signal(SIGALRM, f);
	c &= 0177;
	return (c != '\r');
}

static int
detect(s)
	register char *s;
{
	char c;
	void (*f)();

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	while (*s) {
		if (setjmp(timeoutbuf)) {
			biz22_abort();
			break;
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
		if (c != *s++)
			return (0);
	}
	signal(SIGALRM, f);
	return (timeout == 0);
}
