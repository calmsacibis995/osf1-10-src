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
static char	*sccsid = "@(#)$RCSfile: popen.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:35:30 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: popen, pclose 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * popen.c	1.10  com/lib/c/io,3.1,8943 10/18/89 14:46:22
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak pclose = __pclose
#pragma weak popen = __popen
#endif
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <paths.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _popen_rmutex;
#endif /* _THREAD_SAFE */

#define SINGLESHELL     "/sbin/sh"

struct p_pids {
	struct  p_pids *next;
	pid_t	pid;
	int	fd;
	};
static struct p_pids *_pids;
static void pid_free();


/*                                                                    
 * FUNCTION: Initiates a pipe to/from a process.
 *                                                                    
 * RETURN VALUE DESCRIPTION: FILE * to opened stream, NULL on error. 
 */  

FILE *
popen(const char *cmd, const char *mode)
{
	int	pdes[2];
	FILE	*iop;
	struct p_pids *pidptr, *pptr;
	pid_t pid;
	char *shell;
	extern  char _DYNAMIC[];	/* 
					 * Magic symbol defined by ld:
					 *      set to 0 for -non_shared
					 *      set to X for -call_shared
					 *      set to X for -shared
					 *
					 * where X is the (non-zero) location
					 * of the .dynamic section.
					 */

	if ((*mode != 'r') && (*mode != 'w') || mode[1]) {
		_Seterrno(EINVAL);
		return(NULL);
	}
	if (pipe(pdes) < 0)
		return(NULL);
	switch (pid = fork()) {
	case -1:			/* error */
		(void) close(pdes[0]);
		(void) close(pdes[1]);
		return (NULL);
		/* NOTREACHED */
	case 0:				/* child */
		if (*mode == 'r') {
			if (pdes[1] != STDOUT_FILENO) {
				(void) dup2(pdes[1], STDOUT_FILENO);
				(void) close(pdes[1]);
			}
			(void) close(pdes[0]);
		} else {
			if (pdes[0] != STDIN_FILENO) {
				(void) dup2(pdes[0], STDIN_FILENO);
				(void) close(pdes[0]);
			}
			(void) close(pdes[1]);
		}
		pidptr = _pids;
		while (pidptr) {
			(void) close(pidptr->fd);
			pidptr = pidptr->next;
		}
		/*
		 * Use statically linked shell if the caller was statically
		 * linked (allows popen() to be used during installation)
		 */
		shell = (access(_PATH_BSHELL, X_OK) == 0 && _DYNAMIC) 
				? _PATH_BSHELL : SINGLESHELL;
		(void) execl(shell, "sh", "-c", cmd, (char *)0);
		_exit(127);
		/* NOTREACHED */
	}
	if (*mode == 'r') {
		iop = fdopen(pdes[0], mode);
		(void) close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], mode);
		(void) close(pdes[0]);
	}
	TS_LOCK(&_popen_rmutex);
	if ( !( pidptr = (struct p_pids *)malloc( sizeof(struct p_pids)))) {
		TS_UNLOCK(&_popen_rmutex);
		return (NULL);
	}
	bzero( (char *)pidptr, sizeof(struct p_pids));
	if (!_pids)
		_pids = pidptr;
	else {
		pptr = _pids;
		while (pptr->next)
			pptr = pptr->next; 
		pptr->next = pidptr;
	}
	pidptr->pid = pid;
	pidptr->fd = fileno(iop);
	TS_UNLOCK(&_popen_rmutex);
	return(iop);
}

int
pclose(FILE *ptr)
{
	register int f, r;
	int save_errno;
	int status;
	sigset_t	old,new;
	struct	p_pids *pidptr, *pptr;
	int	nfile;

	f = fileno(ptr);
	if ((nfile = sysconf( _SC_OPEN_MAX)) == -1)
		return( -1);
	if ( f < 0 || f >= nfile)
		goto invalid;

	TS_LOCK(&_popen_rmutex);
	if (! _pids)
		goto invalid;
	pidptr = _pids;
	while (pidptr) {
		if (pidptr->fd == f)
			break;
		pidptr = pidptr->next;
	}
	if (!pidptr) {
		TS_UNLOCK(&_popen_rmutex);
		goto invalid;
	}
	TS_UNLOCK(&_popen_rmutex);
	
	(void) fclose( ptr);

	sigemptyset( &new );
	sigemptyset( &old );
	sigaddset( &new, SIGINT );
	sigaddset( &new, SIGQUIT );
	sigaddset( &new, SIGHUP );
	sigprocmask( SIG_BLOCK, &new, &old );	/* Disable INT QUIT HUP */
	r = waitpid( pidptr->pid, &status, 0);

	save_errno = _Geterrno();
	if (r == -1) {
		status = -1;
		if (save_errno != EINTR)
			pid_free(pidptr);
	} else
		pid_free(pidptr);

	sigprocmask( SIG_SETMASK, &old, NULL);
	_Seterrno( save_errno);
	return( status);
invalid:
	_Seterrno( EBADF);
	return( -1);
}

static void
pid_free(pidptr)
struct p_pids *pidptr;
{
	struct p_pids *pptr;

	TS_LOCK(&_popen_rmutex);

	pptr = _pids;
	while (pptr) {
	    if (pptr->next == pidptr) {
		pptr->next = pidptr->next;
		break;
	    }
	    pptr = pptr->next;
	}

	/* INVARIANT: pidptr has been removed from the list.
	 * unless it was the only entry.
	 */

	if(_pids == pidptr)
	    _pids = NULL;

	free(pidptr);

	TS_UNLOCK(&_popen_rmutex);
}
