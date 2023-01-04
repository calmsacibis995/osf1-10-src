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
static char	*sccsid = "@(#)$RCSfile: mysys.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:57:17 $";
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
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: chgenv, mysys, system, getargs
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * mysys.c	1.4  com/cmd/man/learn,3.1,9021 11/28/89 13:34:34
 */

#include "stdio.h"
#include "signal.h"
#include "sys/types.h"
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

#define	EASY	1
#define	MEDIUM	2
#define	HARD	3
#define	EMAX	256

char *envp[EMAX+1];

/*
 * This routine edits the PATH environment variable so that
 * special commands that learners may need will be found.
 * EXINIT is modified so that the editor will always prompt,
 * will not print \r's, and will be usable with open mode.
 */

chgenv()
{
	register char **p;
	register int i;
	extern char **environ;
	extern char *dname;
	char path[BUFSIZ], exinit[BUFSIZ];
	char *malloc();

	sprintf(path, "PATH=%s/bin:/usr/bin:", dname);
	sprintf(exinit, "EXINIT=set prompt noopt open window=23");
	for (p=environ,i=2; *p != 0 && i < EMAX; p++,i++)   {
		envp[i] = *p;
		if (**p != 'P' && **p != 'E')
			continue;
		if (strncmp(*p, "PATH=", 5) == 0)
			sprintf(path, "PATH=%s/bin:%s", dname, &envp[i--][5]);
		else if (strncmp(*p, "EXINIT=", 7) == 0)
			sprintf(exinit, "%s|set prompt noopt open window=23", envp[i--]);
	}
	envp[0] = malloc(strlen(path) + 1);
	strcpy(envp[0], path);
	envp[1] = malloc(strlen(exinit) + 1);
	strcpy(envp[1], exinit);
	envp[i] = 0;
	environ = envp;
}

mysys(s)
char *s;
{
	/* like "system" but rips off "mv", etc.*/
	/* also tries to guess if can get away with exec cmd */
	/* instead of sh cmd */
	char p[300];
	char *np[40];
	register char *t;
	int nv, type, stat;

	type = EASY;	/* we hope */
	for (t = s; *t && type != HARD; t++) {
		switch (*t) {
		case '*': 
		case '[': 
		case '?': 
		case '>': 
		case '<': 
		case '$':
		case '\'':
		case '"':
		case '`':
		case '{':
		case '~':
			type = MEDIUM;
			break;
		case '|': 
		case ';': 
		case '&':
			type = HARD;
			break;
		}
	}
	switch (type) {
	case HARD:
		return(system(s));
	case MEDIUM:
		strcpy(p, "exec ");
		strcat(p, s);
		return(system(p));
	case EASY:
		strcpy(p,s);
		nv = getargs(p, np);
		t=np[0];
		if ((strcmp(t, "mv") == 0)||
		    (strcmp(t, "cp") == 0)||
		    (strcmp(t, "rm") == 0)||
		    (strcmp(t, "ls") == 0) ) {
			if (fork() == (pid_t) 0) {
				char b[LEN_MAX];
				signal(SIGINT, SIG_DFL);
				np[nv] = 0;
				execvp(t, np);
				perror(t);
				fprintf(stderr, MSGSTR(LEXECVFAIL, "Mysys:  execv failed on %s\n"), np); /*MSG*/
				exit(1);
			}
			wait(&stat);
			return(stat);
		}
		return(system(s));
	}
}

/*
 * system():
 *	same as library version, except that resets
 *	default handling of signals in child, so that
 *	user gets the behavior he expects.
 */

system(s)
char *s;
{
	int status;
	pid_t pid, w;
	register int (*istat)(int), (*qstat)(int);

	istat = (int (*)(int))signal(SIGINT, SIG_IGN);
	qstat = (int (*)(int))signal(SIGQUIT, SIG_IGN);
	if ((pid = fork()) == (pid_t) 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execl("/usr/bin/sh", "sh", "-c", s, (char *) 0);
		exit(1);
	}
	while ((w = wait(&status)) != pid && w != (pid_t) -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, (void (*)(int))istat);
	signal(SIGQUIT, (void (*)(int))qstat);
	return(status);
}

getargs(s, v)
char *s, **v;
{
	int i;

	i = 0;
	for (;;) {
		v[i++]=s;
		while (*s != 0 && *s!=' '&& *s != '\t')
			s++;
		if (*s == 0)
			break;
		*s++ =0;
		while (*s == ' ' || *s == '\t')
			s++;
		if (*s == 0)
			break;
	}
	return(i);
}
