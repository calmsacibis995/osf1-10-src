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
static char rcsid[] = "@(#)$RCSfile: v3451.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/09/07 15:08:48 $";
#endif
/*
v3451.c	1.2  com/cmd/tip/aculib,3.1,9013 10/17/89 16:57:36";
 */
/* 
 * COMPONENT_NAME: UUCP v3451.c
 * 
 * FUNCTIONS: MSGSTR, alarmtr, expect, notin, prefix, v3451_abort, 
 *            v3451_dialer, v3451_disconnect, vawrite 
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

/* static char sccsid[] = "v3451.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for calling up on a Vadic 3451 Modem
 */
#include "tip.h"

static	jmp_buf Sjbuf;
static	vawrite();
static	expect();
static	void alarmtr();
static	notin();
static	prefix();

v3451_dialer(num, acu)
	register char *num;
	char *acu;
{
	int ok;
	void (*func)();
	int slow = number(value(BAUDRATE)) < 1200, rw = 2;
	char phone[50];
#ifdef ACULOG
	char line[80];
#endif

	/*
	 * Get in synch
	 */
	vawrite("I\r", 1 + slow);
	vawrite("I\r", 1 + slow);
	vawrite("I\r", 1 + slow);
	vawrite("\005\r", 2 + slow);
	if (!expect("READY")) {
		printf(MSGSTR(CANTSYNC4, "can't synchronize with vadic 3451\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "vadic", MSGSTR(CANTSYNC2, "can't synch up")); /*MSG*/
#endif
		return (0);
	}
	ioctl(FD, TIOCHPCL, 0);
	sleep(1);
	vawrite("D\r", 2 + slow);
	if (!expect("NUMBER?")) {
		printf(MSGSTR(NOACCEPT, "Vadic will not accept dial command\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "vadic", MSGSTR(NOACCEPT2, "will not accept dial")); /*MSG*/
#endif
		return (0);
	}
	strcpy(phone, num);
	strcat(phone, "\r");
	vawrite(phone, 1 + slow);
	if (!expect(phone)) {
		printf(MSGSTR(NOACCEPTPH, "Vadic will not accept phone number\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "vadic", MSGSTR(NOACCEPTPH2, "will not accept number")); /*MSG*/
#endif
		return (0);
	}
	func = signal(SIGINT,SIG_IGN);
	/*
	 * You cannot interrupt the Vadic when its dialing;
	 * even dropping DTR does not work (definitely a
	 * brain damaged design).
	 */
	vawrite("\r", 1 + slow);
	vawrite("\r", 1 + slow);
	if (!expect("DIALING:")) {
		printf(MSGSTR(FAILEDDIAL, "Vadic failed to dial\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "vadic", MSGSTR(FAILEDDIAL2, "failed to dial")); /*MSG*/
#endif
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	ok = expect("ON LINE");
	signal(SIGINT, func);
	if (!ok) {
		printf(MSGSTR(CALLFAILED, "call failed\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "vadic", MSGSTR(CALLFAILED2, "call failed")); /*MSG*/
#endif
		return (0);
	}
	ioctl(FD, TIOCFLUSH, &rw);
	return (1);
}

v3451_disconnect()
{

	close(FD);
}

v3451_abort()
{

	close(FD);
}

static
vawrite(cp, delay)
	register char *cp;
	int delay;
{

	for (; *cp; sleep(delay), cp++)
		write(FD, cp, 1);
}

static
expect(cp)
	register char *cp;
{
	char buf[300];
	register char *rp = buf;
	void  alarmtr();
	int timeout = 30, online = 0;

	if (strcmp(cp, "\"\"") == 0)
		return (1);
	*rp = 0;
	/*
	 * If we are waiting for the Vadic to complete
	 * dialing and get a connection, allow more time
	 * Unfortunately, the Vadic times out 24 seconds after
	 * the last digit is dialed
	 */
	online = strcmp(cp, "ON LINE") == 0;
	if (online)
		timeout = number(value(DIALTIMEOUT));
	signal(SIGALRM, alarmtr);
	if (setjmp(Sjbuf))
		return (0);
	alarm(timeout);
	while (notin(cp, buf) && rp < buf + sizeof (buf) - 1) {
		if (online && notin("FAILED CALL", buf) == 0)
			return (0);
		if (read(FD, rp, 1) < 0) {
			alarm(0);
			return (0);
		}
		if (*rp &= 0177)
			rp++;
		*rp = '\0';
	}
	alarm(0);
	return (1);
}

static void
alarmtr()
{

	longjmp(Sjbuf, 1);
}

static
notin(sh, lg)
	char *sh, *lg;
{

	for (; *lg; lg++)
		if (prefix(sh, lg))
			return (0);
	return (1);
}

static
prefix(s1, s2)
	register char *s1, *s2;
{
	register char c;

	while ((c = *s1++) == *s2++)
		if (c == '\0')
			return (1);
	return (c == '\0');
}
