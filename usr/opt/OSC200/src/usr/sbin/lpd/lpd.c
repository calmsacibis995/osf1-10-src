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
static char	*sccsid = "@(#)$RCSfile: lpd.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/11/23 22:27:05 $";
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
 * Copyright (c) 1983 Regents of the University of California.
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
 * lpd.c	5.6 (Berkeley) 6/30/88
 * lpd.c	4.1 15:58:34 7/19/90 SecureWare 
 */


/*
 * lpd -- line printer daemon.
 *
 * Listen for a connection and perform the requested operation.
 * Operations are:
 *	\1printer\n
 *		check the queue for jobs and print any found.
 *	\2printer\n
 *		receive a job from another machine and queue it.
 *	\3printer [users ...] [jobs ...]\n
 *		return the current state of the queue (short form).
 *	\4printer [users ...] [jobs ...]\n
 *		return the current state of the queue (long form).
 *	\5printer person [users ...] [jobs ...]\n
 *		remove jobs from the queue.
 *
 * Strategy to maintain protected spooling area:
 *	1. Spooling area is writable only by daemon and spooling group
 *	2. lpr runs setuid root and setgrp spooling group; it uses
 *	   root to access any file it wants (verifying things before
 *	   with an access call) and group id to know how it should
 *	   set up ownership of files in the spooling area.
 *	3. Files in spooling area are owned by root, group spooling
 *	   group, with mode 660.
 *	4. lpd, lpq and lprm run setuid daemon and setgrp spooling group to
 *	   access files and printer.  Users can't get to anything
 *	   w/o help of lpq and lprm programs.
 */
#include <locale.h>
#include "lp.h"

/* undefine sun */
#undef	sun

int	lflag;				/* log requests flag */

void	reapchild();
void	mcleanup();

main(argc, argv)
	int argc;
	char **argv;
{
	int f, funix, finet, defreadfds, fromlen;
	struct sockaddr_un sun, fromunix;
	struct sockaddr_in sin, frominet;
	int omask, lfd, pid;

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if ( security_is_on() ) lpd_initialize();
#endif
	gethostname(host, sizeof(host));
	name = argv[0];

	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-')
			switch (argv[0][1])
			{
			case 'l':
			    lflag++;
			    break;
			default:
			    fprintf(stderr,MSGSTR(LPD_1, "Usage: lpd [-l]\n"));
			    exit(1);
			}
	}

#ifndef DEBUG
	/*
	 * Set up standard environment by detaching from the parent.
	 * daemon() is from libutil and does all the right things.
	 */
	if (daemon(0,0) == -1)
	{
	    fprintf(stderr, MSGSTR(LPD_2, "Lpd: Fatal: Cannot fork\n"));
	    exit(1);
	}
#endif

	openlog("lpd", LOG_PID, LOG_LPR);
	syslog(LOG_LPR | LOG_INFO,MSGSTR(LPD_3, "lpd started"));
	(void) umask(0);
	lfd = open(MASTERLOCK, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		syslog(LOG_LPR | LOG_ERR, MSGSTR(LPD_4, "%s: cannot open masterlock: %m"), MASTERLOCK);
		exit(1);
	}
	if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		syslog(LOG_LPR | LOG_ERR, MSGSTR(LPD_5, "%s: locking failed: %m"), MASTERLOCK);
		exit(2);		/* tell nanny not to run us */
	}
	ftruncate(lfd, 0);
	/*
	 * write process id for others to know
	 */
	sprintf(line, "%u\n", getpid());
	f = strlen(line);
	if (write(lfd, line, f) != f) {
		syslog(LOG_LPR | LOG_ERR, "%s: %m", MASTERLOCK);
		exit(1);
	}
	signal(SIGCHLD, reapchild);
	/*
	 * Restart all the printers.
	 */
	startup();
	(void) unlink(SOCKETNAME);
	funix = socket(AF_UNIX, SOCK_STREAM, 0);
	if (funix < 0) {
		syslog(LOG_LPR | LOG_ERR, MSGSTR(LPD_6, "socket: %m"));
		exit(1);
	}
