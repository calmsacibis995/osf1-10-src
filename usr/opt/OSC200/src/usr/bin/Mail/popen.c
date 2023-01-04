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
static char	*sccsid = "@(#)$RCSfile: popen.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 13:10:30 $";
#endif 

/*
 */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:popen.c	1.4"
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * popen() and pclose()
 *
 * stolen from C library, modified to use SHELL variable
 */


#include <stdio.h>
#include <signal.h>

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern FILE *fdopen();
extern int execlp(), fork(), pipe(), close(), fcntl();
static int popen_pid[20];

FILE *
popen(cmd, mode)
char	*cmd, *mode;
{
	int	p[2];
	register int myside, yourside, pid;
	char *sHELL, *value();

	if ((sHELL = value("SHELL"))==NULL)
		sHELL = "/bin/sh";
	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		stdio = tst(0, 1);
		setgid(getgid());
		setuid(getuid());
		(void) close(myside);
		(void) close(stdio);
		(void) fcntl(yourside, 0, stdio);
		(void) close(yourside);
		(void) execlp(sHELL, "sh", "-c", cmd, 0);
		(void) execlp("/bin/sh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE	*ptr;
{
	register int f, r;
	int status;
#ifndef SIGRETRO
	void (*hstat)(), (*istat)(), (*qstat)();
#else
	int (*hstat)(), (*istat)(), (*qstat)();
#endif /* SIGRETRO */

	f = fileno(ptr);
	(void) fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	return(status);
}
