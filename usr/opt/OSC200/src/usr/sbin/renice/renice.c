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
static char rcsid[] = "@(#)$RCSfile: renice.c,v $ $Revision: 4.2.2.3 $ (OSF) $Date: 1993/10/08 15:53:23 $";
#endif
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
 * renice.c	1.8  com/cmd/cntl,3.1,9021 11/27/89 11:31:54 
 * renice.c	4.2 02:01:31 7/13/90 SecureWare 
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <nl_types.h>
#include <locale.h>
#include "renice_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_RENICE,num,str)  /*MSG*/

nl_catd	catd;
char	*progname;

#if SEC_BASE
static int	has_auth = 0;
extern priv_t	*privvec();
#endif

void usage(void);
int parse_obsolete_options(int argc, char **argv);
int donice(int, int, int, int);

void
usage(void)
{
	fprintf(stderr, MSGSTR(USAGE, 
	  "usage:  renice [-n increment] [ -p | -g | -u] ID...\n\
\trenice priority [-p] pids... [-g pgrp ...] [-u user...]\n"));
	exit(1);
	/*NOTREACHED*/
}

/*
 * Change the priority (nice) of processes
 * or groups of processes which are already
 * running.
 */
int
main(int argc, char **argv)
{
	register int c, optflag = 0, errs = 0;
	int	who = 0;
	int	incr=0;
	int 	which = PRIO_PROCESS;
	char 	*endptr;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_RENICE, NL_CAT_LOCALE);
	progname = argv[0];

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	has_auth = authorized_user("sysadmin");
#endif
	if (argc < 2)
		usage();

	if (isdigit(argv[1][0]) || 
	    ((argv[1][0] == '-' || argv[1][0] == '+') && isdigit(argv[1][1]))) {
		exit(parse_obsolete_options(argc, argv));
	}

	while ((c = getopt(argc, argv, "n:gpu")) != -1) {
		switch (c) {
		case 'n':
			incr = strtol(optarg, &endptr, 10);
			if (*endptr != '\0')
				usage();
			break;
		case 'g':
			which = PRIO_PGRP;
			optflag++;
			break;
		case 'p':
			which = PRIO_PROCESS;
			optflag++;
			break;
		case 'u':
			which = PRIO_USER;
			optflag++;
			break;

		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0 || (optflag > 1))
		usage();

	for (; argc > 0; argc--, argv++) {
		if (which == PRIO_USER && !isdigit(argv[0][0])) {
			register struct passwd *pwd = getpwnam(*argv);
			
			if (pwd == NULL) {
				fprintf(stderr, 
				  MSGSTR(UNK_USER,"%s: User %s is unknown.\n"),
				  progname,*argv);
				continue;
			}
			who = pwd->pw_uid;
		} else {
			who = strtol(*argv, &endptr, 10);
			if ((who < 0) || (*endptr != '\0')) {
				fprintf(stderr, 
				  MSGSTR(BADVALUE,"%s: Parameter %s is bad.\n"),
				  progname,*argv);
				continue;
			}
		}
		errs += donice(which, who, incr, 1);
	}
	exit (errs != 0);
}

int
parse_obsolete_options(int argc, char **argv)
{
	int which = PRIO_PROCESS;
	int who = 0, prio, errs = 0;
	char *endptr;

	argc--, argv++;
	if (argc < 2) {
		usage();
	}

	prio = strtol(*argv, &endptr, 10);
	if (*endptr != '\0') {
		usage();
	}

	argc--, argv++;
	for (; argc > 0; argc--, argv++) {
		if (strcmp(*argv, "-g") == 0) {
			which = PRIO_PGRP;
			continue;
		}
		if (strcmp(*argv, "-u") == 0) {
			which = PRIO_USER;
			continue;
		}
		if (strcmp(*argv, "-p") == 0) {
			which = PRIO_PROCESS;
			continue;
		}
		if (which == PRIO_USER && !isdigit(argv[0][0])) {
			register struct passwd *pwd = getpwnam(*argv);
			
			if (pwd == NULL) {
				fprintf(stderr, MSGSTR(UNK_USER,"%s: User %s is unknown.\n"),
					progname,*argv);
				continue;
			}
			who = pwd->pw_uid;
		} else {
			who = strtol(*argv, &endptr, 10);
			if ((who < 0) || (*endptr != '\0')) {
				fprintf(stderr, MSGSTR(BADVALUE,"%s: Parameter %s is bad.\n"),
					progname,*argv);
				continue;
			}
		}
		errs += donice(which, who, prio, 0);
	}
	return(errs != 0);
}

/*
 * Set the process priority.
 *
 * If increment is true, prio is passed as an increment.
 */
int
donice(int which, int who, int prio, int increment)
{
	int oldprio;
#if SEC_BASE
	privvec_t saveprivs;
#endif

	errno = 0, oldprio = getpriority(which, who);
	if (oldprio == -1 && errno) {
		fprintf(stderr, MSGSTR(GETPRIO,"%s: %d: "), progname,who);
		perror("getpriority");
		return (1);
	}
#if SEC_BASE
	if (has_auth) {
		if (forceprivs(privvec(SEC_OWNER, SEC_LIMIT,
#if SEC_MAC
					SEC_ALLOWMACACCESS,
#endif
					-1), saveprivs)) {
			fprintf(stderr,
				MSGSTR(PRIV, "%s: insufficient privileges\n"),
				progname);
			exit(1);
		}
	} else
		getpriv(SEC_EFFECTIVE_PRIV, saveprivs);
	disablepriv(SEC_SUSPEND_AUDIT);
#endif
	if (increment)
		prio = oldprio + prio;

	if (prio > PRIO_MAX)
		prio = PRIO_MAX;
	else if (prio < PRIO_MIN)
		prio = PRIO_MIN;

	if (setpriority(which, who, prio) < 0) {
#if SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		fprintf(stderr, MSGSTR(SETPRIO,"%s: %d: "), progname ,who);
		perror("setpriority");
		return (1);
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	fprintf(stdout,MSGSTR(NEWPRI, 
		"%d: old priority %d, new priority %d\n"), who, oldprio, prio);
	return (0);
}
