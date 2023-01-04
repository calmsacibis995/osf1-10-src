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
static char	*sccsid = "@(#)$RCSfile: lprm.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/08 15:14:41 $";
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
 * lprm.c	5.4 (Berkeley) 6/30/88
 * lprm.c	4.1 15:58:45 7/19/90 SecureWare 
 */


/*
 * lprm - remove the current user's spool entry
 *
 * lprm [-] [[job #] [user] ...]
 *
 * Using information in the lock file, lprm will kill the
 * currently active daemon (if necessary), remove the associated files,
 * and startup a new daemon.  Priviledged users may remove anyone's spool
 * entries, otherwise one can only remove their own.
 */

#include "lp.h"
#include <locale.h>

#if SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)

#include <sys/security.h>

extern priv_t *privvec();
#endif

#if !SEC_BASE
extern	int notopr;	/* imported from rmjob */
#endif

/*
 * Stuff for handling job specifications
 */
char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */
char	*person;		/* name of person doing lprm */

static char	luser[16];	/* buffer for person */

/* struct passwd *getpwuid(); */

main(argc, argv)
	char *argv[];
{
	register char *arg;
	struct passwd *p;
	struct direct **files;
	int nitems, assasinated = 0;
	void usage();

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	name = argv[0];
	gethostname(host, sizeof(host));
	openlog("lpd", 0, LOG_LPR);
#if SEC_BASE
	if ((p = getpwuid(getluid())) == NULL)
#else
	if ((p = getpwuid(getuid())) == NULL)
#endif
		fatal(MSGSTR(LPRM_1, "Who are you?"));
	if (strlen(p->pw_name) >= sizeof(luser))
		fatal(MSGSTR(LPRM_2, "Your name is too long"));
	strcpy(luser, p->pw_name);
	person = luser;
	while (--argc) {
		if ((arg = *++argv)[0] == '-')
			switch (arg[1]) {
			case 'P':
				if (arg[2])
					printer = &arg[2];
				else if (argc > 1) {
					argc--;
					printer = *++argv;
				}
				break;
			case '\0':
				if (!users) {
					users = -1;
					break;
				}
			default:
				usage();
			}
		else {
			if (users < 0)
				usage();
			if (isdigit(arg[0])) {
				if (requests >= MAXREQUESTS)
					fatal(MSGSTR(LPRM_4, "Too many requests"));
				requ[requests++] = atoi(arg);
			} else {
				if (users >= MAXUSERS)
					fatal(MSGSTR(LPRM_5, "Too many users"));
				user[users++] = arg;
			}
		}
	}
	if (printer == NULL && (printer = getenv("PRINTER")) == NULL)
		printer = DEFLP;

#if SEC_BASE
	/*
	 * Raise privileges needed by rmjob to kill the current daemon
	 * (in case we are removing the currently printing job) and to
	 * connect to a remote daemon in case the printer is remote.
	 * On systems with MAC configured, raise the override privilege
	 * so we can manipulate the queue regardless of our current SL.
	 */
	if (forceprivs(privvec(SEC_KILL, SEC_REMOTE,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lprm");
		exit(1);
	}
#endif
	rmjob();
}

static void usage()
{
	fprintf(stderr, MSGSTR(LPRM_6, "usage: lprm [-] [-Pprinter] [[job #] [user] ...]\n"));
	exit(2);
}

