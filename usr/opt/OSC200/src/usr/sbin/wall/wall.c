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
static char	*sccsid = "@(#)$RCSfile: wall.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/10/08 16:52:42 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* wall.c	3.3 18:26:00 6/19/90 SecureWare */
/* wall.c	5.4 (Berkeley) 10/22/87 */

/* 
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

*/

/*
 * wall.c - Broadcast a message to all users.
 *
 * This program is not related to David Wall, whose Stanford Ph.D. thesis
 * is entitled "Mechanisms for Broadcast and Selective Broadcast".
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <stdio.h>
#include <utmp.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <locale.h>
#include <nl_types.h>
#include "wall_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_WALL,Num,Str)

#define CONSOLE		"console"
#define XDISPLAY	':'					/*FPM001*/

extern struct utmp *getutent();

#define MAXMSG	30000

char	hostname[32];
char	mesg[MAXMSG];
int	msize,sline;
struct	utmp *utmp;
struct	utmp ut;
char	*strcpy();
char	*strcat();
char	*malloc();
char	who[sizeof(ut.ut_name)] = "???";
/* long	clock_var, time(); */ /* paw - ditto for bl8_3 merge */

/* smb - changed the declaration of time variables to be time_t type 
	 time() is already defined */ /* paw - ditto for bl8_3 merge */
time_t clock_var;

struct tm *localtime();
struct tm *localclock;

extern	errno;

main(argc, argv)
char *argv[];
{
	register int i, c;
	register struct utmp *p;
	int f;
	struct stat statb;
	struct utmp *utptr;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_WALL,NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (!authorized_user("sysadmin")) {
		fprintf(stderr,
			MSGSTR(AUTH, "%s: need sysadmin authorization\n"),
			command_name);
		exit(1);
	}
	signal(SIGTTOU, SIG_IGN);
#endif /* SEC_BASE */
	(void) gethostname(hostname, sizeof (hostname));
	setutent();
	clock_var = time( 0 );
	localclock = localtime( &clock_var );
	sline = ttyslot();	/* 'utmp' slot no. of sender */
	(void) stat(UTMP_FILE, &statb);
	utmp = (struct utmp *)malloc(statb.st_size);
	i=0;
	while ((utptr=getutent()) != NULL) {
		memcpy((char *)&utmp[i], (char *)utptr, sizeof(struct utmp));
		i++;
	}
	c=i; 	/* c = number of entries in utmp */
	if (sline)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));
	if (who[0] == '?') {
		struct passwd *pw;

		pw = getpwuid(getuid());
		if (pw)
			strncpy(who, pw->pw_name, sizeof(who)-1);
		endpwent();
	}
	(void)sprintf(mesg,
	    MSGSTR(BRODCAST, "\r\n\007\007Broadcast Message from %s@%s (%.*s) at %d:%02d ...\r\n\n")
		, who
		, hostname
		, sline ? sizeof(utmp[sline].ut_line) : sizeof("???")
		, sline ? utmp[sline].ut_line : "???"
		, localclock -> tm_hour
		, localclock -> tm_min
	);
	msize = strlen(mesg);
	if (argc >= 2) {
		/* take message from unix file instead of standard input */
		if (freopen(argv[1], "r", stdin) == NULL) {
			perror(argv[1]);
			exit(1);
		}
	}
	while ((i = getchar()) != EOF) {
		if (i == '\n')
			mesg[msize++] = '\r';
		if (msize >= MAXMSG) {
			fprintf(stderr, MSGSTR(TOOLONG,"Message too long (max %d chars).\n"), MAXMSG);
			exit(1);
		}
		mesg[msize++] = i;
	}
	fclose(stdin);
#if SEC_BASE
	/*
	 * Turn on privileges needed to open all tty devices.
	 * The SEC_LIMIT allows us to exceed the per-user process limit
	 * if we have to fork in sendmes().
	 */
	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_LIMIT,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr,
			MSGSTR(PRIV, "%s: insufficient privileges\n"),
			command_name);
		exit(1);
	}
#endif
	sendmes(CONSOLE);
	for (i=0; i<c; i++) {
		p = &utmp[i];
		if (p->ut_name[0] == 0 ||
		    p->ut_type != USER_PROCESS ||
		    strncmp(p->ut_line, CONSOLE, sizeof(p->ut_line)) == 0 ||
		    p->ut_line[0] == XDISPLAY) 			/*FPM001*/
			continue;
		sendmes(p->ut_line);
	}
	exit(0);
}

sendmes(tty)
char *tty;
{
	register f, flags;
	static char t[50] = "/dev/";
	int e, i;

	strcpy(t + 5, tty);

	if ((f = open(t, O_WRONLY|O_NDELAY)) < 0) {
		if (errno != EWOULDBLOCK)
			perror(t);
		return;
	}
	if ((flags = fcntl(f, F_GETFL, 0)) == -1) {
		perror(t);
		return;
	}
	if (fcntl(f, F_SETFL, flags | FNDELAY) == -1)
		goto oldway;
	i = write(f, mesg, msize);
	e = errno;
	(void) fcntl(f, F_SETFL, flags);
	if (i == msize) {
		(void) close(f);
		return;
	}
	if (e != EWOULDBLOCK) {
		errno = e;
		perror(t);
		(void) close(f);
		return;
	}
oldway:
	while ((i = fork()) == -1)
		if (wait((int *)0) == -1) {
			fprintf(stderr, MSGSTR(TRYAGAIN,"Try again\n"));
			return;
		}
	if (i) {
		(void) close(f);
		return;
	}

	(void) write(f, mesg, msize);
	exit(0);
}