#define	mask(s)	(1 << ((s) - 1))
	omask = sigblock(mask(SIGHUP)|mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	signal(SIGHUP, mcleanup);
	signal(SIGINT, mcleanup);
	signal(SIGQUIT, mcleanup);
	signal(SIGTERM, mcleanup);
	strcpy(sun.sun_path, SOCKETNAME);
	sun.sun_family = AF_UNIX;
	if (bind(funix, &sun, 
		 strlen(sun.sun_path) + sizeof(sun.sun_family)) < 0)
	{
		syslog(LOG_LPR | LOG_ERR, MSGSTR(LPD_7, "ubind: %m"));
		exit(1);
	}
#if SEC_BASE
	lpd_setsockattr(SOCKETNAME);
#endif
	sigsetmask(omask);
	defreadfds = 1 << funix;
	listen(funix, 5);
	finet = socket(AF_INET, SOCK_STREAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		sp = getservbyname("printer", "tcp");
		if (sp == NULL) {
			syslog( LOG_LPR |LOG_ERR, MSGSTR(LPD_9, "printer/tcp: unknown service"));
			mcleanup();
		}
		bzero (&sin, sizeof(sin));	/* 001 - gray */
		sin.sin_family = AF_INET;
		sin.sin_port = sp->s_port;
		if (bind(finet, &sin, sizeof(sin), 0) < 0) {
			syslog( LOG_LPR |LOG_ERR, "bind: %m");
			mcleanup();
		}
		defreadfds |= 1 << finet;
		listen(finet, 5);
	}
	/*
	 * Main loop: accept, do a request, continue.
	 */
	for (;;) {
		int domain, nfds, s, readfds = defreadfds;

		nfds = select(20, &readfds, 0, 0, 0);
		if (nfds <= 0) {
			if (nfds < 0 && errno != EINTR)
				syslog( LOG_LPR |LOG_WARNING, "select: %m");
			continue;
		}
		if (readfds & (1 << funix)) {
			domain = AF_UNIX, fromlen = sizeof(fromunix);
			s = accept(funix, &fromunix, &fromlen);
		} else if (readfds & (1 << finet)) {
			domain = AF_INET, fromlen = sizeof(frominet);
			s = accept(finet, &frominet, &fromlen);
		}
		if (s < 0) {
			if (errno != EINTR)
				syslog( LOG_LPR |LOG_WARNING, "accept: %m");
			continue;
		}
		if (fork() == 0) {
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
			(void) close(funix);
			(void) close(finet);
			/* don't pass the lock file descriptor to child */
			(void) close(lfd);
			dup2(s, 1);
			(void) close(s);
			if (domain == AF_INET)
				chkhost(&frominet);
#if SEC_BASE
			/*
			 * Make sure that client at other end of UNIX domain
			 * connection has privilege.
			 */
			else
				lpd_checkclient(1);
#endif
			doit();
			exit(0);
		}
		(void) close(s);
	}
}

void reapchild()
{
	/* union wait status; */	/* Not needed any more */
	int stat_loc;
	pid_t i;

	while ((i = waitpid(-1, &stat_loc, WNOHANG)) > 0)
	    if (WTERMSIG(stat_loc) || WIFEXITED(stat_loc) )
		syslog( LOG_LPR |LOG_INFO, MSGSTR(LPD_10, "pid=%d (sig=%d,err=%d)"),
		       i, WTERMSIG(stat_loc), WEXITSTATUS(stat_loc));
}

void mcleanup()
{
	if (lflag)
		syslog( LOG_LPR |LOG_INFO, MSGSTR(LPD_11, "exiting"));
	unlink(SOCKETNAME);
	exit(0);
}

/*
 * Stuff for handling job specifications
 */
char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */
char	*person;		/* name of person doing lprm */

char	fromb[32];	/* buffer for client's machine name */
char	cbuf[BUFSIZ];	/* command line buffer */
char	*cmdnames[] = {
	"null",
	"printjob",
	"recvjob",
	"displayq short",
	"displayq long",
	"rmjob"
};

doit()
{
	register char *cp;
	register int n;

	for (;;) {
		cp = cbuf;
		do {
			if (cp >= &cbuf[sizeof(cbuf) - 1])
				fatal(MSGSTR(LPD_12, "Command line too long"));
			if ((n = netread(1, cp, 1)) != 1) {
				if (n < 0)
					fatal(MSGSTR(LPD_13, "Lost connection"));
				return;
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = cbuf;
		if (lflag) {
			if (*cp >= '\1' && *cp <= '\5')
				syslog( LOG_LPR |LOG_INFO, MSGSTR(LPD_14, "%s requests %s %s"),
					from, cmdnames[*cp], cp+1);
			else
				syslog( LOG_LPR |LOG_INFO, MSGSTR(LPD_15, "bad request (%d) from %s"),
					*cp, from);
		}
		switch (*cp++) {
		case '\1':	/* check the queue and print any jobs there */
			printer = cp;
			printjob();
			break;
		case '\2':	/* receive files to be queued */
			printer = cp;
			recvjob();
			break;
		case '\3':	/* display the queue (short form) */
		case '\4':	/* display the queue (long form) */
			printer = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal(MSGSTR(LPD_16, "Too many requests"));
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal(MSGSTR(LPD_17, "Too many users"));
					user[users++] = cp;
				}
			}
			displayq(cbuf[0] - '\3');
			exit(0);
		case '\5':	/* remove a job from the queue */
			printer = cp;
			while (*cp && *cp != ' ')
				cp++;
			if (!*cp)
				break;
			*cp++ = '\0';
			person = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal(MSGSTR(LPD_16, "Too many requests"));
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal(MSGSTR(LPD_17, "Too many users"));
					user[users++] = cp;
				}
			}
			rmjob();
			break;
		}
		fatal(MSGSTR(LPD_18, "Illegal service request (%d) from %s"), *--cp, from);
	}
}

