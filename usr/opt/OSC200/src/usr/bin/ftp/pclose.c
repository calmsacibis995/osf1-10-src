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
static char	*sccsid = "@(#)$RCSfile: pclose.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/05 16:56:47 $";
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

#endif not lint

#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern	char *malloc();

static	pid_t *popen_pid;
static	int nfiles;

FILE *
mypopen(cmd,mode)
	char *cmd;
	char *mode;
{
	int p[2];
	int myside, hisside;
	pid_t pid;

	if (nfiles <= 0)
		nfiles = getdtablesize();
	if (popen_pid == NULL) {
		popen_pid = (int *)malloc((unsigned) nfiles * sizeof *popen_pid);
		if (popen_pid == NULL)
			return (NULL);
		for (pid = 0; pid < nfiles; pid++)
			popen_pid[pid] = -1;
	}
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if ((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		(void) close(myside);
		if (hisside != tst(0, 1)) {
			(void) dup2(hisside, tst(0, 1));
			(void) close(hisside);
		}
		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(127);
	}
	if (pid == -1) {
		(void) close(myside);
		(void) close(hisside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	(void) close(hisside);
	return (fdopen(myside, mode));
}

void
pabort()
{
	extern int mflag;

	mflag = 0;
}

mypclose(ptr)
	FILE *ptr;
{
	void pabort();
	int omask, child;
	pid_t pid;
	void (*istat)();
	union wait status;

	child = popen_pid[fileno(ptr)];
	popen_pid[fileno(ptr)] = -1;
	(void) fclose(ptr);
	if (child == -1)
		return (-1);
	istat = signal(SIGINT, pabort);
	omask = sigblock(sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&status)) != child && pid != -1)
		;
	(void) sigsetmask(omask);
	(void) signal(SIGINT, istat);
	return (pid == -1 ? -1 : 0);
}
