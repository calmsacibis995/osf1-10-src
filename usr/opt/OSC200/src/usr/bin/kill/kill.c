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
static char rcsid[] = "@(#)$RCSfile: kill.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 16:59:52 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * kill.c	1.10  com/cmd/cntl,3.1,9013 2/16/90 12:55:11
 * kill.c	4.1 23:50:36 7/12/90 SecureWare 
 */

/*
 *   The kill command sends a  signal to a running process, by default
 *   signal SIGTERM  (Software  Terminate #15).  This  default action
 *   normally kills  processes  that do  not catch  or ignore the
 *   signal.   You specify a process  by giving its process-ID (process
 *   identification  number, or PID).
 */                                                                   

#include	<sys/secdefines.h>
#if SEC_BASE
#include	<sys/security.h>

extern priv_t	*privvec();
#endif

#include	<stdio.h>
#include	<signal.h>
#include	<limits.h>
#include	<sys/errno.h>

#include        <nl_types.h>
#include 	<locale.h>
#include        "kill_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_KILL,num,str)  /*MSG*/

nl_catd         catd;

extern int signum(char *signal);
extern void sigprt(int signo);

main(argc, argv)
char **argv;
{
	register signo, res;
	pid_t 	pid;
	int	errlev = 0, neg = 0, zero = 0;
	extern	errno;
	char	*msg;


	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_KILL, NL_CAT_LOCALE);
	if (argc <= 1)
		usage();
#if SEC_BASE
	set_auth_parameters(argc, argv);
#endif
	if (!strcmp(argv[1],"-l")) {
		if ( argc > 2) {
		  signo = signum(argv[2]);
		} else
		  signo = -1;
		sigprt(signo);
		exit(0);
	}
	if (!strcmp(argv[1],"-s")) {
		if ( argc > 2)
		  signo = signum(argv[2]);
		else
		  signo = -1;
		if (signo == -1) {
			fprintf(stderr, MSGSTR(BADNO,"bad signal number\n"));
			usage();
		}
		argc -= 2;
		argv += 2;
	}
	else if (*argv[1] == '-') {
		signo = signum(argv[1]+1);
		if (signo == -1) {
			fprintf(stderr, MSGSTR(BADNO,"bad signal number\n"));
			usage();
		}
		argc--;
		argv++;
	} else
		signo = SIGTERM;
#if SEC_BASE
	initprivs();
	if (authorized_user("sysadmin") &&
	    forceprivs(privvec(SEC_KILL,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(PRIV, "%s: insufficient privileges\n"),
			"kill");
		exit(1);
	}
#endif
	argv++;
	while (argc > 1) {
		if (**argv == '-') neg++;
		if (**argv == '0') zero++;
		pid = atoi(*argv);
		if (	((pid == 0) && !zero)
		     || ((pid < 0) && !neg)
		     || (pid > PID_MAX)
		     || (pid < -PID_MAX)
		      ) usage();
#if SEC_BASE
		disablepriv(SEC_SUSPEND_AUDIT);
#endif
		res = kill(pid, signo);
#if SEC_BASE
		forcepriv(SEC_SUSPEND_AUDIT);
#endif
		if (res<0) {
			if(pid <= 0) {
				pid = abs(pid);
				msg = MSGSTR(EPGROUP,
					     "not a killable process group");
			}
			else if (errno == EPERM)
				msg = MSGSTR(EPDENIED, "permission denied");
			else if (errno == EINVAL)
				msg = MSGSTR(ESIGNAL, "invalid signal");
			else msg = MSGSTR(ENOPROC, "no such process");
			fprintf(stderr,MSGSTR(EKILL, "kill: %d: %s\n"), pid, msg);
			errlev = 2;
		}
		argc--;
		argv++;
		neg = zero = 0;
	}
	return(errlev);
}
usage()
{
	fprintf(stderr, MSGSTR(USAGE, "usage: kill -s signal_name pid ...\n"));
	fprintf(stderr, MSGSTR(USAGE1, "usage: kill [ -signal ] pid ...\n"));
	fprintf(stderr, MSGSTR(USAGE2, "usage: kill -l [exit_status]\n"));
	exit(2);
}
