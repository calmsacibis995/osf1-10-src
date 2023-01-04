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
static char	*sccsid = "@(#)$RCSfile: comsat.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/10/07 23:08:22 $";
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
 * COMPONENT_NAME: CMDMAILX comsat.c
 * 
 * FUNCTIONS: jkfprintf, mailfor, notify, onalrm, 
 *            reapchildren, trace_handler 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * 	comsat.c	5.13 (Berkeley) 8/23/88
 *
 * Copyright (c) 1989 IBM Corporation.  All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <strings.h>

#include "comsat_msg.h" 


/*
 * comsat
 */
int	debug = 0;
#define	dsyslog	if (debug) syslog

#define MAXIDLE	120

char	hostname[MAXHOSTNAMELEN];
struct	utmp *utmp = NULL;
time_t	lastmsgtime, time();
int	nutmp, uf;
int	tracing = 0;

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(Num, Def)	catgets(scmc_catd, MS_comsat, Num, Def)

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno;
	register int cc;
	char msgbuf[100];
	struct sockaddr_in from;
	int fromlen;
	void reapchildren(), onalrm();
	int ch;
	int on = 1;
	struct sigvec sv;
	void trace_handler();
	char *maildir = "/usr/spool/mail";  /* default mailbox dir */
	extern char *optarg;

	scmc_catd = catopen(MF_COMSAT, NL_CAT_LOCALE);
	openlog(MSGSTR(M_COMSAT, "comsat") , LOG_PID | LOG_CONS, LOG_DAEMON);

        while ((ch = getopt(argc,argv,"sd:")) != EOF)
                switch(ch) {
		    case 'd':  /* set system mailbox directory */
			    maildir = optarg;
			    break;
		    case 's':
			    tracing = 1;
			    break;
		    default:
			    syslog(LOG_ERR, MSGSTR(M_USAGE,
				"usage: comsat [-s] [-d directory]"));
			    exit(2);
			    break;
                }

	/* verify proper invocation */
	fromlen = sizeof (from);
	if (getsockname(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror(MSGSTR(M_GETSOCK, "getsockname"));
		exit(1);
	}

	dsyslog(LOG_DEBUG, "chdir()ing to '%s'", maildir);
	if (chdir(maildir)) {
		syslog(LOG_ERR, MSGSTR(M_ECHDIR, "chdir: %s: %m"), maildir);
		exit(1);
	}
	if ((uf = open(UTMP_FILE, O_RDONLY, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(M_ETMP, "open: %s: %m"), UTMP_FILE);
		(void) recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		exit(1);
	}

	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m"));

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	(void)time(&lastmsgtime);
	(void)gethostname(hostname, sizeof (hostname));
	onalrm();  /* set up utmp, nutmp and set alarm */
	(void)signal(SIGALRM, onalrm);
	(void)signal(SIGTTOU, SIG_IGN);
	(void)signal(SIGCHLD, reapchildren);

	/* loop on getting msgs until onalarm() kills us after waiting
	   too long between msgs */
	for (;;) {
		cc = recv(0, msgbuf, sizeof (msgbuf) - 1, 0);
		if (cc <= 0) {  /* error: try again */
			if (errno != EINTR)
				sleep(1);
			errno = 0;
			continue;
		}
		if (!nutmp)		/* no one has logged in yet */
			continue;
		sigblock(sigmask(SIGALRM));
		msgbuf[cc] = 0;
		dsyslog(LOG_DEBUG, "got %d chars = '%s'", cc, msgbuf);
		(void)time(&lastmsgtime);  /* reset time-to-live */
		mailfor(msgbuf);  /* try to notify user */
		sigsetmask(0L);
	}
}

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
void
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m"));
}


void 
reapchildren()
{
	while (wait3((union wait *)NULL, WNOHANG, (struct rusage *)NULL) > 0);
}

/* alarm handler: wakes itself every 15 seconds to check for new logins;
   kills program after MAXIDLE seconds since last msg recvd, otherwise
   updates utmp and nutmp with new login data. */

