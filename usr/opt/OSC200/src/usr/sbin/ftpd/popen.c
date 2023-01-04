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
static char	*sccsid = "@(#)$RCSfile: popen.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1993/01/18 14:22:09 $";
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

*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software written by Ken Arnold and
 * published in UNIX Review, Vol. 6, No. 8.
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
 * static char sccsid[] = "popen.c	5.7 (Berkeley) 9/1/88";
 */
/*
#ifndef lint
static char sccsid[] = "popen.c	5.8 (Berkeley) 6/1/90";
#endif  not lint */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdio.h>

/*
 * Special version of popen which avoids call to shell.  This insures noone
 * may create a pipe to a hidden program as a side effect of a list or dir
 * command.
 */
extern char *malloc(), *strcpy();
extern char **environ;

char   *envinit[] = {NULL};

static uid_t *pids;
static int fds;

FILE *
ftpd_popen(program, type)
	char *program, *type;
{
	register char *cp;
	FILE *iop;
	int argc, gargc, pdes[2];
	pid_t pid;
	char **pop, *argv[100], *gargv[1000], *vv[2], *bargv[100];
	extern char **glob(), **copyblk(), *strtok();

	if (*type != 'r' && *type != 'w' || type[1])
		return(NULL);

	if (!pids) {
		if ((fds = getdtablesize()) <= 0)
			return(NULL);
		if ((pids =
		    (uid_t *)malloc((u_int)(fds * sizeof(uid_t)))) == NULL)
			return(NULL);
		bzero((char *)pids, fds * sizeof(uid_t));
	}
	if (pipe(pdes) < 0)
		return(NULL);

	/* break up string into pieces */
	for (argc = 0, cp = program;; cp = NULL)
		if (!(bargv[argc] = argv[argc++] = strtok(cp, " \t\n")))
			break;
	    
	/* glob each piece */
	gargv[0] = argv[0];
	for (gargc = argc = 1; argv[argc]; argc++) {
		if (!(pop = glob(argv[argc]))) {	/* globbing failed */
			vv[0] = argv[argc];
			vv[1] = NULL;
			pop = copyblk(vv);
		}
		argv[argc] = (char *)pop;		/* save to free later */
		while (*pop && gargc < 1000)
			gargv[gargc++] = *pop++;
	}
	gargv[gargc] = NULL;

	iop = NULL;
	switch(pid = vfork()) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		goto pfree;
		/* NOTREACHED */
	case 0:				/* child */
		if (*type == 'r') {
			if (pdes[1] != 1) {
				dup2(pdes[1], 1);
				dup2(pdes[1], 2);	/* stderr, too!	*/
				(void)close(pdes[1]);
			}
			(void)close(pdes[0]);
		} else {
			if (pdes[0] != 0) {
				dup2(pdes[0], 0);
				(void)close(pdes[0]);
			}
			(void)close(pdes[1]);
		}
		environ = envinit;
		execv(gargv[0], gargv);
		perror(gargv[0]);
		_exit(1);
	}
	/* parent; assume fdopen can't fail...  */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}
	pids[fileno(iop)] = pid;

pfree:	 for (argc = 1; argv[argc] != NULL; argc++) {
		pop = (char **)argv[argc];
		if (strcmp(*pop,bargv[argc]) != 0)
                	blkfree((char **)argv[argc]);
                free((char *)argv[argc]);
        }
	return(iop);
}

ftpd_pclose(iop)
	FILE *iop;
{
	register int fdes;
	int omask;
	union wait stat_loc;
	pid_t pid;

	/*
	 * pclose returns -1 if stream is not associated with a
	 * `popened' command, or, if already `pclosed'.
	 */
	if (pids == 0 || pids[fdes = fileno(iop)] == 0)
		return(-1);
	(void)fclose(iop);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&stat_loc)) != pids[fdes] && pid != -1);
	(void)sigsetmask(omask);
	pids[fdes] = 0;
	return(pid == -1 ? -1 : stat_loc.w_status);
}
