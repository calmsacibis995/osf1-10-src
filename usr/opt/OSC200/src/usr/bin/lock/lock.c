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
static char	*sccsid = "@(#)$RCSfile: lock.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/10/11 17:19:06 $";
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
 * FUNCTIONS: lock
 *
 * ORIGINS: 26, 27
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
 * lock.c	1.4  com/cmd/oper,3.1,9021 10/22/89 17:39:13
 */

/*
 * Lock a terminal up until the given key is entered,
 * or until the root password is entered,
 * or the given interval times out.
 *
 * Timeout interval is by default TIMEOUT, it can be changed with
 * an argument of the form -time where time is in minutes
 */

#include <pwd.h>
#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "lock_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LOCK,n,s) 

#define TIMEOUT 15

struct	passwd *pwd;
char	*crypt();
char	*getpass();
char	*index();

int	quit(void);
int	bye(void);
int	hi(void);

struct timeval	timeout	= {0, 0};
struct timeval	zerotime = {0, 0};
struct termios 	tty, ntty;
long	nexttime;		/* keep the timeout time */

main(argc, argv)
	int argc;
	char **argv;
{
	char	*ttynam;
	char	*ap;
	int	sectimeout = TIMEOUT;
	char	s[BUFSIZ], s1[BUFSIZ];
	char	hostname[32];
	char date_buffer[100];
	struct timeval	timval;
	struct itimerval	ntimer, otimer;
	struct timezone	timzone;
	struct tm	*timp;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_LOCK,NL_CAT_LOCALE);

	/* process arguments */

	if (argc > 1){
		if (argv[1][0] != '-')
			usage();
		if (sscanf(&(argv[1][1]), "%d", &sectimeout) != 1)
			usage();
	}
	timeout.tv_sec = sectimeout * 60;

	/* get information for header */

	if (tcgetattr(0, &tty))
		exit(1);

	pwd = getpwuid((uid_t)0);
	gethostname(hostname, sizeof(hostname));
	if (!(ttynam = ttyname(0))){
		printf(MSGSTR(LNOTTERM, "lock: not a terminal?\n"));
		exit (1);
	}
	gettimeofday(&timval, &timzone);
	nexttime = timval.tv_sec + (sectimeout * 60);
	timp = localtime((time_t *)&timval.tv_sec);

	/* get key and check again */

	signal(SIGINT,(void (*)(int)) quit);
	signal(SIGQUIT, (void (*)(int))quit);
	ntty = tty; 
	ntty.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &ntty);
	printf(MSGSTR(LKEY, "Key: "));
	if (fgets(s, (int)sizeof(s), stdin) == NULL) {
		putchar('\n');
		quit();
	}
	printf(MSGSTR(LAGAIN, "\nAgain: "));
	/*
	 * Don't need EOF test here, if we get EOF, then s1 != s
	 * and the right things will happen.
	 */
	(void) fgets(s1, (int)sizeof(s1), stdin);
	putchar('\n');
	if (strcmp(s1, s)) {
		putchar(07);
		tcsetattr(0, TCSANOW, &tty);
		exit(1);
	}
	s[0] = 0;

	/* Set signal handlers */

	signal(SIGINT, (void (*)(int))hi);
	signal(SIGQUIT, (void (*)(int))hi);
	signal(SIGTSTP, (void (*)(int))hi);
	signal(SIGALRM, (void (*)(int))bye);
	ntimer.it_interval = zerotime;
	ntimer.it_value = timeout;
	setitimer(ITIMER_REAL, &ntimer, &otimer);

	/* Header info */

	printf (MSGSTR(LWHENTIMEOUT, "lock: %s on %s. timeout in %d minutes\n"),
		ttynam, hostname, sectimeout);
	NLstrtime(date_buffer, (int)sizeof(date_buffer), 
		MSGSTR(LDATEFMT, "%a %sD %T %z %Y"), timp);
	printf(MSGSTR(LTIMENOWIS, "time now is %s\n"), date_buffer);

	/* wait */

	for (;;) {
		printf(MSGSTR(LKEY, "Key: "));
		if (fgets(s, (int)sizeof(s), stdin) == NULL) {
			clearerr(stdin);
			hi();
			continue;
		}
		if (strcmp(s1, s) == 0)
			break;
		if (pwd == (struct passwd *) 0 || pwd->pw_passwd[0] == '\0')
			break;
		ap = index(s, '\n');
		if (ap != NULL)
			*ap = '\0';
		if (strcmp(pwd->pw_passwd, crypt(s, pwd->pw_passwd)) == 0) {
			break;
		}
		printf("\07\n");
		if (tcgetattr(0, &ntty))
			exit(1);
	}
	tcsetattr(0, TCSANOW, &tty);
	putchar('\n');
	exit (0);
}

/*
 * NAME: usage
 * FUNCTION: displays the usage statement to the user
 */
usage()
{
	printf(MSGSTR(LUSAGE, "Usage: lock [-timeout]\n"));
	exit (1);
}

/*
 * NAME: quit
 * FUNCTION: get out of here
 */
quit(void)
{
	tcsetattr(0, TCSANOW, &tty);
	exit (0);
}

/*
 * NAME: bye
 * FUNCTION: inform user time has run out and exit
 */
bye(void)
{
	tcsetattr(0, TCSANOW, &tty);
	printf(MSGSTR(LLOCKTIMOUT, "\nlock: timeout\n"));
	exit (1);
}

/*
 * NAME: hi
 * FUCNTION: tell the user we are waiting
 */
hi(void)
{
	long	curtime;
	struct timeval	timval;
	struct timezone	timzone;

	gettimeofday(&timval, &timzone);
	curtime = timval.tv_sec;
	printf(MSGSTR(LTYPINULK, "\nlock: type in the unlock key. timeout in %d minutes\n"),
		(nexttime-curtime)/60);
	printf(MSGSTR(LKEY, "Key: "));
	fflush(stdout);
}