void
onalrm()
{
	static u_int utmpsize;		/* last malloced size for utmp */
	static u_int utmpmtime;		/* last modification time for utmp */
	struct stat statbf;
	off_t lseek();
	char *malloc(), *realloc();

	if (time((time_t *)NULL) - lastmsgtime >= MAXIDLE) {  /* time to die */
		closelog();
		exit(0);
	}
	(void)alarm((u_int)15);  /* hit the snooze button */
	(void)fstat(uf, &statbf);  /* check the utmp file */
	if (statbf.st_mtime > utmpmtime) {  /* has it been modified? */
		utmpmtime = statbf.st_mtime;  /* update last mod time */
		/* realloc if it has been appended to */
		if (statbf.st_size > utmpsize) {
			utmpsize = statbf.st_size + 10 * sizeof(struct utmp);
			if (utmp)
				utmp = (struct utmp *)realloc((char *)utmp, utmpsize);
			else
				utmp = (struct utmp *)malloc(utmpsize);
			if (!utmp) {
				syslog(LOG_ERR, MSGSTR(M_EMALLOC, "malloc failed"));
				exit(1);
			}
		}
		/* read entries into utmp, set nutmp to number of records */
		(void)lseek(uf, 0L, L_SET);
		nutmp = read(uf, utmp, (int)statbf.st_size)/sizeof(struct utmp);
	}
}

mailfor(name)
	char *name;
{
	register struct utmp *utp = &utmp[nutmp];
	register char *cp;
	off_t offset, atol();

	if (!(cp = index(name, '@')))  /* must be "user@offset" */
		return;
	*cp = '\0';  /* parse fields */
	offset = atoi(cp + 1);
	while (--utp >= utmp)  /* look for user's login record */
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name))) {
			dsyslog(LOG_DEBUG,
			    "mailfor: found user '%s', offset %d",
			    name, offset);
			notify(utp, offset);
		}
}

static char	*cr;

notify(utp, offset)
	register struct utmp *utp;
	off_t offset;
{
	static char tty[20] = "/dev/";
	struct sgttyb gttybuf;
	FILE *tp;
	char name[sizeof (utmp[0].ut_name) + 1];
	struct stat stb;

	(void)strncpy(tty + 5, utp->ut_line, sizeof(utp->ut_line));
	dsyslog(LOG_DEBUG, "notify: checking tty '%s' for right mode", tty);
	if (stat(tty, &stb) || !(stb.st_mode & S_IEXEC)) {
		dsyslog(LOG_DEBUG, "%s: wrong mode on %s", utp->ut_name, tty);
		return;
	}
	dsyslog(LOG_DEBUG, "notify %s on %s", utp->ut_name, tty);
	if (fork())  /* fork child to do the dirty work */
		return;
	(void)signal(SIGALRM, SIG_DFL);
	(void)alarm((u_int)30);  /* kill child after 30 seconds of trying */
	if ((tp = fopen(tty, "w")) == NULL) {
		dsyslog(LOG_ERR, "fopen of tty %s failed: %m", tty);
		_exit(-1);
	}
	(void)ioctl(fileno(tp), TIOCGETP, &gttybuf);
	cr = (gttybuf.sg_flags&CRMOD) && !(gttybuf.sg_flags&RAW) ? "" : "\r";
	(void)strncpy(name, utp->ut_name, sizeof (utp->ut_name));
	name[sizeof (name) - 1] = '\0';
	dsyslog(LOG_DEBUG, "really notifying %s", name);
	fprintf(tp, MSGSTR(M_NEWMAIL, "%s\n\007New mail for %s@%.*s\007 has arrived:%s\n----%s\n") ,
	    cr, name, sizeof (hostname), hostname, cr, cr);
	jkfprintf(tp, name, offset);
	fclose(tp);
	_exit(0);
}

jkfprintf(tp, name, offset)
	register FILE *tp;
	char name[];
	off_t offset;
{
	register char *cp;
	register FILE *fi;
	register int linecnt, charcnt, inheader, loopcnt;
	char line[BUFSIZ];

	if ((fi = fopen(name, "r")) == NULL)
		return;
	(void)fseek(fi, offset, L_SET);
	/* 
	 * Print the first 7 lines or 560 characters of the new mail
	 * (whichever comes first).  Skip header crap other than
	 * From, Subject, To, and Date.
	 */
	linecnt = 7;
	charcnt = 560;
	inheader = 1;
	loopcnt = 10;
	do {
		while (fgets(line, sizeof (line), fi) != NULL) {
			if (inheader) {
				if (line[0] == '\n') {
					inheader = 0;
					continue;
				}
				if (line[0] == ' ' || line[0] == '\t' ||
				    strncmp(line, "From:", 5) &&
				    strncmp(line, "Subject:", 8))
					continue;
			}
			if (linecnt <= 0 || charcnt <= 0) {
				fprintf(tp, MSGSTR(M_MORE, "...more...%s\n") , cr);
				return;
			}
			if (cp = index(line, '\n'))
				*cp = '\0';
			fprintf(tp, "%s%s\n", line, cr);
			charcnt -= strlen(line);
			linecnt--;
			loopcnt = 0;
		}
		if (loopcnt) {
			sleep(1);
			loopcnt--;
		}
	} while (loopcnt);
	fprintf(tp, "----%s\n", cr);
}