/*
 * Make a pass through the printcap database and start printing any
 * files left from the last time the machine went down.
 */
startup()
{
	char buf[BUFSIZ];
	register char *cp;
	int pid;
	char spooldir[512], *sp;
        int nitems; /* number of items checkq discovers */

	printer = buf;

	/*
	 * Restart the daemons.
	 */
	while (getprent(buf) > 0) {
		/* get spool directory without screwing up */
		/* printcap enviroment. */
		sp = spooldir;
		if (pgetstr("sd", &sp) == NULL)
			strcpy(spooldir, DEFSPOOL);
		for (cp = buf; *cp; cp++)
			if (*cp == '|' || *cp == ':') {
				*cp = '\0';
				break;
			}
		/* only if we truly believe there are no entries do we */
		/* skip calling printjob() after forking a process */
		if ((nitems = checkq(spooldir)) == 0)
			continue;
		/* nitems will be -1 if there is an error */
		if (nitems == -1){ 
			syslog(LOG_LPR | LOG_WARNING, MSGSTR(LPD_24, 
			       "Warning: spooling directory \"%s\" is inaccessible"),
			       spooldir);
			continue;
		} 
		if ((pid = fork()) < 0) {
			syslog(LOG_LPR | LOG_WARNING, MSGSTR(LPD_19, "startup: cannot fork"));
			mcleanup();
		}
		if (!pid) {
			endprent();
			printjob();
		}
	}
}

#define DUMMY MSGSTR(LPD_20, ":nobody::")

/*
 * Check to see if the from host has access to the line printer.
 */
static
chkhost(f)
	struct sockaddr_in *f;
{
	register struct hostent *hp;
	register FILE *hostf;
	register char *cp, *sp;
	char ahost[MAXHOSTNAMELEN + 1];
	char temp_printer[40];		/* holds printer name if access fails,
					   used to generate error message only */
	int n, length;
	extern char *inet_ntoa();
	int baselen = -1;

	f->sin_port = ntohs(f->sin_port);
	if (f->sin_family != AF_INET || f->sin_port >= IPPORT_RESERVED)
		fatal(MSGSTR(LPD_21, "Malformed from address"));
	hp = gethostbyaddr((char *) (&f->sin_addr), sizeof(struct in_addr), f->sin_family);
	if (hp == 0)
		fatal(MSGSTR(LPD_22, "Host name for your address (%s) unknown"),
			inet_ntoa(f->sin_addr));

	strcpy(fromb, hp->h_name);
	from = fromb;
	if (!strcmp(from, host))
		return;

	sp = fromb;
	cp = ahost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - fromb;
			*cp++ = *sp++;
		} else
			*cp++ = isupper(*sp) ? tolower(*sp++) : *sp++;
	}
	*cp++ = '\0';

	/*
	 * Check the hosts.equiv file.
	 */
	hostf = fopen("/etc/hosts.equiv", "r");
	if (hostf) {
		if (_validuser(hostf, ahost, DUMMY, DUMMY, baselen) == 1) {
			/*
			 * Connection with host is allowed
			 */
			(void) fclose(hostf);
			return;
		}
		(void) fclose(hostf);
	}

	/*
	 * Check the hosts.lpd file.
	 */
	hostf = fopen("/etc/hosts.lpd", "r");
	if (hostf) {
		while (fgets(ahost, sizeof(ahost), hostf)) {
			if (ahost[0] == '#')			/* a comment line */
				continue;

			if (ahost[0] == '*') {
				/*
				 * Allow everybody
				 */
				(void) fclose(hostf);
				return;
			}
			if (cp = index(ahost, '\n'))
				*cp = '\0';

			cp = index(ahost, ' ');
			if (cp == NULL) {
				if (strcmp(from, ahost) == 0) {
					(void) fclose(hostf);
					return;
				}
				else if (((length = local_hostname_length(from)) != NULL) &&
					 strlen(ahost) == length && (strncmp(from, ahost, length) == 0)) {
					(void) fclose(hostf);
					return;
				}
			}
		}
		(void) fclose(hostf);
	}
	/*
	 * The remote host does not have access to the printer.  Read
	 * the printer name off the socket for use in an error message.
	 */
	if ((n = read(1, temp_printer, sizeof(temp_printer))) < 0)
		fatal(MSGSTR(LPD_13, "Lost connection"));
	cp = &temp_printer[1];
	while (cp < &temp_printer[sizeof(temp_printer)] && *cp != '\n' && *cp != ' ')
		++cp;
	*cp = '\0';
	printer = &temp_printer[1];

	fprintf(stdout, MSGSTR(LPD_25, "\nWarning: %s does not have access to remote printer %s\n"), from, printer);

	fatal(MSGSTR(LPD_23, "%s does not have access to remote printer %s"), from, printer);
}

