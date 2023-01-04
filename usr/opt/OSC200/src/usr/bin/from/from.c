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
static char	*sccsid = "@(#)$RCSfile: from.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 17:07:51 $";
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
 * COMPONENT_NAME: CMDMAILX from.c
 * 
 * FUNCTIONS: Mfrom, match 
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
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * 	from.c	5.2 (Berkeley) 11/4/85
 */

#include <sys/secdefines.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>

#include "from_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(Num, Def)	catgets(scmc_catd, MS_from, Num, Def)

main(argc, argv)
	int argc;
	register char **argv;
{
	char lbuf[BUFSIZ];
	char lbuf2[BUFSIZ];
	register struct passwd *pp;
	int stashed = 0, ch;
	register char *name;
	char *sender;
	char *getlogin();
	char *maildir = "/usr/spool/mail";  /* default mailbox dir */
	extern char *optarg;
	extern int optind;

#if SEC_MAC
	from_init(argc, argv);
#endif
	scmc_catd = catopen(MF_FROM, NL_CAT_LOCALE);

	sender = NULL;
	while ((ch = getopt(argc, argv, "s:d:")) != EOF)
		switch(ch) {
		    case 'd':  /* set system mailbox directory */
			    maildir = optarg;
			    break;
		    case 's':
			    sender = optarg;
			    for (name = sender; *name; name++)
				    *name = tolower(*name);
			    break;
		    default:
			    fprintf(stderr, MSGSTR(M_MSG_1,
			    "Usage: from [-d directory] [-s sender] [user]\n"));
			    exit (1);
			    break;
		}

	if (chdir(maildir) < 0) {
		perror(maildir);
		exit(1);
	}
	if (argc > optind)  /* user is on cmd line */
		name = argv[optind];
	else {
		name = getlogin ();
		if (name == NULL || strlen(name) == 0) {
			pp = getpwuid(getuid());
			if (pp == NULL) {
				fprintf(stderr, MSGSTR(M_MSG_2,
				    "Cannot get your login name\n"));
				exit(1);
			}
			name = pp->pw_name;
		}
	}
#if SEC_MAC
	if (!from_check_mailbox(name)) {
		fprintf(stderr, "Log in at your clearance to use %s\n",
			argv[0]);
		exit(1);
	}
#endif

	if (freopen(name, "r", stdin) == NULL) {
		if (errno != ENOENT) {  /* ok if file doesn't exist: no mail */
			perror(MSGSTR(M_MSG_3, "Cannot open mailbox"));
			exit(1);
		}
		exit(0);
	}
	while (fgets(lbuf, sizeof lbuf, stdin) != NULL)
		if (lbuf[0] == '\n' && stashed) {
			stashed = 0;
			printf("%s", lbuf2);
		} else if (strncmp(lbuf, "From ", 5) == 0 &&
		    (sender == NULL || match(&lbuf[4], sender))) {
			strcpy(lbuf2, lbuf);
			stashed = 1;
		}
	if (stashed)
		printf("%s", lbuf2);
	exit(0);
}

match (line, str)
	register char *line, *str;
{
	register char ch;

	while (*line == ' ' || *line == '\t')
		++line;
	if (*line == '\n')
		return (0);
	while (*str && *line != ' ' && *line != '\t' && *line != '\n') {
		ch = isupper(*line) ? tolower(*line) : *line;
		if (ch != *str++)
			return (0);
		line++;
	}
	return (*str == '\0');
}
