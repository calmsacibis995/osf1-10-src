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
static char	*sccsid = "@(#)$RCSfile: fork.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/21 11:00:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1989 Michael B. Jones
 */

#ifndef	lint
#endif	not lint

/*
 * Implementation of the unix fork call. This is necesary to ensure the world
 * is in a known state after the fork and we don't deadlock because the other
 * threads are no longer with us.
 */

#include <mach.h>
#include <pthread.h>
#include "internal.h"
#include <syscall.h>

/*
 * Function:
 *	unix_fork
 *
 * Return value:
 *	The same as fork().
 *
 * Description:
 *	Implement a straight unix fork() call even though we overload fork().
 */
pid_t
unix_fork()
{
	pid_t	parent_pid;
	pid_t	fork_return;
	pid_t	our_pid;

	parent_pid = getpid();
	fork_return = syscall(SYS_fork);
	our_pid = getpid();

	if (fork_return < 0)
		return(fork_return);		/* Fork error */

	if (parent_pid == our_pid) {
		/*
		 * Parent.
		 */
		return(fork_return);		/* Parent returns child pid */
	} else {
		/*
		 * Child.
		 *
		 * Reinitialize mach ports, etc. as per libmach
		 */
		(void) mach_init();
		/*
		 * Syscall(SYS_fork) doesn't return 0 to the child.
		 * Returning fork_return would be incorrect.
		 */
		return(0);
	}
}

/*
 * Function:
 *	fork
 *
 * Return value:
 *	0	if this is the child
 *	childpid if this is the parent
 *	-1	to the parent if the fork failed.
 *
 * Description:
 *      Fork a multi-threaded process, cleaning up the pthread
 *	state in the child. The child will come up with only one
 *	thread running.
 */
pid_t
fork()
{
	pid_t	fork_return;

	pthread_fork_prepare();		/* Grab pthread locks to insure a
    					   consistent state after the fork */

	fork_return = unix_fork();	/* Do an actual fork (and mach_init) */

	if (fork_return != 0) {
		/*
		 * Parent and error cases.
		 */
		pthread_fork_parent();	/* Release locks grabbed by prepare */
	} else {
		/*
		 * Child.
		 */
		pthread_fork_child();	/* Rebuild a consistent state */
	}
	return(fork_return);
}

/*
 * Function:
 *	vfork
 *
 * Description:
 *	layer vfork over our fork so this is safe too.
 */
pid_t
vfork()
{
	return(fork());
}
