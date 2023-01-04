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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: nice.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 17:37:12 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 *
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
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * nice.c	1.7  com/cmd/cntl,3.1,9013 10/27/89 16:59:54
 */

/*
 *   The nice command lets you  run the specified command at a
 *   lower priority (or higher if you are the superuser).
 */                                                                   

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <nl_types.h>
#include <errno.h>
#include "nice_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_NICE,Num,Str)
#include <locale.h>

void usage(void);

main(int argc, char **argv)
{
	int	nicarg = 10;
	int	been_here = 0;
	int	status;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_NICE,NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	/*
	 * Parsing is complex for compatibility with old usage, and
	 * "--" usage.
	 * POSIX and XPG4 usage is "nice [-n priority] command [arguments ...]"
	 *
	 * Accept any of the following:
	 * nice -n[-]X, nice -n [-]X, nice -[-]X, nice --, nice -X --
	 */
	while (argc > 1 && argv[1][0] == '-') {
		register char	*n, *p = argv[1];

		/* if extra "-", only valid if it's "--" */
		if (been_here)
			if (strcmp(p, "--"))
				usage();
			else {
				/* stop processing arguments now */
				argc--;
				argv++;
				break;
			}
		else
			been_here++;
		/*
		 * p points to priority, including sign; n points
		 * to numeric portion only -- used for correctness check
		 */
		if (*++p == 'n') {
			if (*++p == '\0') {
				p = argv[2];
				if (--argc < 2)
					usage();
				argv++;
			}
		}
		n = p;
		/* check for "nice - " : bad form */
		if (*n == '\0')
			usage();
		/* start check loop one character back */
		if(*n != '-')
			--n;
		while(*++n)
			if (!isdigit((int)*n)) {
				fprintf(stderr, MSGSTR(MSGBNUM,"nice: argument must be numeric.\n"));
				exit(2);
			}
		argc--;
		argv++;
		/* stop processing if "--"; use default nicarg */
		if ((n == (p + 1)) && *p == '-')
			break;
		nicarg = atoi(p);
	}
	if(argc < 2)
		usage();
#if SEC_BASE
	if (authorized_user("sysadmin"))
		forcepriv(SEC_LIMIT);
	disablepriv(SEC_SUSPEND_AUDIT);
#endif
	errno = 0;
	status = nice(nicarg);	/* Set priority of this process. */
	/*
	 * If it failed, let the caller know, but we still do the exec.
	 */
	if ((status == -1) && errno)
		fprintf(stderr, MSGSTR(NICEFAIL,"nice: not able to set requested priority: %s.\n"), strerror(errno));
	execvp(argv[1], &argv[1]);
	perror (argv[1]);
	/*
	 * Exit status as specified by XPG4:
	 * 1-125 if internal error in nice;
	 * 126 if problem invoking command;
	 * 127 if command not found;
	 * If we're here, we had a problem with execvp().
	 */
	if (errno == ENOENT)
		status = 127;
	else
		status = 126;
	exit(status);
}

void
usage(void)
{
	fprintf(stderr, MSGSTR(USAGE,"nice: usage: nice [-n priority] command [argument ...]\n"));
	exit(2);
}
