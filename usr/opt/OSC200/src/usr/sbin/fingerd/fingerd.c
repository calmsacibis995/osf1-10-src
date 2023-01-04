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
static char	*sccsid = "@(#)$RCSfile: fingerd.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/07 23:08:45 $";
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
 * COMPONENT_NAME: TCPIP fingerd.c
 * 
 * FUNCTIONS: MSGSTR, Mfingerd, fatal 
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
 * Copyright (c) 1983 The Regents of the University of California.
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
 * fingerd.c   1.9  com/sockcmd/finger,3.1,9021 4/4/90 09:21:23
 * fingerd.c	5.4 (Berkeley) 11/23/88
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif 
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include "fingerd_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_FINGERD, Num, Str)

int	tracing = 0;

main(argc, argv)
	int	argc;
	char	*argv[];
{
	register FILE *fp;
	register int ch;
	register char *lp;
	int p[2];
	int on = 1;
	struct sigvec sv;
	int trace_handler();
#define	ENTRIES	50
	char **ap, *av[ENTRIES + 1], line[1024], *strtok();

	catd = catopen( MF_FINGERD, NL_CAT_LOCALE);

	/* Make sure it is the root */ 

	if (geteuid()) {
                fprintf(stderr, MSGSTR(NO_ROOT, "NOT super-user\n"));
                exit(1);
        }

	while ((ch = getopt(argc,argv,"s")) != EOF)
		switch(ch) {
		case 's':	tracing = 1;
				break;
		default:	fprintf(stderr,"usage: fingerd [-s]\n");
				break;
		}

#ifdef LOGGING					/* unused for now */
#include <netinet/in.h>
	struct sockaddr_in sin;
	int sval;

	sval = sizeof(sin);
	if (getpeername(0, &sin, &sval) < 0)
		fatal(MSGSTR(PEER,"getpeername"));
#endif

	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		fprintf(stderr,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m"));

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = (void (*)(int))trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = (void (*)(int))trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);


	if (!fgets(line, sizeof(line), stdin))
		exit(1);

	av[0] = "finger";
	for (lp = line, ap = &av[1];;) {
		*ap = strtok(lp, " \t\r\n");
		if (!*ap)
			break;
		/* RFC742: "/[Ww]" == "-l" */
		if ((*ap)[0] == '/' && ((*ap)[1] == 'W' || (*ap)[1] == 'w'))
			*ap = "-l";
		if (++ap == av + ENTRIES)
			break;
		lp = NULL;
	}

	if (pipe(p) < 0)
		fatal(MSGSTR(PIPE, "pipe"));

	switch(fork()) {
	case 0:
		(void)close(p[0]);
		if (p[1] != 1) {
			(void)dup2(p[1], 1);
			(void)close(p[1]);
		}
		execv("/usr/bin/finger", av);
		fatal("execv");
	case -1:
		fatal(MSGSTR(FORK, "fork"));
	}
	(void)close(p[1]);
	if (!(fp = fdopen(p[0], "r")))
		fatal(MSGSTR(FOPEN, "fdopen"));
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			putchar('\r');
		putchar(ch);
	}
	exit(0);
}

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		fprintf(stderr,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); 
}


fatal(msg)
	char *msg;
{
	extern int errno;
	int err;
	char *strerror();

	err = errno;  /* save it */
	fprintf(stderr, MSGSTR(ERRMSG,"fingerd: %s: %s\r\n"),
	    msg, strerror(err));
	exit(1);
}
