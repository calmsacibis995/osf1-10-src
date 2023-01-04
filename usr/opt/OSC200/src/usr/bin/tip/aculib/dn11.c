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
static char rcsid[] = "@(#)$RCSfile: dn11.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/09/07 15:08:03 $";
#endif
/*
dn11.c	1.2  com/cmd/tip/aculib,3.1,9013 10/17/89 16:57:02";
 */
/* 
 * COMPONENT_NAME: UUCP dn11.c
 * 
 * FUNCTIONS: MSGSTR, alarmtr, dn_abort, dn_dialer, dn_disconnect 
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

/* static char sccsid[] = "dn11.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for dialing up on DN-11
 */
#include "tip.h"

int dn_abort();
void alarmtr();
static jmp_buf jmpbuf;
static int child = -1, dn;

dn_dialer(num, acu)
	char *num, *acu;
{
	extern errno;
	char *p, *q, phone[40];
	int lt, nw, connected = 1;
	register int timelim;

	if (boolean(value(VERBOSE)))
		printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
	if ((dn = open(acu, 1)) < 0) {
		if (errno == EBUSY)
			printf(MSGSTR(LINEBUSY, "line busy...")); /*MSG*/
		else
			printf(MSGSTR(ACUERR, "acu open error...")); /*MSG*/
		return (0);
	}
	if (setjmp(jmpbuf)) {
		kill(child, SIGKILL);
		close(dn);
		return (0);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(num);
	alarm(timelim < 30 ? 30 : timelim);
	if ((child = fork()) == 0) {
		/*
		 * ignore this stuff for aborts
		 */
		signal(SIGALRM, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		sleep(2);
		nw = write(dn, num, lt = strlen(num));
		exit(nw != lt);
	}
	/*
	 * open line - will return on carrier
	 */
	if ((FD = open(DV, 2)) < 0) {
		if (errno == EIO)
			printf(MSGSTR(LOSTCARRIER, "lost carrier...")); /*MSG*/
		else
			printf(MSGSTR(LINEFAILED, "dialup line open failed...")); /*MSG*/
		alarm(0);
		kill(child, SIGKILL);
		close(dn);
		return (0);
	}
	alarm(0);
	ioctl(dn, TIOCHPCL, 0);
	signal(SIGALRM, SIG_DFL);
	while ((nw = wait(&lt)) != child && nw != -1)
		;
	fflush(stdout);
	close(dn);
	if (lt != 0) {
		close(FD);
		return (0);
	}
	return (1);
}

void
alarmtr()
{

	alarm(0);
	longjmp(jmpbuf, 1);
}

/*
 * Insurance, for some reason we don't seem to be
 *  hanging up...
 */
dn_disconnect()
{

	sleep(2);
	if (FD > 0)
		ioctl(FD, TIOCCDTR, 0);
	close(FD);
}

dn_abort()
{

	sleep(2);
	if (child > 0)
		kill(child, SIGKILL);
	if (dn > 0)
		close(dn);
	if (FD > 0)
		ioctl(FD, TIOCCDTR, 0);
	close(FD);
}
