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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 17:42:45 $";
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
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * main.c	1.5  com/cmd/arch/rdist,3.1,9013 11/1/89 14:15:05
 */

/*
#ifndef lint
char copyright[] =
"(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "(#)main.c	5.5 (Berkeley) 6/1/90";
#endif not lint
*/

#include <nl_types.h>
#include <locale.h>
#include "defs.h"

#define NHOSTS 100

/*
 * Remote distribution program.
 */

char	*distfile = NULL;
char    tmpf[] = _PATH_TMP;
char    *tmpname = &tmpf[5];

int	debug;		/* debugging flag */
int	nflag;		/* NOP flag, just print commands without executing */
int	qflag;		/* Quiet. Don't print messages */
int	options;	/* global options */
int	iamremote;	/* act as remote server for transfering files */

FILE	*fin = NULL;	/* input file pointer */
int	rem = -1;	/* file descriptor to remote source/sink process */
char	host[32];	/* host name */
int	nerrs;		/* number of errors while sending/receiving */
char	user[10];	/* user's name */
char	homedir[128];	/* user's home directory */
int	userid;		/* user's user ID */
int	groupid;	/* user's group ID */

struct	passwd *pw;	/* pointer to static area used by getpwent */
struct	group *gr;	/* pointer to static area used by getgrent */

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	int cmdargs = 0;
	char *dhosts[NHOSTS], **hp = dhosts;

	setlocale(LC_ALL,"");
	catd = NLcatopen(MF_RDIST,NL_CAT_LOCALE);
	pw = getpwuid((uid_t)(userid = (int)getuid()));
	if (pw == NULL) {
		fprintf(stderr, MSGSTR(WHOARE, "%s: Who are you?\n"), argv[0]);
		exit(1);
	}
	strcpy(user, pw->pw_name);
	strcpy(homedir, pw->pw_dir);
	groupid = pw->pw_gid;
	gethostname(host, sizeof(host));

	while (--argc > 0) {
		if ((arg = *++argv)[0] != '-')
			break;
		if (!strcmp(arg, "-Server"))
			iamremote++;
		else while (*++arg)
			switch (*arg) {
			case 'f':
				if (--argc <= 0)
					usage();
				distfile = *++argv;
				if (distfile[0] == '-' && distfile[1] == '\0')
					fin = stdin;
				break;

			case 'm':
				if (--argc <= 0)
					usage();
				if (hp >= &dhosts[NHOSTS-2]) {
					fprintf(stderr, MSGSTR(DESTTOO, "rdist: too many destination hosts\n"));
					exit(1);
				}
				*hp++ = *++argv;
				break;

			case 'd':
				if (--argc <= 0)
					usage();
				define(*++argv);
				break;

			case 'D':
				debug++;
				break;

			case 'c':
				cmdargs++;
				break;

			case 'n':
				if (options & VERIFY) {
					printf(MSGSTR(OVERR, "rdist: -n overrides -v\n"));
					options &= ~VERIFY;
				}
				nflag++;
				break;

			case 'q':
				qflag++;
				break;

			case 'b':
				options |= COMPARE;
				break;

			case 'R':
				options |= REMOVE;
				break;

			case 'v':
				if (nflag) {
					printf(MSGSTR(OVERR, "rdist: -n overrides -v\n"));
					break;
				}
				options |= VERIFY;
				break;

			case 'w':
				options |= WHOLE;
				break;

			case 'y':
				options |= YOUNGER;
				break;

			case 'h':
				options |= FOLLOW;
				break;

			case 'i':
				options |= IGNLNKS;
				break;

			default:
				usage();
			}
	}
	*hp = NULL;

	setreuid(0, userid);
	mktemp(tmpf);

	if (iamremote) {
		server();
		exit(nerrs != 0);
	}

	if (cmdargs)
		docmdargs(argc, argv);
	else {
		if (fin == NULL) {
			if(distfile == NULL) {
				if((fin = fopen("distfile","r")) == NULL)
					fin = fopen("Distfile", "r");
			} else
				fin = fopen(distfile, "r");
			if(fin == NULL) {
				perror(distfile ? distfile : "distfile");
				exit(1);
			}
		}
		yyparse();
		if (nerrs == 0)
			docmds(dhosts, argc, argv);
	}

	exit(nerrs != 0);
}

usage()
{
	printf(MSGSTR(USAGE, "Usage: rdist [-bhinqRvwyD] [-f distfile] [-d var=value] [-m host] [file ...]\n"));
	printf(MSGSTR(USAGE2, "or: rdist [-bhinqRvwyD] -c source [...] machine[:dest]\n"));
	exit(1);
}

/*
 * rcp like interface for distributing files.
 */
docmdargs(nargs, args)
	int nargs;
	char *args[];
{
	register struct namelist *nl, *prev;
	register char *cp;
	struct namelist *files, *hosts;
	struct subcmd *cmds;
	char *dest;
	static struct namelist tnl = { NULL, NULL };
	int i;

	if (nargs < 2)
		usage();

	prev = NULL;
	for (i = 0; i < nargs - 1; i++) {
		nl = makenl(args[i]);
		if (prev == NULL)
			files = prev = nl;
		else {
			prev->n_next = nl;
			prev = nl;
		}
	}

	cp = args[i];
	if ((dest = index(cp, ':')) != NULL)
		*dest++ = '\0';
	tnl.n_name = cp;
	hosts = expand(&tnl, E_ALL);
	if (nerrs)
		exit(1);

	if (dest == NULL || *dest == '\0')
		cmds = NULL;
	else {
		cmds = makesubcmd(INSTALL);
		cmds->sc_options = options;
		cmds->sc_name = dest;
	}

	if (debug) {
		printf("docmdargs()\nfiles = ");
		prnames(files);
		printf("hosts = ");
		prnames(hosts);
	}
	insert(NULL, files, hosts, cmds);
	docmds(NULL, 0, NULL);
}

/*
 * Print a list of NAME blocks (mostly for debugging).
 */
prnames(nl)
	register struct namelist *nl;
{
	printf("( ");
	while (nl != NULL) {
		printf("%s ", nl->n_name);
		nl = nl->n_next;
	}
	printf(")\n");
}

/*VARARGS*/
warn(fmt, a1, a2,a3)
	char *fmt;
{
	extern int yylineno;

	fprintf(stderr, MSGSTR(LINEW, "rdist: line %d: Warning: "), yylineno);
	fprintf(stderr, fmt, a1, a2, a3);
	fputc('\n', stderr);
}
