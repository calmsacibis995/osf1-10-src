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
static char *rcsid = "@(#)$RCSfile: grantpt.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 18:04:44 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define grantpt __grantpt
#pragma weak grantpt = __grantpt
#endif

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/signal.h>
#define _PATH_CHGPT	"/usr/lbin/chgpt"

/*
 * grantpt
 */
int
grantpt(int mastfd)
{
	int		cstat, cpid, w;
	sigset_t	new, old;

	if (ioctl(mastfd, ISPTM, 0) == -1)
		return (-1);

	/* The SIGCHLD signal is blocked to prevent some handler
	 * catching it and then waiting for our child.
	 * In the thread safe case this is not legal since we may
	 * "restore" the wrong state. We assume thread safe
	 * programmers are less cavalier about waiting for random
	 * children.
	 */
#ifndef _THREAD_SAFE
	sigemptyset(&new);
	sigaddset(&new, SIGCHLD);
	sigprocmask(SIG_BLOCK, &new, &old);
#endif	/* _THREAD_SAFE */

	if ((cpid = fork()) == -1) {
#ifndef _THREAD_SAFE
		sigprocmask(SIG_SETMASK, &old, 0);
#endif	/* _THREAD_SAFE */
		return (-1);
	}
	if (cpid == 0) {
		if (mastfd == 0
		    || (dup2(mastfd, 0) != -1 && close(mastfd) != -1))
			execl(_PATH_CHGPT, "chgpt", 0);
		_exit(127);
	}

	w = waitpid(cpid, &cstat, 0);
#ifndef _THREAD_SAFE
	sigprocmask(SIG_SETMASK, &old, 0);
#endif	/* _THREAD_SAFE */
	
	if (w != -1 && WEXITSTATUS(cstat) == 0)
		return (0);
	return (-1);
}

