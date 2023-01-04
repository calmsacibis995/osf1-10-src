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
static char	*sccsid = "@(#)$RCSfile: leave.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 17:18:57 $";
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
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: leave
 *
 * ORIGINS: 9, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * leave.c	1.8  com/cmd/oper,3.1,9013 2/16/90 10:44:51
 */

#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "leave_msg.h" 

#define MSGSTR(n,s) catgets(catd,MS_LEAVE,n,s) 
nl_catd catd;

/*
 * leave [[+]hhmm]
 *
 * Reminds you when you have to leave.
 * Leave prompts for input and goes away if you hit return.
 * It nags you like a mother hen.
 */

main(argc, argv)
char **argv;
{
	time_t when, tod, now, diff, hours, minutes;
	char *cp;
	struct tm *nv;
	char buf[50];

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_LEAVE,NL_CAT_LOCALE);
	if (argc < 2) {
		printf(MSGSTR(WHEN, "When do you have to leave? ")); /*MSG*/
		cp = fgets(buf, sizeof(buf), stdin);
		if (*cp == '\n')
			exit(0);
	} else
		cp = argv[1];
	if (*cp == '+') {
		cp++;
		if (*cp < '0' || *cp > '9')
			usage();
		tod = atoi(cp);
		hours = tod / 100;
		minutes = tod % 100;
		if (minutes < 0 || minutes > 59)
			usage();
		diff = 60*hours+minutes;
		doalarm(diff);
		exit(0);
	}
	if (*cp < '0' || *cp > '9')
		usage();
	tod = atoi(cp);
	hours = tod / 100;
	if (hours > 12)
		hours -= 12;
	if (hours == 12)
		hours = 0;
	minutes = tod % 100;

	if (hours < 0 || hours > 12 || minutes < 0 || minutes > 59)
		usage();

	time(&now);
	nv = localtime(&now);
	when = 60*hours+minutes;
	if (nv->tm_hour > 12)
		nv->tm_hour -= 12;	/* do am/pm bit */
	now = 60*nv->tm_hour + nv->tm_min;
	diff = when - now;
	while (diff < 0)
		diff += 12*60;
	if (diff > 11*60) {
		printf(MSGSTR(PAST, "That time has already passed!\n")); /*MSG*/
		exit(1);
	}
	doalarm(diff);
	exit(0);
}

/*
 * NAME: usage
 * FUNCTION: displays the usage statement to the user
 */
usage()
{
	printf(MSGSTR(USAGE, "usage: leave [[+]hhmm]\n")); /*MSG*/
	exit(1);
}

/*
 * NAME: doalarm
 * FUNCTION: send user messages at the proper time
 */
doalarm(nmins)
long nmins;
{
	char *msg1, *msg2, *msg3, *msg4;
	register int i;
	int slp1, slp2, slp3, slp4;
	int seconds, gseconds;
	long daytime;
	char whenleave[NLTBMAX];

	seconds = 60 * nmins;
	if (seconds <= 0)
		seconds = 1;
	gseconds = seconds;

	msg1 = MSGSTR(FIVE, "\007\007You have to leave in 5 minutes.\n");/*MSG*/
	if (seconds <= 60*5) {
		slp1 = 0;
	} else {
		slp1 = seconds - 60*5;
		seconds = 60*5;
	}

	msg2 = MSGSTR(ONE, "\007\007Just one more minute!\n"); /*MSG*/
	if (seconds <= 60) {
		slp2 = 0;
	} else {
		slp2 = seconds - 60;
		seconds = 60;
	}

	msg3 = MSGSTR(TIME, "\007\007Time to leave!\n"); /*MSG*/
	slp3 = seconds;

	msg4 = MSGSTR(LATE, "\007\007You're going to be late!\n"); /*MSG*/
	slp4 = 60;

	time(&daytime);
	daytime += gseconds;
	if (strftime(whenleave, NLTBMAX, "%c\n", localtime(&daytime)))
		printf(MSGSTR(SET, "Alarm set for %s"), whenleave); /*MSG*/
	if (fork())
		exit(0);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	if (slp1)
		bother(slp1, msg1);
	if (slp2)
		bother(slp2, msg2);
	bother(slp3, msg3);
	for (i = 0; i < 10; i++)
		bother(slp4, msg4);
	printf(MSGSTR(BYE, "That was the last time I'll tell you. Bye.\n")); /*MSG*/
	exit(0);
}

/*
 * NAME: bother
 * FUNCTION: wait slp*100 seconds and then beep user (sound bell)
 */
bother(slp, msg)
int slp;
char *msg;
{
	int len = strlen(msg);

	delay(slp);

	/*
	 * if write fails, we've lost the terminal through someone else
	 * causing a vhangup by logging in.
	 */
	if (write(1, msg, len) != len)
		exit(0);
}

/*
 * delay is like sleep but does it in 100 sec pieces and
 * knows what zero means.
 */
delay(secs)
int secs;
{
	int n;

	while (secs > 0) {
		n = 100;
		if (secs < n)
			n = secs;
		secs -= n;
		if (n > 0)
			sleep((unsigned)n);
	}
}
