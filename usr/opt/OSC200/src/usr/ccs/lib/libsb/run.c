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
static char	*sccsid = "@(#)$RCSfile: run.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/08 00:46:02 $";
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
 * run.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 *  run, runv, runp, runvp --  execute process and wait for it to exit
 *
 *  Usage:
 *	i = run (file, arg1, arg2, ..., argn, 0);
 *	i = runv (file, arglist);
 *	i = runp (file, arg1, arg2, ..., argn, 0);
 *	i = runvp (file, arglist);
 *
 *  Run, runv, runp and runvp have argument lists exactly like the
 *  corresponding routines, execl, execv, execlp, execvp.  The run
 *  routines perform a fork, then:
 *  IN THE NEW PROCESS, an execl[p] or execv[p] is performed with the
 *  specified arguments.  The process returns with a -1 code if the
 *  exec was not successful.
 *  IN THE PARENT PROCESS, the signals SIGQUIT and SIGINT are disabled,
 *  the process waits until the newly forked process exits, the
 *  signals are restored to their original status, and the return
 *  status of the process is analyzed.
 *  All run routines return:  -1 if the exec failed or if the child was
 *  terminated abnormally; otherwise, the exit code of the child is
 *  returned.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if	VA_ARGV_IS_RECAST
#if defined(__alpha)
#define va_argv(list) ((char **) ((list)._a0))
#else
#define va_argv(list) ((char **) list)
#endif
#else
#if	!VA_ARGV_IS_ROUTINE
error Please define va_argv() macro(?) for your machine!!
#endif
#endif

#ifdef __STDC__
int run (char *name, ...)
#else
int run (va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	char *name;
#endif
	va_list ap;
	int val;

#ifdef __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runv (name, va_argv(ap));
	va_end(ap);
	return(val);
}

int runv (name,argv)
char *name,**argv;
{
	return (dorun (name, argv, 0));
}

#ifdef __STDC__
int runp (char *name, ...)
#else
int runp (va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	char *name;
#endif
	va_list ap;
	int val;

#ifdef __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runvp (name, va_argv(ap));
	va_end(ap);
	return (val);
}

int runvp (name,argv)
char *name,**argv;
{
	return (dorun (name, argv, 1));
}

static
int dorun (name,argv,usepath)
char *name,**argv;
int usepath;
{
	int wpid;
	register int pid;
	struct sigvec ignoresig,intsig,quitsig;
	union wait status;
	int execvp(), execv();
	int (*execrtn)() = usepath ? execvp : execv;

	if ((pid = vfork()) == -1)
		return(-1);	/* no more process's, so exit with error */

	if (pid == 0) {			/* child process */
		setgid (getgid());
		setuid (getuid());
		(*execrtn) (name,argv);
		fprintf (stderr,"run: can't exec %s\n",name);
		_exit (0377);
	}

	ignoresig.sv_handler = SIG_IGN;	/* ignore INT and QUIT signals */
	ignoresig.sv_mask = 0;
	ignoresig.sv_onstack = 0;
	sigvec (SIGINT,&ignoresig,&intsig);
	sigvec (SIGQUIT,&ignoresig,&quitsig);
	do {
/*		wpid = wait3 (&status.w_status, WUNTRACED, 0);	*/
		wpid = wait3 (&status, WUNTRACED, 0);
		if (WIFSTOPPED (status)) {
		    kill (0,SIGTSTP);
		    wpid = 0;
		}
	} while (wpid != pid && wpid != -1);
	sigvec (SIGINT,&intsig,0);	/* restore signals */
	sigvec (SIGQUIT,&quitsig,0);

	if (WIFSIGNALED (status) || status.w_retcode == 0377)
		return (-1);

	return (status.w_retcode);
}
