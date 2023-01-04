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
static char	*sccsid = "@(#)$RCSfile: su.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/12/20 21:44:34 $";
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

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#include <limits.h>
#include <sys/secdefines.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sia.h>
#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "su_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SU,n,s) 
#define MSGSTR_SEC(n,s) catgets(catd,MS_SU_SEC,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


char	*cleanenv[] = { "PATH=/usr/bin:.", 0, 0, 0, 0, 0, 0};
char	*user = "root";
char	*shell = "/sbin/sh";
int	fulllogin;
int	fastlogin;

extern char	**environ;
struct	passwd *pwd;

static int setenv();	/* forward declaration */

main(argc,argv)
	int argc;
	char *argv[];
{
	char *password;
	char buf[1000];
	FILE *fp;
	register char *p;
	SIAENTITY *entity=NULL;
	int oargc;
	char **oargv;

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_SU,NL_CAT_LOCALE);
#endif

	openlog("su", LOG_ODELAY, LOG_AUTH);

	oargc = argc;
	oargv = argv;
again:
	if (argc > 1 && strcmp(argv[1], "-f") == 0) {
		fastlogin++;
		argc--, argv++;
		goto again;
	}
	if (argc > 1 && strcmp(argv[1], "-") == 0) {
		fulllogin++;
		argc--, argv++;
		goto again;
	}
	if (argc > 1 && argv[1][0] != '-') {
		user = argv[1];
		argc--, argv++;
	}
	if(sia_ses_init(&entity, oargc, oargv, NULL, user, ttyname(2), 1, NULL) != SIASUCCESS)
		exit(2);
	if(sia_ses_suauthent(sia_collect_trm, entity) != SIASUCCESS) {
		exit(1);
	}

	pwd = entity->pwd;

	if (pwd->pw_shell && *pwd->pw_shell)
		shell = strdup(pwd->pw_shell);
	if (fulllogin) {
		int i;

		for(i=0; environ[i]; i++)
			if(!strncmp(environ[i], "TERM=", 5)) {
				cleanenv[1] = environ[i];
				break;
			}
		environ = cleanenv;
	}
	setenv("LOGNAME", pwd->pw_name); /* DAL001 */
	setenv("USER", pwd->pw_name);
	setenv("SHELL", shell);
	setenv("HOME", pwd->pw_dir);
	setpriority(PRIO_PROCESS, 0, 0);
	if(sia_ses_launch(sia_collect_trm, entity) != SIASUCCESS) {
		exit(3);
	}
	if (fastlogin) {
		*argv-- = "-f";
		*argv = "su";
	} else if (fulllogin) {
		if (chdir(pwd->pw_dir) < 0) {
			fprintf(stderr, MSGSTR(NO_DIR, "No directory\n"));
			exit(6);
		}
		*argv = "-su";
	} else
		*argv = "su";
	sia_ses_release(&entity);
	if(setreuid(geteuid(),geteuid()) < 0) {
		perror("setreuid()");
		exit(1);
	}
	execv(shell, argv);
	fprintf(stderr, MSGSTR(NO_SHELL, "No shell\n"));
	exit(7);
}

static int setenv(ename, evalue)
char *ename, *evalue;
{
	char *buf;

	buf = malloc(strlen(ename)+strlen(evalue)+1+1);
	strcpy(buf, ename);
	strcat(buf, "=");
	strcat(buf, evalue);
	return putenv(buf);
}
